#include <cdda_interface.h>
#include <cdda_paranoia.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

// clang-format off
#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>
#include <alsa/pcm.h>
// clang-format on

typedef struct data_list_t {
  struct data_list_t volatile *next;
  char *buf;
} data_list;

static data_list volatile *volatile data_first;
static data_list volatile *volatile data_cur;
static int volatile first_track;

static void callback(long inpos, int function) {}

static int cd_reader(void *prm) {
  cdrom_drive *d = prm;
  data_list volatile *data_new = NULL;
  long cursor;
  long lastsector;
  int16_t *readbuf;
  cdrom_paranoia *p = paranoia_init(d);
  paranoia_modeset(p, PARANOIA_MODE_FULL ^ PARANOIA_MODE_NEVERSKIP);
  for (int i = first_track; i <= d->tracks; i++)
    if (cdda_track_audiop(d, i)) {
      paranoia_seek(p, cursor = cdda_track_firstsector(d, i), SEEK_SET);
      lastsector = cdda_track_lastsector(d, i);
      while (cursor <= lastsector) {
        readbuf = paranoia_read_limited(p, callback, 5);
        if (readbuf == NULL) {
          if (errno == EBADF || errno == ENOMEDIUM) {
            data_first = NULL;
            return 1;
          }
        } else {
          if (!data_new) {
            data_new = data_first;
          } else {
            data_new->next = malloc(sizeof(data_list));
            data_new->next->next = NULL;
            data_new = data_new->next;
            data_new->buf = malloc(CD_FRAMESIZE_RAW);
          }
          memcpy(data_new->buf, readbuf, CD_FRAMESIZE_RAW);
        }
        cursor++;
      }
    }
  paranoia_free(p);
  cdda_close(d);
  return 0;
}

static inline int cd_player(snd_pcm_t *pcm_p) {
  long data_size;
  while (data_first && data_size <= CD_FRAMESIZE_RAW) {
    data_cur = data_first;
    data_size = 0;
    while (data_cur) {
      data_size += CD_FRAMESIZE_RAW / 4;
      data_cur = data_cur->next;
    }
  }
  if (!data_first)
    return 1;
  data_cur = data_first;
  while (data_first && data_cur) {
    snd_pcm_mmap_writei(pcm_p, data_cur->buf, CD_FRAMESIZE_RAW / 4);
    data_cur = data_cur->next;
  }
  if (!data_first)
    return 1;
  snd_pcm_drain(pcm_p);
  return 0;
}

static inline int init_alsa(snd_pcm_t **pcm_p) {
  int card = -1;
  char card_name[10];
  snd_pcm_hw_params_t *pcm_hw;
  if (snd_card_next(&card) || card == -1)
    return 1;
  sprintf(card_name, "hw:%d,0", card);
  if (snd_pcm_open(pcm_p, card_name, SND_PCM_STREAM_PLAYBACK, 0))
    return 1;
  snd_pcm_hw_params_malloc(&pcm_hw);
  snd_pcm_hw_params_any(*pcm_p, pcm_hw);
  snd_pcm_hw_params_set_rate(*pcm_p, pcm_hw, 44100, 0);
  snd_pcm_hw_params_set_format(*pcm_p, pcm_hw, SND_PCM_FORMAT_S16);
  snd_pcm_hw_params_set_access(*pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
  snd_pcm_hw_params_set_buffer_size(*pcm_p, pcm_hw,
                                    (snd_pcm_uframes_t)CD_FRAMESIZE_RAW);
  if (snd_pcm_hw_params(*pcm_p, pcm_hw) || snd_pcm_prepare(*pcm_p))
    return 1;
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char *url_track;
  char *rsp = malloc(getpagesize());
  cdrom_drive *d = cdda_identify("/dev/sr0", CDDA_MESSAGE_FORGETIT, NULL);
  snd_pcm_t *pcm_p;
  thrd_t thr;
  int fd;
  pid_t pid = getpid();
  fd = open(play_pid_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0)
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(fd, &pid, sizeof(pid_t));
  close(fd);
  if (!d || cdda_open(d) || init_alsa(&pcm_p))
    execl(resp_err, "resp_err", prm[1], NULL);
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
  close(sock);
  url_track = strchr(prm[2], '?');
  if (!url_track)
    first_track = 1;
  else {
    url_track++;
    first_track = strtol(url_track, NULL, 10);
  }
  data_first = malloc(sizeof(data_list));
  data_first->next = NULL;
  data_first->buf = malloc(CD_FRAMESIZE_RAW);
  if (thrd_create(&thr, cd_reader, d) != thrd_success)
    return 1;
  return cd_player(pcm_p);
}