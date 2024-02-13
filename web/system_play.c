#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize(), write_size;
  char *rsp = malloc(rsp_size);
  strcpy(rsp, "HTTP/1.1 200 OK\r\n\r\n");
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size == strlen(rsp))
    return 0;
  else
    return 1;
}
