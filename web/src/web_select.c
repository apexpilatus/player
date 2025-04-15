#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int stop_playing() {
  pid_t pid = -1;
  ssize_t read_size;
  int fd = open(play_pid_path, O_RDONLY);
  if (fd >= 0) {
    read_size = read(fd, &pid, sizeof(pid_t));
    kill(pid, SIGTERM);
    close(fd);
  }
  while ((fd = open(play_pid_path, O_RDONLY)) >= 0) {
    pid_t pid_check;
    read_size = read(fd, &pid_check, sizeof(pid_t));
    close(fd);
    if (pid_check != pid)
      return 1;
  }
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t read_size;
  ssize_t msg_size = getpagesize() * 100;
  char *end;
  char *host;
  char *range;
  char *url = malloc(msg_size);
  read_size = read(sock, url, msg_size);
  if (read_size < 5 || strncmp(url, "GET", 3))
    return 1;
  url[read_size] = '\0';
  range = strstr(url, "Range:");
  host = strstr(url, "Host:");
  if ((range)) {
    range += 6;
    while (*range == ' ')
      range++;
    range += 6;
    end = strstr(range, "\r\n");
    *end = '\0';
  }
  if ((host)) {
    host += 5;
    while (*host == ' ')
      host++;
    while ((end = strstr(host, "\r\n")) || (end = strchr(host, ':')))
      *end = '\0';
  }
  printf("range - %s; host - %s\n", range ? range : "", host);
  url += 3;
  while (*url == ' ') {
    url++;
  }
  end = strchr(url, ' ');
  if (end)
    *end = '\0';
  else
    return 1;
  if (!strncmp(music_path, url, strlen(music_path))) {
    if (strcmp(url + strlen(url) - 5, ".flac"))
      execl(data_picture, "data_picture", prm[1], url, NULL);
    execl(data_flac, "data_flac", prm[1], url, NULL);
  } else if (!strncmp("/tracks", url, strlen("/tracks"))) {
    execl(html_tracks, "html_tracks", prm[1], url, NULL);
  } else if (!strcmp("/cdcontrol", url)) {
    execl(html_cd_control, "html_cd_control", prm[1], NULL);
  } else if (!strncmp("/albums", url, strlen("/albums"))) {
    execl(html_albums, "html_albums", prm[1], url, NULL);
  } else if (!strcmp("/getvolume", url)) {
    execl(html_volume, "html_volume", prm[1], url, NULL);
  } else if (!strncmp("/setvolume", url, strlen("/setvolume"))) {
    execl(system_volume, "system_volume", prm[1], url, NULL);
  } else if (!strncmp("/stream_cd", url, strlen("/stream_cd"))) {
    if (stop_playing())
      execl(resp_err, "resp_err", prm[1], NULL);
    execl(data_cd, "data_cd", prm[1], url, range ? range : "", NULL);
  } else if (!strncmp("/play", url, strlen("/play"))) {
    if (stop_playing())
      execl(resp_err, "resp_err", prm[1], NULL);
    if (!strncmp("/playflac", url, strlen("/playflac")))
      execl(system_play_flac, "system_play_flac", prm[1], url, prm[2], NULL);
    execl(system_play_cd, "system_play_cd", prm[1], url, prm[2], NULL);
  } else if (!strcmp("/poweroff", url)) {
    if (stop_playing() || system("/root/init.sh finish"))
      execl(resp_err, "resp_err", prm[1], NULL);
    execl(system_poweroff, "system_poweroff", prm[1], NULL);
  } else if (!strncmp("/setdate", url, strlen("/setdate"))) {
    if (strcmp(prm[2], "127.0.0.1") && host && strcmp(host, "localhost") &&
        strcmp(host, "127.0.0.1") && strcmp(host, prm[2]))
      execl(system_setdate, "system_setdate", prm[1], url, NULL);
    execl(resp_err, "resp_err", prm[1], NULL);
  } else if (!strcmp("/cdcontrol", url)) {
    execl(html_cd_control, "html_cd_control", prm[1], NULL);
  } else {
    execl(data_static, "data_static", prm[1], url, NULL);
  }
}
