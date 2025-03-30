#include "lib_read_cd.h"
#include <cdda_interface.h>
#include <fcntl.h>
#include <stdio.h>
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

static int cd_player(snd_pcm_t *pcm_p) {
  data_list *data_cur;
  snd_pcm_sframes_t avail_frames;
  char channel;
  char channels = 2;
  char *buf_tmp[channels];
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset;
  unsigned int blocksize;
  while (in_work && filled_buf_check((data_list *)data_first))
    usleep(10);
  data_cur = (data_list *)data_first;
  while (data_cur) {
    while ((avail_frames = snd_pcm_avail_update(pcm_p)) < CD_FRAMESIZE_RAW / 4)
      if (avail_frames < 0)
        return 1;
      else
        usleep(50);
    for (blocksize = 0; blocksize < CD_FRAMESIZE_RAW / 2; blocksize += 2) {
      snd_pcm_uframes_t frames = 1;
      snd_pcm_sframes_t commitres;
      if (snd_pcm_mmap_begin(pcm_p, &areas, &offset, &frames) < 0)
        return 1;
      for (channel = 0; channel < channels; channel++) {
        buf_tmp[channel] = areas[channel].addr + (areas[channel].first / 8) +
                           (offset * areas[channel].step / 8);
        memcpy(buf_tmp[channel], (short *)data_cur->buf + blocksize + channel,
               2);
      }
      commitres = snd_pcm_mmap_commit(pcm_p, offset, frames);
      if (commitres < 0 || commitres != frames)
        return 1;
    }
    if (snd_pcm_state(pcm_p) == SND_PCM_STATE_PREPARED && snd_pcm_start(pcm_p))
      return 1;
    data_cur = (data_list *)data_cur->next;
  }
  return 0;
}

static int init_alsa(snd_pcm_t **pcm_p) {
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
  if (snd_pcm_hw_params_test_rate(*pcm_p, pcm_hw, 44100, 0))
    return 1;
  snd_pcm_hw_params_set_rate(*pcm_p, pcm_hw, 44100, 0);
  if (snd_pcm_hw_params_test_format(*pcm_p, pcm_hw, SND_PCM_FORMAT_S16))
    return 1;
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
  char *cmd = malloc(getpagesize());
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
  url_track = strchr(prm[2], '?');
  if (!url_track)
    first_track = 1;
  else {
    url_track++;
    first_track = strtol(url_track, NULL, 10);
  }
  if (!d || cdda_open(d) || first_track > d->tracks)
    execl(resp_err, "resp_err", prm[1], NULL);
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  sprintf(cmd, "echo /stream_cd/%d.wav|nc -w 1 %s 9696 1>/dev/null 2>/dev/null",
          first_track, prm[3]);
  if (!system(cmd)) {
    write_size = write(sock, rsp, strlen(rsp));
    if (write_size != strlen(rsp))
      return 1;
    else
      return 0;
  }
  if (init_alsa(&pcm_p))
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
  close(sock);
  data_first = malloc(sizeof(data_list));
  data_first->next = NULL;
  data_first->buf = malloc(CD_FRAMESIZE_RAW);
  if (thrd_create(&thr, cd_reader, d) != thrd_success ||
      thrd_detach(thr) != thrd_success)
    return 1;
  return cd_player(pcm_p);
}