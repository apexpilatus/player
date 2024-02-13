#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <threads.h>
#include <unistd.h>

#define listen_port 8080

static ssize_t msg_size;
static char *req;
static volatile pid_t player_pid = -1;

int kill_zombies(void *prm) {
  pid_t pid;
  while (1) {
    pid = wait(NULL);
    if (pid == player_pid)
      player_pid = -1;
  }
}

static inline void selector(int sock) {
  ssize_t read_size;
  pid_t pid;
  char sock_txt[15], *url, *end;
  sprintf(sock_txt, "%d", sock);
  read_size = read(sock, req, msg_size);
  url = req + 4;
  end = strchr(url, ' ');
  if (end)
    *end = '\0';
  if (!strncmp(music, url, strlen(music))) {
    pid = fork();
    if (!pid)
      execl(data_picture, "data_picture", sock_txt, url, NULL);
  } else if (!strncmp("/tracks", url, strlen("/tracks"))) {
    pid = fork();
    if (!pid)
      execl(resp_tracks, "resp_tracks", sock_txt, url, NULL);
  } else if (!strcmp("/", url)) {
    pid = fork();
    if (!pid)
      execl(resp_main, "resp_main", sock_txt, NULL);
  } else if (!strncmp("/albums", url, strlen("/albums"))) {
    pid = fork();
    if (!pid)
      execl(resp_albums, "resp_albums", sock_txt, NULL);
  } else if (!strncmp("/play", url, strlen("/play"))) {
    if (player_pid > 0) {
      kill(player_pid, SIGTERM);
      while (player_pid > 0)
        ;
    }
    player_pid = fork();
    if (!player_pid)
      execl(system_play, "system_play", sock_txt, url, NULL);
  } else if (!(strcmp("/favicon.ico", url) &&
               strcmp("/apple-touch-icon-precomposed.png", url) &&
               strncmp("/style", url, strlen("/style")) &&
               strncmp("/script", url, strlen("/script")))) {
    pid = fork();
    if (!pid)
      execl(data_static, "data_static", sock_txt, url, NULL);
  } else if (!strcmp("/poweroff", url)) {
    pid = fork();
    if (!pid)
      execl(system_poweroff, "system_poweroff", sock_txt, NULL);
  } else {
    pid = fork();
    if (!pid)
      execl(resp_err, "resp_err", sock_txt, NULL);
  }
  close(sock);
}

int main(void) {
#ifdef WEB_INIT
  if (system("/init.sh") && system("poweroff -f"))
    return 1;
#endif
  int sock_listen, sock;
  struct sockaddr_in addr;
  thrd_t thr;
  msg_size = getpagesize();
  req = malloc(msg_size);
  if (thrd_create(&thr, kill_zombies, NULL) != thrd_success)
#ifdef WEB_INIT
    if (system("poweroff -f"))
#endif
      return 1;
  sock_listen = socket(PF_INET, SOCK_STREAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(listen_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  socklen_t addr_size = sizeof(addr);
  if (bind(sock_listen, (struct sockaddr *)&addr, addr_size) < 0)
#ifdef WEB_INIT
    if (system("poweroff -f"))
#endif
      return 1;
  if (listen(sock_listen, 100) < 0)
#ifdef WEB_INIT
    if (system("poweroff -f"))
#endif
      return 1;
  while (1) {
    sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size);
    if (sock < 0)
      continue;
    selector(sock);
  }
#ifdef WEB_INIT
  if (system("poweroff -f"))
#endif
    return 1;
}
