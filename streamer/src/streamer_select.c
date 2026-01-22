#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
  char *req = malloc(msg_size);
  char *end;
  while (read_size < msg_size && read(sock, req + read_size, 1) == 1) {
    read_size++;
    if (read_size < msg_size)
      req[read_size] = '\0';
    if (read_size > 3 && !strcmp(req + read_size - 4, "\r\n\r\n"))
      break;
  }
  if (read_size == msg_size || read_size < 5)
    return 1;
  if (!strncmp(req, "GET ", 4))
    req += 4;
  if ((end = strchr(req, ' ')) || (end = strchr(req, '\r')) ||
      (end = strchr(req, '\n')))
    *end = '\0';
  if (!strcmp("/", req)) {
    struct stat stat_buf;
    while (!stat(mix_pid_path, &stat_buf))
      ;
    execl(html_volume, "html_volume", prm[1], NULL);
  } else if (!strcmp("/store", req)) {
    execl(html_store, "html_store", prm[1], NULL);
  } else if (!strncmp("/setvolume", req, strlen("/setvolume"))) {
    struct stat stat_buf;
    while (!stat(mix_pid_path, &stat_buf))
      ;
    execl(system_volume, "system_volume", prm[1], req, NULL);
  } else if (!strncmp("/stream", req, strlen("/stream"))) {
    if (stop_playing())
      return 1;
    execl(system_play, "system_play", prm[1], req, NULL);
  } else {
    execl(data_static, "data_static", prm[1], req, NULL);
  }
}
