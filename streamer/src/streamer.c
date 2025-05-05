#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#define listen_port 8888

static ssize_t msg_size;
static char *req;
static volatile pid_t player_pid = -1;
static volatile pid_t mixer_pid = -1;

static void kill_zombies(int signum) {
  pid_t pid;
  while ((pid = waitpid(WAIT_ANY, NULL, WNOHANG)) > 0) {
    if (pid == mixer_pid)
      mixer_pid = -1;
    else if (pid == player_pid)
      player_pid = -1;
  }
}

static void selector(int sock, struct sockaddr_in *addr) {
  ssize_t read_size = 0;
  pid_t pid;
  char sock_txt[15];
  char *end;
  sprintf(sock_txt, "%d", sock);
  while (read_size < msg_size && read(sock, req + read_size, 1) == 1) {
    read_size++;
    if (read_size < msg_size)
      req[read_size] = '\0';
    if (read_size > 3 && !strcmp(req + read_size - 4, "\r\n\r\n"))
      break;
  }
  if (read_size == msg_size || read_size < 5)
    goto exit;
  if (!strncmp(req, "GET ", 4))
    req += 4;
  if ((end = strchr(req, ' ')) || (end = strchr(req, '\n')))
    *end = '\0';
  if (!strcmp("/getvolume", req)) {
    while (mixer_pid > 0)
      ;
    mixer_pid = fork();
    if (!mixer_pid)
      execl(html_volume, "html_volume", sock_txt, req, NULL);
  } else if (!strncmp("/setvolume", req, strlen("/setvolume"))) {
    while (mixer_pid > 0)
      ;
    mixer_pid = fork();
    if (!mixer_pid)
      execl(system_volume, "system_volume", sock_txt, req, NULL);
  } else if (!strncmp("/stream", req, strlen("/stream"))) {
    if (player_pid > 0) {
      kill(player_pid, SIGTERM);
      while (player_pid > 0)
        ;
    }
    player_pid = fork();
    if (!player_pid) {
      char client_address[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &addr->sin_addr, client_address, INET_ADDRSTRLEN);
      execl(system_play, "system_play", sock_txt, req, client_address, NULL);
    }
  } else if (!strncmp("/poweroff", req, strlen("/poweroff"))) {
    if (player_pid > 0) {
      kill(player_pid, SIGTERM);
      while (player_pid > 0)
        ;
    } else if (!system("/root/init.sh finish")) {
      pid = fork();
      if (!pid)
        execl(system_poweroff, "system_poweroff", sock_txt, NULL);
    }
  } else {
    pid = fork();
    if (!pid)
      execl(data_static, "data_static", sock_txt, req, NULL);
  }
exit:
  close(sock);
}

static int init_socket(int *sock_listen, struct sockaddr_in *addr,
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
  int sock_listen;
  int sock;
  struct sockaddr_in addr;
  socklen_t addr_size;
  signal(SIGCHLD, kill_zombies);
  msg_size = getpagesize() * 100;
  req = malloc(msg_size);
#ifdef PLAYER_AS_INIT
  if (system("/init.sh") && system("poweroff"))
    return 1;
#endif
  if (init_socket(&sock_listen, &addr, &addr_size))
#ifdef PLAYER_AS_INIT
    if (system("poweroff"))
#endif
      return 1;
  setpriority(PRIO_PROCESS, getpid(), PRIO_MIN);
  while (1) {
    sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size);
    if (sock < 0)
      continue;
    selector(sock, &addr);
  }
}
