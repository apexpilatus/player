#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  int fd;
  struct stat stat_buf;
  char *data;
  ssize_t read_size;
  ssize_t write_size;
  char *rsp = malloc(getpagesize());
  if (stat(prm[2], &stat_buf))
    execl(resp_err, "resp_err", prm[1], NULL);
  data = malloc(stat_buf.st_size);
  if ((fd = open(prm[2], O_RDONLY)) >= 0) {
    read_size = read(fd, data, stat_buf.st_size);
    close(fd);
    if (read_size != stat_buf.st_size)
      execl(resp_err, "resp_err", prm[1], NULL);
  } else
    execl(resp_err, "resp_err", prm[1], NULL);
  sprintf(rsp, "%s\r\n%s%ld\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
          "Content-Length: ", read_size, "Content-Type: audio/x-flac");
  write_size = write(sock, rsp, strlen(rsp));
  write_size += write(sock, data, read_size);
  if (write_size == strlen(rsp) + read_size)
    return 0;
  else
    return 1;
}