#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char rsp[getpagesize()];
  sprintf(rsp, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
          "Content-Type: text/html; charset=utf-8", "Cache-control: no-cache",
          "X-Content-Type-Options: nosniff");
  if (write(sock, rsp, strlen(rsp)) == strlen(rsp)) {
    close(sock);
    if (!system("poweroff"))
      return 0;
  }
  return 1;
}
