#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int stop_playing() {
  pid_t pid = -1;
  ssize_t read_size;
  int fd = open(play_pid_path, O_RDONLY);
  if (fd >= 0) {
    read_size = read(fd, &pid, sizeof(pid_t));
    if (read_size != sizeof(pid_t))
      return 1;
    kill(pid, SIGTERM);
    close(fd);
  }
  while ((fd = open(play_pid_path, O_RDONLY)) >= 0) {
    pid_t pid_check;
    read_size = read(fd, &pid_check, sizeof(pid_t));
    close(fd);
    if (pid_check != pid || read_size != sizeof(pid_t))
      return 1;
  }
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t read_size = 0;
  ssize_t msg_size = getpagesize() * 100;
  char *end;
  char *range;
  char *agent;
  char *url = malloc(msg_size);
  while (read_size < msg_size && read(sock, url + read_size, 1) == 1) {
    read_size++;
    if (read_size < msg_size)
      url[read_size] = '\0';
    if (read_size > 3 && !strcmp(url + read_size - 4, "\r\n\r\n"))
      break;
  }
  if (read_size == msg_size || read_size < 5 || strncmp(url, "GET", 3))
    return 1;
  agent = strstr(url, "User-Agent:");
  range = strstr(url, "Range:");
  if (range) {
    range += 6;
    while (*range == ' ')
      range++;
    range += 6;
    end = strstr(range, "\r\n");
    *end = '\0';
  }
  if (agent)
    end = strstr(agent, "\r\n");
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
    execl(data_picture, "data_picture", prm[1], url, NULL);
  } else if (!strncmp("/tracks", url, strlen("/tracks"))) {
    execl(html_tracks, "html_tracks", prm[1], url,
          agent && strstr(agent, "Android") ? "no" : "yes", NULL);
  } else if (!strcmp("/", url)) {
    execl(html_main, "html_main", prm[1], NULL);
  } else if (!strcmp("/cdcontrol", url)) {
    execl(html_cd_control, "html_cd_control", prm[1],
          agent && strstr(agent, "Android") ? "no" : "yes", NULL);
  } else if (!strncmp("/albums", url, strlen("/albums"))) {
    execl(html_albums, "html_albums", prm[1], url, NULL);
  } else if (!strncmp("/stream_cd", url, strlen("/stream_cd"))) {
    if (stop_playing())
      execl(resp_err, "resp_err", prm[1], NULL);
    execl(data_cd, "data_cd", prm[1], url, range ? range : "", NULL);
  } else if (!strncmp("/stream_album", url, strlen("/stream_album"))) {
    execl(data_flac_extracted, "data_flac_extracted", prm[1], url,
          range ? range : "", NULL);
  } else if (!strncmp("/play", url, strlen("/play"))) {
    char *addr_v6 = prm[2];
    char *addr_v4 = addr_v6;
    while ((addr_v6 = strchr(addr_v4, ':')))
      addr_v4 = ++addr_v6;
    execl(forward_play_request, "forward_play_request", prm[1], url, addr_v4,
          NULL);
  } else if (!strcmp("/poweroff", url)) {
    if (stop_playing() || system("/root/init.sh finish"))
      execl(resp_err, "resp_err", prm[1], NULL);
    execl(system_poweroff, "system_poweroff", prm[1], url, NULL);
  } else if (!strcmp("/cdcontrol", url)) {
    execl(html_cd_control, "html_cd_control", prm[1], NULL);
  } else {
    execl(data_static, "data_static", prm[1], url, NULL);
  }
}
