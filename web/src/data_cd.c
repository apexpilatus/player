#include "lib_read_cd.h"
#include <cdda_interface.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

static int write_header(int fd, int size) {
  int int_var;
  short short_var;
  ssize_t write_size = 0;
  int_var = size + 44 - 8;
  write_size = write(fd, "RIFF", 4);
  write_size += write(fd, &int_var, 4);
  write_size += write_size = write(fd, "WAVEfmt ", 8);
  int_var = 16;
  write_size += write(fd, &int_var, 4);
  short_var = 1;
  write_size += write(fd, &short_var, 2);
  short_var = 2;
  write_size += write(fd, &short_var, 2);
  int_var = 44100;
  write_size += write(fd, &int_var, 4);
  int_var *= 2;
  int_var *= 2;
  write_size += write(fd, &int_var, 4);
  short_var = 4;
  write_size += write(fd, &short_var, 2);
  short_var = 16;
  write_size += write(fd, &short_var, 2);
  write_size += write(fd, "data", 4);
  write_size += write(fd, &size, 4);
  if (write_size != 44)
    return 1;
  else
    return 0;
}

static int cd_stream(int sock, int data_size) {
  long start_size = 0;
  char *rsp = malloc(getpagesize());
  ssize_t write_size;
  data_list *data_cur;
  while (in_work && filled_buf_check((data_list *)data_first))
    usleep(10);
  sprintf(rsp, "%s\r\n%s%d\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
          "Content-Length: ", data_size + 44, "Content-Type: audio/wav");
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
  if (write_header(sock, data_size))
    return 1;
  data_cur = (data_list *)data_first;
  while (data_cur) {
    while (in_work && filled_buf_check(data_cur))
      usleep(10);
    write_size = write(sock, (char *)data_cur->buf, CD_FRAMESIZE_RAW);
    if (write_size != CD_FRAMESIZE_RAW)
      return 1;
    data_cur = (data_list *)data_cur->next;
  }
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  pid_t pid = getpid();
  thrd_t thr;
  cdrom_drive *d = cdda_identify("/dev/sr0", CDDA_MESSAGE_FORGETIT, NULL);
  int fd = open(play_pid_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0)
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(fd, &pid, sizeof(pid_t));
  close(fd);
  if (!d || cdda_open(d))
    execl(resp_err, "resp_err", prm[1], NULL);
  *strchr(strchr(prm[2] + 1, '/'), '.') = '\0';
  first_track = strtol(strchr(prm[2] + 1, '/') + 1, NULL, 10);
  for (int i = first_track; i <= d->tracks; i++)
    if (cdda_track_audiop(d, i))
      write_size = cdda_track_lastsector(d, i);
  write_size -= cdda_track_firstsector(d, first_track) - 1;
  write_size *= CD_FRAMESIZE_RAW;
  if (thrd_create(&thr, cd_reader, d) != thrd_success ||
      thrd_detach(thr) != thrd_success)
    return 1;
  return cd_stream(sock, write_size);
}
