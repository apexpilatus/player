#include <stdio.h>

int err(void) {
  printf("%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 404 shit happens",
         "Cache-control: no-cache", "X-Content-Type-Options: nosniff");
  return 1;
}

int main(void) { return err(); }