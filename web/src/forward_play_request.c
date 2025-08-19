#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

int forward_request(struct sockaddr_in *addr, char *msg) {
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (!connect(sock, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) &&
      write(sock, msg, strlen(msg)) == strlen(msg))
    return 1;
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char rsp[getpagesize()];
  char msg[getpagesize()];
  struct hostent *host;
  char streamer_address[INET_ADDRSTRLEN];
  char *path_track = strchr(prm[2], '?') + 1;
  char *end = strchr(path_track, '&');
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  if (!strncmp("/playflac", prm[2], strlen("/playflac")))
    sprintf(msg, "/stream_album?%s", path_track);
  else
    sprintf(msg, "/stream_cd?%s", path_track);
  if (inet_aton(prm[3], &addr.sin_addr)) {
    addr.sin_port = htons(android_client_port);
    if (forward_request(&addr, msg))
      goto ok;
  }
  if ((host = gethostbyname(streamer_host))) {
    addr.sin_addr = *(struct in_addr *)host->h_addr;
    addr.sin_port = htons(streamer_port);
    strcat(msg, " \r\n\r\n");
    if (forward_request(&addr, msg))
      goto ok;
  }
  execl(resp_err, "resp_err", prm[1], NULL);
ok:
  if (end) {
    *end = '\0';
    if (utime(path_track, NULL))
      execl(resp_err, "resp_err", prm[1], NULL);
  }
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  if (write(sock, rsp, strlen(rsp)) != strlen(rsp))
    return 1;
  else
    return 0;
}
