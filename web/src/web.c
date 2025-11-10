#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

void kill_zombie(int signum) {
  pid_t pid;
  pid_t run_pid;
  int fd;
  ssize_t read_size;
  while ((pid = waitpid(WAIT_ANY, NULL, WNOHANG)) > 0) {
    fd = open(play_pid_path, O_RDONLY);
    if (fd >= 0) {
      read_size = read(fd, &run_pid, sizeof(pid_t));
      close(fd);
      if (read_size != sizeof(pid_t) || run_pid == pid) {
        unlink(play_pid_path);
        continue;
      }
    }
#ifdef mix_pid_path
    fd = open(mix_pid_path, O_RDONLY);
    if (fd >= 0) {
      read_size = read(fd, &run_pid, sizeof(pid_t));
      close(fd);
      if (run_pid == pid)
        unlink(mix_pid_path);
    }
#endif
  }
}

int init_socket(int *sock_listen, struct sockaddr_in6 *addr,
                socklen_t *addr_size) {
  *sock_listen = socket(PF_INET6, SOCK_STREAM, 0);
  addr->sin6_family = AF_INET6;
  addr->sin6_addr = in6addr_any;
  addr->sin6_flowinfo = 0;
  addr->sin6_scope_id = 0;
  addr->sin6_port = htons(listen_port);
  *addr_size = sizeof(struct sockaddr_in6);
  if (bind(*sock_listen, (struct sockaddr *)addr, *addr_size) < 0 ||
      listen(*sock_listen, 20) < 0)
    return 1;
  return 0;
}

int main(void) {
  pid_t pid;
  int sock_listen;
  int sock;
  struct sockaddr_in6 addr;
  socklen_t addr_size;
  signal(SIGCHLD, kill_zombie);
  unlink(play_pid_path);
  if (system("/root/init.sh"))
    while (1)
      ;
  if (init_socket(&sock_listen, &addr, &addr_size))
    return 1;
  while (1) {
    sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size);
    if (sock < 0)
      continue;
    pid = fork();
    if (!pid) {
      char sock_txt[15];
      char scope_txt[15];
      sprintf(sock_txt, "%d", sock);
      sprintf(scope_txt, "%d", addr.sin6_scope_id);
      execl(web_select, "web_select", sock_txt, scope_txt, NULL);
    }
    if (pid > 0)
      setpriority(PRIO_PROCESS, pid, PRIO_MIN);
    close(sock);
  }
}
