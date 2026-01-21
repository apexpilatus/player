#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char rsp[getpagesize()];
  char *path = strchr(prm[2], '?') + 1;
  char *end = strchr(path, '&');
  if (end) {
    *end = '\0';
  }
  if (utime(path, NULL))
    execl(resp_err, "resp_err", prm[1], NULL);
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  if (write(sock, rsp, strlen(rsp)) != strlen(rsp))
    return 1;
  else
    return 0;
}
