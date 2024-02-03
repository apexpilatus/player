#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t msg_size = 4096, read_size, write_size;
  char req[msg_size], rsp[msg_size];
  pid_t pid;
  read_size = read(sock, req, msg_size);
  req[read_size] = '\0';
  if (!strncmp("GET / ", req, 6)) {
    execl(page_main, "page_main", prm[1], NULL);
  } else if (!strncmp("GET /favicon.ico ", req, 17)) {
    execl(picture_favicon, "picture_favicon", prm[1], NULL);
  } else if (!strncmp("GET /apple-touch-icon-precomposed.png ", req, 17)) {
    execl(picture_favicon, "picture_favicon", prm[1], NULL);
  } else {
    strcpy(rsp, "HTTP/1.1 404 Not Found");
    write_size = write(sock, rsp, strlen(rsp));
    kill(getppid(), SIGUSR1);
    if (write_size == strlen(rsp)) {
      return 0;
    } else {
      return 1;
    }
  }
}
