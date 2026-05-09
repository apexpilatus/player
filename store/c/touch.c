#include <stdio.h>
#include <utime.h>

int main(int prm_n, char *prm[]) {
  if (utime(prm[1], NULL))
    return 1;
  printf("HTTP/1.1 200 OK\r\n%s\r\n%s\r\n%s\r\n\r\n",
         "Content-Type: text/html; charset=utf-8", "Cache-control: no-cache",
         "X-Content-Type-Options: nosniff");
  return 0;
}
