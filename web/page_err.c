#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = 4096, write_size;
  char rsp[rsp_size];
  strcpy(rsp, "HTTP/1.1 404 Not Found");
  write_size = write(sock, rsp, strlen(rsp));
  kill(getppid(), SIGUSR1);
  if (write_size == strlen(rsp)) {
    return 0;
  } else {
    return 1;
  }
}
