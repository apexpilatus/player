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
    struct sockaddr_in6 addr;
    char streamer_N[strlen(streamer_host) + 11];
    int streamer_index = 0;
    addr.sin6_family = AF_INET6;
    addr.sin6_flowinfo = 0;
    addr.sin6_scope_id = strtol(prm[3], NULL, 10),
    strcpy(streamer_N, streamer_host);
    while ((host = gethostbyname2(streamer_N, AF_INET6))) {
      addr.sin6_addr = *(struct in6_addr *)host->h_addr;
      addr.sin6_port = htons(streamer_port);
      sock = socket(PF_INET6, SOCK_STREAM, 0);
      if (!connect(sock, (struct sockaddr *)&addr, sizeof(addr)) &&
          write(sock, prm[2], strlen(prm[2])) == strlen(prm[2]))
        return 0;
      sprintf(streamer_N, "%s%d", streamer_host, ++streamer_index);
    }
    if (!system("poweroff"))
      return 0;
  }
  return 1;
}
