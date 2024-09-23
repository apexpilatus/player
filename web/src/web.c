#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <threads.h>
#include <unistd.h>

#define listen_port 80

static ssize_t msg_size;
static char *req;
static volatile pid_t player_pid = -1;
static volatile pid_t mixer_pid = -1;

static int kill_zombies(void *prm) {
  pid_t pid;
  while (1) {
    pid = wait(NULL);
    if (pid == mixer_pid)
      mixer_pid = -1;
    else if (pid == player_pid)
      player_pid = -1;
  }
}

static inline void selector(int sock) {
  ssize_t read_size;
  pid_t pid;
  char sock_txt[15];
  char *url;
  char *end;
  sprintf(sock_txt, "%d", sock);
  read_size = read(sock, req, msg_size);
  if (read_size < 5 || strncmp(req, "GET ", 4))
    goto exit;
  url = req + 4;
  end = strchr(url, ' ');
  if (end)
    *end = '\0';
  else
    goto exit;
  if (!strncmp(music_path, url, strlen(music_path))) {
    pid = fork();
    if (!pid)
      execl(data_picture, "data_picture", sock_txt, url, NULL);
  } else if (!strncmp("/tracks", url, strlen("/tracks"))) {
    pid = fork();
    if (!pid)
      execl(html_tracks, "html_tracks", sock_txt, url, NULL);
  } else if (!strncmp("/albums", url, strlen("/albums"))) {
    pid = fork();
    if (!pid)
      execl(html_albums, "html_albums", sock_txt, NULL);
  } else if (!strncmp("/play", url, strlen("/play"))) {
    if (player_pid > 0) {
      kill(player_pid, SIGTERM);
      while (player_pid > 0)
        ;
    }
    player_pid = fork();
    if (!player_pid)
      execl(system_play_flac, "system_play_flac", sock_txt, url, NULL);
    if (player_pid > 0)
      setpriority(PRIO_PROCESS, player_pid, PRIO_MIN);
  } else if (!(strcmp("/getvolume", url) &&
               strncmp("/setvolume", url, strlen("/setvolume")))) {
    mixer_pid = fork();
    if (!mixer_pid)
      execl(system_volume, "system_volume", sock_txt, url, NULL);
    while (mixer_pid > 0)
      ;
  } else if (!strcmp("/poweroff", url)) {
    if (player_pid > 0) {
      kill(player_pid, SIGTERM);
      while (player_pid > 0)
        ;
    }
    pid = fork();
    if (!pid) {
      if (system("/init.sh finish"))
        execl(resp_err, "resp_err", sock_txt, NULL);
      else
        execl(system_poweroff, "system_poweroff", sock_txt, NULL);
    }
  } else if (!strncmp("/setdate", url, strlen("/setdate"))) {
    pid = fork();
    if (!pid) {
      execl(system_setdate, "system_setdate", sock_txt, url, NULL);
    }
  } else {
    pid = fork();
    if (!pid)
      execl(data_static, "data_static", sock_txt, url, NULL);
  }
exit:
  close(sock);
}

static inline int init_socket(int *sock_listen, struct sockaddr_in *addr,
                              socklen_t *addr_size) {
  *sock_listen = socket(PF_INET, SOCK_STREAM, 0);
  addr->sin_family = AF_INET;
  addr->sin_port = htons(listen_port);
  addr->sin_addr.s_addr = htonl(INADDR_ANY);
  *addr_size = sizeof(struct sockaddr_in);
  if (bind(*sock_listen, (struct sockaddr *)addr, *addr_size) < 0 ||
      listen(*sock_listen, 20) < 0)
    return 1;
  return 0;
}

int main(void) {
#ifdef PLAYER_AS_INIT
  if (system("/init.sh") && system("poweroff -f"))
    return 1;
#endif
  int sock_listen;
  int sock;
  struct sockaddr_in addr;
  socklen_t addr_size;
  thrd_t thr;
  msg_size = getpagesize();
  req = malloc(msg_size);
  if (thrd_create(&thr, kill_zombies, NULL) != thrd_success)
#ifdef PLAYER_AS_INIT
    if (system("poweroff -f"))
#endif
      return 1;
  if (init_socket(&sock_listen, &addr, &addr_size))
#ifdef PLAYER_AS_INIT
    if (system("poweroff -f"))
#endif
      return 1;
  while (1) {
    sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size);
    if (sock < 0)
      continue;
    selector(sock);
  }
}
