#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize(), write_size;
  char *rsp = malloc(rsp_size);
  strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Length: 8\r\n\r\npoweroff");
  write_size = write(sock, rsp, strlen(rsp));
  close(sock);
  if (write_size == strlen(rsp)) {
#ifdef WEB_INIT
    if (!system("poweroff -f"))
#endif
      return 0;
  } else
    return 1;
}
