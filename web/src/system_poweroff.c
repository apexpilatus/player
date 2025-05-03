#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char rsp[getpagesize()];
  char cmd[getpagesize()];
  struct hostent *host;
  char streamer_address[INET_ADDRSTRLEN];
  sprintf(rsp, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
          "Content-Type: text/html; charset=utf-8", "Cache-control: no-cache",
          "X-Content-Type-Options: nosniff");
  write_size = write(sock, rsp, strlen(rsp));
  close(sock);
  if (write_size == strlen(rsp)) {
    if ((host = gethostbyname(streamer_host))) {
      inet_ntop(AF_INET, (struct in_addr *)host->h_addr, streamer_address,
                INET_ADDRSTRLEN);
      sprintf(cmd,
              "echo \"/poweroff\r\n\r\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
              streamer_address, streamer_port);
      if (!system(cmd))
        return 0;
    }
    if (!system("poweroff"))
      return 0;
  }
  return 1;
}
