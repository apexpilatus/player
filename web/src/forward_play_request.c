#include "lib_flac_tracks.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

static void android_cmd(char *cmd, track_list *tracks, char *client_address) {
  size_t cmd_end;
  strcat(cmd, "echo \"");
  while (tracks) {
    strcat(cmd, "$(pwd)/");
    strcat(cmd, tracks->file_name);
    if (tracks->next)
      strcat(cmd, "|");
    tracks = tracks->next;
  }
  cmd_end = strlen(cmd);
  sprintf(cmd + cmd_end, "\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
          client_address, android_client_port);
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char rsp[getpagesize()];
  char cmd[getpagesize()];
  struct hostent *host;
  char streamer_address[INET_ADDRSTRLEN];
  if (!strncmp("/playflac", prm[2], strlen("/playflac"))) {
    char url[getpagesize()];
    track_list *tracks;
    strcpy(url, prm[2]);
    tracks = get_tracks_in_dir(url);
    if (!tracks)
      execl(resp_err, "resp_err", prm[1], NULL);
    android_cmd(cmd, tracks, prm[3]);
    if (!system(cmd)) {
      if (utime(".", NULL))
        execl(resp_err, "resp_err", prm[1], NULL);
      goto ok;
    }
    if ((host = gethostbyname(streamer_host))) {
      inet_ntop(AF_INET, (struct in_addr *)host->h_addr, streamer_address,
                INET_ADDRSTRLEN);
      sprintf(cmd,
              "echo \"/stream_album%s\r\n\r\"|nc -w 1 %s %s 1>/dev/null "
              "2>/dev/null",
              strchr(prm[2], '?'), streamer_address, streamer_port);
      if (!system(cmd)) {
        if (utime(".", NULL))
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
          "echo \"/stream_cd?%s\r\n\r\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
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