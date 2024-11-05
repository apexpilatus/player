#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t read_size;
  ssize_t msg_size = getpagesize();
  char *url;
  char *end;
  char req[msg_size];
  read_size = read(sock, req, msg_size);
  if (read_size < 5 || strncmp(req, "GET ", 4))
    return 1;
  url = req + 4;
  end = strchr(url, ' ');
  if (end)
    *end = '\0';
  else
    return 1;
  if (!strncmp(music_path, url, strlen(music_path))) {
    execl(data_picture, "data_picture", prm[1], url, NULL);
  } else if (!strncmp("/tracks", url, strlen("/tracks"))) {
    execl(html_tracks, "html_tracks", prm[1], url, NULL);
  } else if (!strcmp("/cdcontrol", url)) {
    execl(html_cd_control, "html_cd_control", prm[1], NULL);
  } else if (!strncmp("/albums", url, strlen("/albums"))) {
    execl(html_albums, "html_albums", prm[1], url, NULL);
  } else if (!strncmp("/setdate", url, strlen("/setdate"))) {
    execl(system_setdate, "system_setdate", prm[1], url, NULL);
  } else {
    execl(data_static, "data_static", prm[1], url, NULL);
  }
}