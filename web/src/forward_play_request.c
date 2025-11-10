#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

int forward_request(struct sockaddr_in6 *addr, char *msg) {
  int sock = socket(PF_INET6, SOCK_STREAM, 0);
  if (!connect(sock, (struct sockaddr *)addr, sizeof(struct sockaddr_in6)) &&
      write(sock, msg, strlen(msg)) == strlen(msg))
    return 1;
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char rsp[getpagesize()];
  char msg[getpagesize()];
  struct hostent *host;
  char *path_track = strchr(prm[2], '?') + 1;
  char *end = strchr(path_track, '&');
  struct sockaddr_in6 addr;
  char streamer_N[strlen(streamer_host) + 11];
  int streamer_index = 0;
  addr.sin6_family = AF_INET6;
  addr.sin6_flowinfo = 0;
  addr.sin6_scope_id = strtol(prm[3], NULL, 10);
  if (!strncmp("/playflac", prm[2], strlen("/playflac")))
    sprintf(msg, "/stream_album?%s", path_track);
  else
    sprintf(msg, "/stream_cd?%s", path_track);
  strcpy(streamer_N, streamer_host);
  while ((host = gethostbyname2(streamer_N, AF_INET6))) {
    addr.sin6_addr = *(struct in6_addr *)host->h_addr;
    addr.sin6_port = htons(streamer_port);
    if (forward_request(&addr, msg))
      goto ok;
    sprintf(streamer_N, "%s%d", streamer_host, ++streamer_index);
  }
  execl(resp_err, "resp_err", prm[1], NULL);
ok:
  if (end)
    *end = '\0';
  if (utime(path_track, NULL))
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
