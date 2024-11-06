#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t read_size;
  ssize_t msg_size = getpagesize() * 100;
  char *url;
  char *end;
  int fd;
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
  } else if (!(strcmp("/getvolume", url) &&
               strncmp("/setvolume", url, strlen("/setvolume")))) {
    execl(system_volume, "system_volume", prm[1], url, NULL);
  } else if (!strncmp("/play", url, strlen("/play"))) {
    pid_t pid = -1;
    fd = open(play_pid_path, O_RDONLY);
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
        execl(resp_err, "resp_err", prm[1], NULL);
    }
    if (!strncmp("/playflac", url, strlen("/playflac")))
      execl(system_play_flac, "system_play_flac", prm[1], url, NULL);
    else
      execl(system_play_cd, "system_play_cd", prm[1], url, NULL);
  } else if (!strcmp("/poweroff", url)) {
    pid_t pid = -1;
    fd = open(play_pid_path, O_RDONLY);
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
        execl(resp_err, "resp_err", prm[1], NULL);
    }
    if (system("/init.sh finish"))
      execl(resp_err, "resp_err", prm[1], NULL);
    else
      execl(system_poweroff, "system_poweroff", prm[1], NULL);
  } else if (!strncmp("/setdate", url, strlen("/setdate"))) {
    execl(system_setdate, "system_setdate", prm[1], url, NULL);
  } else if (!strcmp("/cdcontrol", url)) {
    execl(html_cd_control, "html_cd_control", prm[1], NULL);
  } else {
    execl(data_static, "data_static", prm[1], url, NULL);
  }
}