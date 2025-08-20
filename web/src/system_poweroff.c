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
  char hostname[getpagesize()];
  sprintf(rsp, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
          "Content-Type: text/html; charset=utf-8", "Cache-control: no-cache",
          "X-Content-Type-Options: nosniff");
  write_size = write(sock, rsp, strlen(rsp));
  close(sock);
  if (write_size == strlen(rsp)) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    if ((host = gethostbyname(streamer_host))) {
      addr.sin_addr = *(struct in_addr *)host->h_addr;
      addr.sin_port = htons(streamer_port);
      sock = socket(PF_INET, SOCK_STREAM, 0);
      if (!connect(sock, (struct sockaddr *)&addr,
                   sizeof(struct sockaddr_in)) &&
          write(sock, prm[2], strlen(prm[2])) == strlen(prm[2]))
        return 0;
    }
    if (!gethostname(hostname, getpagesize()) &&
        (host = gethostbyname(hostname))) {
      addr.sin_addr = *(struct in_addr *)host->h_addr;
      addr.sin_port = htons(streamer_port);
      sock = socket(PF_INET, SOCK_STREAM, 0);
      if (!connect(sock, (struct sockaddr *)&addr,
                   sizeof(struct sockaddr_in)) &&
          write(sock, prm[2], strlen(prm[2])) == strlen(prm[2]))
        return 0;
    }
    if (!system("poweroff"))
      return 0;
  }
  return 1;
}
