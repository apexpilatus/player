#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <threads.h>
#include <unistd.h>

#define listen_port 80

ssize_t msg_size;
char *req;

int kill_zombies(void *prm) {
  while (1)
    wait(NULL);
}

static inline void selector(int sock) {
  ssize_t read_size;
  pid_t pid = -1;
  char sock_txt[15];
  sprintf(sock_txt, "%d", sock);
  read_size = read(sock, req, msg_size);
  req[read_size] = '\0';
  if (!strncmp("GET / ", req, 6)) {
    pid = fork();
    if (!pid)
      execl(page_main, "page_main", sock_txt, NULL);
  } else if (!(strncmp("GET /favicon.ico ", req, 17) &&
               strncmp("GET /apple-touch-icon-precomposed.png ", req, 17))) {
    pid = fork();
    if (!pid)
      execl(picture_favicon, "picture_favicon", sock_txt, NULL);
  } else if (!strncmp("GET /poweroff ", req, 14)) {
    pid = fork();
    if (!pid)
      execl(system_poweroff, "system_poweroff", sock_txt, NULL);
  } else if (!strncmp(music, req + 4, strlen(music))) {
    char *end = strchr(req, '\r');
    while (strncmp(end, "HTTP", 4))
      end--;
    *(end - 1) = '\0';
    pid = fork();
    if (!pid)
      execl(picture_album, "picture_album", sock_txt, req + 4, NULL);
  } else {
    pid = fork();
    if (!pid)
      execl(page_err, "page_err", sock_txt, NULL);
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
