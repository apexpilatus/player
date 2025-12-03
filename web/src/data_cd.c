#include <cdda_interface.h>
#include <cdda_paranoia.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

typedef struct data_list_t {
  struct data_list_t volatile *next;
  char volatile *buf;
} data_list;

int data_size;
int min_range;
int max_range;
int first_shift;
data_list volatile *volatile data_first;
long volatile first_sector;
long volatile last_sector;
char volatile in_work = 1;

void callback(long inpos, int function) {}

int cd_reader(void *prm) {
  cdrom_drive *d = prm;
  data_list volatile *data_new = NULL;
  long cursor;
  cdrom_paranoia *p = paranoia_init(d);
  paranoia_modeset(p, PARANOIA_MODE_FULL ^ PARANOIA_MODE_NEVERSKIP);
  paranoia_seek(p, cursor = first_sector, SEEK_SET);
  while (cursor <= last_sector) {
    int16_t *readbuf = paranoia_read_limited(p, callback, 5);
    if (readbuf == NULL) {
      paranoia_free(p);
      cdda_close(d);
      kill(getpid(), SIGTERM);
    } else {
      if (!data_new) {
        data_first = malloc(sizeof(data_list));
        data_first->next = NULL;
        data_first->buf = malloc(CD_FRAMESIZE_RAW);
        data_new = data_first;
      } else {
        data_new->next = malloc(sizeof(data_list));
        data_new->next->next = NULL;
        data_new = data_new->next;
        data_new->buf = malloc(CD_FRAMESIZE_RAW);
      }
      memcpy((char *)data_new->buf, readbuf, CD_FRAMESIZE_RAW);
    }
    cursor++;
  }
  paranoia_free(p);
  cdda_close(d);
  in_work = 0;
  return 0;
}

int filled_buf_check(data_list volatile *data) {
  int count = 0;
  while (data) {
    count++;
    if (count > 200)
      return 0;
    data = (data_list *)data->next;
  }
  return 1;
}

int write_header(int fd, int size) {
  int int_var;
  short short_var;
  ssize_t write_size = 0;
  int_var = size + 44 - 8;
  write_size = write(fd, "RIFF", 4);
  write_size += write(fd, &int_var, 4);
  write_size += write(fd, "WAVEfmt ", 8);
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

int cd_stream(int sock) {
  char rsp[getpagesize()];
  ssize_t write_size;
  data_list volatile *data_cur;
  int bytes_left = max_range - min_range + 1;
  sprintf(rsp, "%s\r\n%s%d\r\nContent-Range: bytes %d-%d/%d\r\n%s\r\n\r\n",
          "HTTP/1.1 200 OK", "Content-Length: ", bytes_left, min_range,
          max_range, data_size + 44, "Content-Type: audio/wav");
  if (write(sock, rsp, strlen(rsp)) != strlen(rsp))
    return 1;
  if (min_range == 0 && max_range < 43) {
    char buf[bytes_left];
    write_size = write(sock, buf, bytes_left);
    return 1;
  }
  if (min_range == 0) {
    if (write_header(sock, data_size))
      return 1;
    else
      bytes_left -= 44;
  }
  while (in_work && filled_buf_check(data_first))
    usleep(50000);
  data_cur = data_first;
  while (data_cur && bytes_left) {
    while (in_work && filled_buf_check(data_cur))
      usleep(50000);
    write_size = write(sock, (char *)data_cur->buf + first_shift,
                       bytes_left < CD_FRAMESIZE_RAW - first_shift
                           ? bytes_left
                           : CD_FRAMESIZE_RAW - first_shift);
    if (write_size != (bytes_left < CD_FRAMESIZE_RAW - first_shift
                           ? bytes_left
                           : CD_FRAMESIZE_RAW - first_shift))
      return 1;
    bytes_left -= write_size;
    first_shift = 0;
    data_cur = data_cur->next;
  }
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  pid_t pid = getpid();
  thrd_t thr;
  char *end;
  int first_track;
  cdrom_drive *d = cdda_identify("/dev/sr0", CDDA_MESSAGE_FORGETIT, NULL);
  int fd = open(play_pid_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0)
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(fd, &pid, sizeof(pid_t));
  close(fd);
  if (write_size != sizeof(pid_t)) {
    unlink(play_pid_path);
    execl(resp_err, "resp_err", prm[1], NULL);
  }
  if (!d || cdda_open(d))
    execl(resp_err, "resp_err", prm[1], NULL);
  first_track = strtol(strchr(prm[2], '?') + 1, NULL, 10);
  for (int i = first_track; i <= d->tracks; i++)
    if (cdda_track_audiop(d, i))
      last_sector = cdda_track_lastsector(d, i);
  first_sector = cdda_track_firstsector(d, first_track);
  data_size = last_sector + 1 - first_sector;
  data_size *= CD_FRAMESIZE_RAW;
  if ((end = strchr(prm[3], '-')) && strlen(++end) > 0)
    max_range = strtol(end, NULL, 10);
  else
    max_range = data_size + 43;
  if ((end = strchr(prm[3], '-'))) {
    *end = '\0';
    min_range = strtol(prm[3], NULL, 10);
  }
  if (min_range > 0 && min_range < 44)
    execl(resp_err, "resp_err", prm[1], NULL);
  if (min_range > 0) {
    first_shift = (min_range - 44) % CD_FRAMESIZE_RAW;
    first_sector += (min_range - 44) / CD_FRAMESIZE_RAW;
  }
  if (thrd_create(&thr, cd_reader, d) != thrd_success ||
      thrd_detach(thr) != thrd_success)
    execl(resp_err, "resp_err", prm[1], NULL);
  return cd_stream(sock);
}
