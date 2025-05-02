#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define store_port 80
#define store_host "store"

int main(void) {
  struct hostent *host;
  int sock;
  struct sockaddr_in addr;
  ssize_t msg_size = getpagesize() * 100;
  char *msg = malloc(msg_size);
  ssize_t write_size;
  ssize_t read_size = 0;
  if (!(host = gethostbyname(store_host)))
    return 1;
  sock = socket(PF_INET, SOCK_STREAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(store_port);
  addr.sin_addr = *(struct in_addr *)host->h_addr;
  if (0 > connect(sock, (struct sockaddr *)&addr, sizeof(addr)))
    return 1;
  strcpy(msg, "GET ");
  strcat(msg, "/stream_album?/home/music/nvme/Immaterial_Possession__1&3");
  strcat(msg, " HTTP/1.1\r\n\r\n");
  if ((write_size = write(sock, msg, strlen(msg))) != strlen(msg))
    return 1;
  msg[read_size] = '\0';
  while (read(sock, msg + read_size, 1) == 1) {
    read_size++;
    msg[read_size] = '\0';
    if (read_size > 3 && !strcmp(msg + read_size - 4, "\r\n\r\n")) {
      break;
    }
  }
  if (strncmp(msg, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK")))
    return 1;
  printf("%s\n", msg);
}
