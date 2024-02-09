#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define listen_port 8080

void kill_zombie(int signum) {
  wait(NULL);
}

static inline void selector(int sock) {
  ssize_t msg_size = 4096, read_size;
  char req[msg_size];
  pid_t pid = -1;
  pid_lst *pids_new;
  char arg[15];
  sprintf(arg, "%d", sock);
  read_size = read(sock, req, msg_size);
  req[read_size] = '\0';
  if (!strncmp("GET / ", req, 6)) {
    pid = fork();
    if (!pid)
      execl(page_main, "page_main", arg, NULL);
  } else if (!(strncmp("GET /favicon.ico ", req, 17) &&
               strncmp("GET /apple-touch-icon-precomposed.png ", req, 17))) {
    pid = fork();
    if (!pid)
      execl(picture_favicon, "picture_favicon", arg, NULL);
  } else if (!strncmp("GET /poweroff ", req, 14)) {
    char rsp[msg_size];
    ssize_t write_size;
    strcpy(rsp, "HTTP/1.1 200 OK\r\n\r\n");
    write_size = write(sock, rsp, strlen(rsp));
    if (write_size == strlen(rsp))
#ifdef WEB_INIT
    {
      close(sock);
      if (system("poweroff -f"))
        ;
    }
#else
      ;
#endif
  } else {
    pid = fork();
    if (!pid)
      execl(page_err, "page_err", arg, NULL);
  }
  close(sock);
}

int main(void) {
#ifdef WEB_INIT
  if (system("/init.sh") && system("poweroff -f"))
    return 1;
#endif
  sigset_t block_alarm;
  int sock_listen, sock;
  struct sockaddr_in addr;
  sigemptyset(&block_alarm);
  sigaddset(&block_alarm, SIGUSR1);
  signal(SIGUSR1, kill_zombie);
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
  if (listen(sock_listen, 10) < 0)
#ifdef WEB_INIT
    if (system("poweroff -f"))
#endif
      return 1;
  while (1) {
    sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size);
    if (sock < 0)
      continue;
    sigprocmask(SIG_BLOCK, &block_alarm, NULL);
    selector(sock);
    sigprocmask(SIG_UNBLOCK, &block_alarm, NULL);
  }
#ifdef WEB_INIT
  if (system("poweroff -f"))
#endif
    return 1;
}
