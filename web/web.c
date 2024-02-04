#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define listen_port 8080

typedef struct pid_lst_t {
  pid_t pid;
  int sock;
  struct pid_lst_t *prev;
  struct pid_lst_t *next;
} pid_lst;

pid_lst *pids_first = NULL, *pids_last = NULL;

void kill_zombie(int signum) {
  pid_t pid = wait(NULL);
  pid_lst *pid_tmp = pids_first;
  while (pid_tmp) {
    if (pid_tmp->pid == pid) {
      if (pid_tmp->prev) {
        pid_tmp->prev->next = pid_tmp->next;
      } else {
        pids_first = pid_tmp->next;
      }
      if (pid_tmp->next) {
        pid_tmp->next->prev = pid_tmp->prev;
      } else {
        pids_last = pid_tmp->prev;
      }
      close(pid_tmp->sock);
      free(pid_tmp);
      break;
    }
    pid_tmp = pid_tmp->next;
  }
}

void selector(int sock){
  ssize_t msg_size = 4096, read_size;
  char req[msg_size];
}

int main(void) {
  sigset_t block_alarm;
  pid_t pid;
  char arg[12];
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
  if (bind(sock_listen, (struct sockaddr *)&addr, addr_size) < 0) {
    return 1;
  }
  if (listen(sock_listen, 10) < 0) {
    return 1;
  }
  while (1) {
    sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size);
    if (sock < 0) {
      continue;
    }
    sigprocmask(SIG_BLOCK, &block_alarm, NULL);
    sprintf(arg, "%d", sock);
    pid = fork();
    if (!pid) {
      execl(selector, "selector", arg, NULL);
    }
    if (pid > 0) {
      pid_lst *pids_new = malloc(sizeof(pid_lst));
      if (!pids_first) {
        pids_first = pids_new;
        pids_new->prev = NULL;
      } else {
        pids_last->next = pids_new;
        pids_new->prev = pids_last;
      }
      pids_last = pids_new;
      pids_last->pid = pid;
      pids_last->sock = sock;
      pids_last->next = NULL;
    }
    sigprocmask(SIG_UNBLOCK, &block_alarm, NULL);
  }
  return 1;
}
