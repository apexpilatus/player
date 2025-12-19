#include "lib_play.h"
#include <alsa/conf.h>
#include <arpa/inet.h>
#include <signal.h>
#include <threads.h>

typedef struct card_list_t {
  struct card_list_t *next;
  snd_pcm_t *pcm;
  uint32_t off;
} card_list;

typedef struct reader_params_t {
  int sock;
  unsigned int bytes_left;
} read_params;

data_list volatile *volatile data_first;
char volatile in_work = 1;
unsigned int channels = 2;

int data_reader(void *prm) {
  read_params *params = prm;
  ssize_t read_size;
  data_list volatile *data_new = NULL;
  while (params->bytes_left) {
    if (!data_new) {
      data_first = malloc(sizeof(data_list));
      data_new = data_first;
    } else {
      data_new->next = malloc(sizeof(data_list));
      data_new = data_new->next;
    }
    data_new->next = NULL;
    data_new->buf = malloc(data_buf_size);
    data_new->data_size = 0;
    while (data_new->data_size != data_buf_size && params->bytes_left != 0) {
      read_size =
          read(params->sock, (char *)data_new->buf + data_new->data_size,
               data_buf_size - data_new->data_size);
      if (read_size < 0)
        kill(getpid(), SIGTERM);
      data_new->data_size += read_size;
      params->bytes_left -= read_size;
    }
    if (buf_len(data_first->next) > 10000) {
      usleep(100000);
      continue;
    }
  }
  in_work = 0;
  return 0;
}

card_list *init_alsa(unsigned int rate, unsigned short bits_per_sample) {
  int card_number = -1;
  char card_name[10];
  card_list *card_first = NULL;
  card_list *card_tmp = NULL;
  while (!snd_card_next(&card_number) && card_number != -1) {
    uint32_t off = 0;
    snd_pcm_t *pcm_p = NULL;
    snd_pcm_hw_params_t *pcm_hw = NULL;
    snd_pcm_hw_params_malloc(&pcm_hw);
    sprintf(card_name, "hw:%d,0", card_number);
    if (snd_pcm_open(&pcm_p, card_name, SND_PCM_STREAM_PLAYBACK, 0))
      goto next;
    snd_pcm_hw_params_any(pcm_p, pcm_hw);
    if (snd_pcm_hw_params_test_rate(pcm_p, pcm_hw, rate, 0))
      goto next;
    snd_pcm_hw_params_set_rate(pcm_p, pcm_hw, rate, 0);
    if (snd_pcm_hw_params_test_channels(pcm_p, pcm_hw, channels))
      goto next;
    snd_pcm_hw_params_set_channels(pcm_p, pcm_hw, channels);
    if (snd_pcm_hw_params_test_buffer_size(pcm_p, pcm_hw, alsa_buf_size))
      goto next;
    snd_pcm_hw_params_set_buffer_size(pcm_p, pcm_hw, alsa_buf_size);
    if (bits_per_sample == 16) {
      if (snd_pcm_hw_params_test_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S16))
        goto next;
      snd_pcm_hw_params_set_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S16);
    } else if (bits_per_sample == 24) {
      if (!snd_pcm_hw_params_test_format(pcm_p, pcm_hw,
                                         SND_PCM_FORMAT_S24_3LE)) {
        snd_pcm_hw_params_set_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S24_3LE);
      } else if (!snd_pcm_hw_params_test_format(pcm_p, pcm_hw,
                                                SND_PCM_FORMAT_S32)) {
        snd_pcm_hw_params_set_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S32);
        off = 1;
      } else
        goto next;
    } else
      return NULL;
    snd_pcm_hw_params_set_access(pcm_p, pcm_hw,
                                 SND_PCM_ACCESS_MMAP_INTERLEAVED);
    if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p))
      goto next;
    if (!card_first) {
      card_tmp = malloc(sizeof(card_list));
      card_first = card_tmp;
    } else {
      card_tmp->next = malloc(sizeof(card_list));
      card_tmp = card_tmp->next;
    }
    memset(card_tmp, 0, sizeof(card_list));
    card_tmp->pcm = pcm_p;
    card_tmp->off = off;
  next:
    snd_pcm_hw_params_free(pcm_hw);
  }
  return card_first;
}

int play(card_list *cards_first, size_t bytes_per_sample) {
  data_list volatile *data_cur;
  data_list volatile *data_free;
  card_list *cards_tmp;
  snd_pcm_sframes_t avail_frames;
  int cursor;
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset;
  snd_pcm_uframes_t frames = 1;
  unsigned char channel;
  char *buf_tmp;
  snd_pcm_sframes_t commitres = 0;
  while (in_work && buf_len(data_first) < 200)
    usleep(100000);
  data_cur = data_first;
  while (data_cur) {
    cards_tmp = cards_first;
    while (cards_tmp) {
      while ((avail_frames = snd_pcm_avail(cards_tmp->pcm)) <
             data_cur->data_size / (channels * bytes_per_sample))
        if (avail_frames < 0)
          return 1;
        else
          usleep(100000);
      cards_tmp = cards_tmp->next;
    }
    cursor = 0;
    while (cursor < data_cur->data_size) {
      cards_tmp = cards_first;
      while (cards_tmp) {
        if (snd_pcm_avail_update(cards_tmp->pcm) < 0 ||
            snd_pcm_mmap_begin(cards_tmp->pcm, &areas, &offset, &frames) < 0)
          return 1;
        for (channel = 0; channel < channels; channel++) {
          buf_tmp = areas[channel].addr + (areas[channel].first / 8) +
                    (offset * areas[channel].step / 8) + cards_tmp->off;
          memcpy(buf_tmp,
                 (char *)data_cur->buf + (channel * bytes_per_sample) + cursor,
                 bytes_per_sample);
        }
        commitres = snd_pcm_mmap_commit(cards_tmp->pcm, offset, frames);
        if (commitres < 0 || commitres != frames)
          return 1;
        cards_tmp = cards_tmp->next;
      }
      cursor += commitres * channels * bytes_per_sample;
    }
    cards_tmp = cards_first;
    while (cards_tmp) {
      if (snd_pcm_state(cards_tmp->pcm) == SND_PCM_STATE_PREPARED &&
          snd_pcm_start(cards_tmp->pcm))
        return 1;
      cards_tmp = cards_tmp->next;
    }
    data_cur = data_cur->next;
    data_free = data_first;
    data_first = data_first->next;
    free((char *)data_free->buf);
    free((data_list *)data_free);
  }
  return 0;
}

int main(int prm_n, char *prm[]) {
  unsigned int rate;
  unsigned short bits_per_sample;
  read_params params;
  thrd_t thr;
  card_list *cards;
  ssize_t write_size;
  pid_t pid = getpid();
  int fd = open(play_pid_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0)
    return 1;
  write_size = write(fd, &pid, sizeof(pid_t));
  close(fd);
  if (write_size != sizeof(pid_t)) {
    unlink(play_pid_path);
    return 1;
  }
  params.sock = strtol(prm[1], NULL, 10);
  close(params.sock);
  params.sock = socket(PF_INET6, SOCK_STREAM, 0);
  if (send_request(params.sock, prm))
    return 1;
  if (read_headers(params.sock, &rate, &bits_per_sample, &params.bytes_left))
    return 1;
  cards = init_alsa(rate, bits_per_sample);
  if (!cards)
    return 1;
  if (thrd_create(&thr, data_reader, &params) != thrd_success ||
      thrd_detach(thr) != thrd_success)
    return 1;
  return play(cards, bits_per_sample / 8);
}
