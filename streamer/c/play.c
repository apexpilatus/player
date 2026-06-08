#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
  ssize_t msg_size = getpagesize();
  ssize_t read_size = 0;
  char *hdr = malloc(msg_size);
  printf("c start\n");
  while (read_size < msg_size && fread(hdr + read_size, 1, 1, stdin) == 1) {
    read_size++;
    if (read_size < msg_size)
      hdr[read_size] = '\0';
    if (read_size > 3 && !strcmp(hdr + read_size - 4, "\r\n\r\n"))
      break;
  }
  printf("c after while\n");
  if (read_size == msg_size || read_size < 5)
    return 1;
  else
    printf("%s", hdr);
  printf("c end\n");

  return 1;
}
