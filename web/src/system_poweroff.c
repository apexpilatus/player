#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char rsp[getpagesize()];
  struct hostent *host;
  sprintf(rsp, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
          "Content-Type: text/html; charset=utf-8", "Cache-control: no-cache",
          "X-Content-Type-Options: nosniff");
  write_size = write(sock, rsp, strlen(rsp));
  close(sock);
  if (write_size == strlen(rsp)) {
    struct sockaddr_in addr;
    char streamer_N[strlen(streamer_host) + 11];
    int streamer_index = 0;
    addr.sin_family = AF_INET;
    strcpy(streamer_N, streamer_host);
    while ((host = gethostbyname(streamer_N))) {
      addr.sin_addr = *(struct in_addr *)host->h_addr;
      addr.sin_port = htons(streamer_port);
      sock = socket(PF_INET, SOCK_STREAM, 0);
      if (!connect(sock, (struct sockaddr *)&addr,
                   sizeof(struct sockaddr_in)) &&
          write(sock, prm[2], strlen(prm[2])) == strlen(prm[2]))
        return 0;
      sprintf(streamer_N, "%s%d", streamer_host, ++streamer_index);
    }
    if (!system("poweroff"))
      return 0;
  }
  return 1;
}
