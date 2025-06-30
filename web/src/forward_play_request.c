#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char rsp[getpagesize()];
  char cmd[getpagesize()];
  struct hostent *host;
  char streamer_address[INET_ADDRSTRLEN];
  char *path_track = strchr(prm[2], '?') + 1;
  if (!strncmp("/playflac", prm[2], strlen("/playflac"))) {
    sprintf(cmd,
            "echo \"/stream_album?%s\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
            path_track, prm[3], android_client_port);
    if (!system(cmd)) {
      char *end = strchr(path_track, '&');
      if (end)
        *end = '\0';
      if (utime(path_track, NULL))
        execl(resp_err, "resp_err", prm[1], NULL);
      goto ok;
    }
    if ((host = gethostbyname(streamer_host))) {
      inet_ntop(AF_INET, (struct in_addr *)host->h_addr, streamer_address,
                INET_ADDRSTRLEN);
      sprintf(cmd, "echo \"/stream_album?%s \r\n\r\"|nc -w 1 %s %s %s",
              path_track, streamer_address, streamer_port,
              "1>/dev/null 2>/dev/null");
      if (!system(cmd)) {
        char *end = strchr(path_track, '&');
        if (end)
          *end = '\0';
        if (utime(path_track, NULL))
          execl(resp_err, "resp_err", prm[1], NULL);
        goto ok;
      }
    }
  }
  if (!strncmp("/playcd", prm[2], strlen("/playcd"))) {
    char *url_track = strchr(prm[2], '?');
    sprintf(cmd, "echo \"/stream_cd?%s\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
            url_track ? url_track + 1 : "1", prm[3], android_client_port);
    if (!system(cmd))
      goto ok;
    if ((host = gethostbyname(streamer_host))) {
      inet_ntop(AF_INET, (struct in_addr *)host->h_addr, streamer_address,
                INET_ADDRSTRLEN);
      sprintf(
          cmd,
          "echo \"/stream_cd?%s \r\n\r\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
          url_track ? url_track + 1 : "1", streamer_address, streamer_port);
      if (!system(cmd))
        goto ok;
    }
  }
  execl(resp_err, "resp_err", prm[1], NULL);
ok:
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  if (write(sock, rsp, strlen(rsp)) != strlen(rsp))
    return 1;
  else
    return 0;
}