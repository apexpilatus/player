#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// clang-format off
#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>
#include <alsa/pcm.h>
// clang-format on

#define store_port 80

typedef struct card_list_t {
  struct card_list_t *next;
  snd_pcm_t *pcm;
  long buf_size;
  uint32_t off;
} card_list;

static card_list *init_alsa(unsigned int rate, unsigned short bits_per_sample) {
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
    snd_pcm_hw_params_get_buffer_size(pcm_hw,
                                      (snd_pcm_uframes_t *)&card_tmp->buf_size);
  next:
    snd_pcm_hw_params_free(pcm_hw);
  }
  return card_first;
}

static int play(int sock, card_list *cards_first, size_t bytes_per_sample,
                int bytes_left) {
  card_list *cards_tmp;
  char channel;
  char channels = 2;
  int blocksize;
  int cursor;
  ssize_t msg_size = bytes_per_sample * channels * 1000;
  char msg[msg_size];
  char *buf_tmp[channels];
  snd_pcm_sframes_t avail_frames;
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset;
  snd_pcm_uframes_t frames = 1;
  snd_pcm_sframes_t commitres;
  while (bytes_left) {
    for (blocksize = 0; (cursor = read(sock, msg + blocksize, 1)) == 1;) {
      if (cursor < 0)
        return 1;
      blocksize++;
      bytes_left--;
      if (blocksize == msg_size || bytes_left == 0)
        break;
    }
    cards_tmp = cards_first;
    while (cards_tmp) {
      while ((avail_frames = snd_pcm_avail(cards_tmp->pcm)) < blocksize)
        if (avail_frames < 0)
          return 1;
        else
          usleep(5000);
      cards_tmp = cards_tmp->next;
    }
    cursor = 0;
    while (cursor < blocksize) {
      cards_tmp = cards_first;
      while (cards_tmp) {
        if (snd_pcm_avail_update(cards_tmp->pcm) < 0 ||
            snd_pcm_mmap_begin(cards_tmp->pcm, &areas, &offset, &frames) < 0)
          return 1;
        for (channel = 0; channel < channels; channel++) {
          buf_tmp[channel] = areas[channel].addr + (areas[channel].first / 8) +
                             (offset * areas[channel].step / 8) +
                             cards_tmp->off;
          memcpy(buf_tmp[channel], msg + (channel * bytes_per_sample) + cursor,
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
  }
  while ((avail_frames = snd_pcm_avail(cards_first->pcm)) <
         cards_first->buf_size) {
    if (avail_frames < 0)
      break;
    else
      usleep(5000);
  }
  cards_tmp = cards_first;
  while (cards_tmp) {
    snd_pcm_drop(cards_tmp->pcm);
    cards_tmp = cards_tmp->next;
  }
  return 0;
}

static int read_headers(int sock, unsigned int *rate,
                        unsigned short *bits_per_sample, int *bytes_left) {
  ssize_t read_size = 0;
  ssize_t msg_size = getpagesize() * 100;
  char msg[msg_size];
  while (read_size < msg_size && read(sock, msg + read_size, 1) == 1) {
    read_size++;
    if (read_size < msg_size)
      msg[read_size] = '\0';
    if (read_size > 3 && !strcmp(msg + read_size - 4, "\r\n\r\n")) {
      break;
    }
  }
  if (read_size == msg_size ||
      strncmp(msg, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK")))
    return 1;
  for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
    read_size++;
    if (read_size == 28)
      break;
  }
  if (read_size < 28)
    return 1;
  *rate = *((int *)(msg + 24));
  for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
    read_size++;
    if (read_size == 8)
      break;
  }
  if (read_size < 8)
    return 1;
  *bits_per_sample = *((short *)(msg + 6));
  if (*bits_per_sample == 16) {
    for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
      read_size++;
      if (read_size == 8)
        break;
    }
    if (read_size < 8)
      return 1;
    *bytes_left = *((int *)(msg + 4));
  } else {
    for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
      read_size++;
      if (read_size == 32)
        break;
    }
    if (read_size < 32)
      return 1;
    *bytes_left = *((int *)(msg + 28));
  }
  return 0;
}

static int send_request(int sock, char *prm[]) {
  char msg[getpagesize()];
  struct sockaddr_in addr;
  if (!inet_aton(prm[3], &addr.sin_addr))
    return 1;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(store_port);
  if (0 > connect(sock, (struct sockaddr *)&addr, sizeof(addr)))
    return 1;
  strcpy(msg, "GET ");
  strcat(msg, prm[2]);
  strcat(msg, " \r\n\r\n");
  if (write(sock, msg, strlen(msg)) != strlen(msg))
    return 1;
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  unsigned int rate;
  unsigned short bits_per_sample;
  int bytes_left;
  card_list *cards;
  close(sock);
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (send_request(sock, prm))
    return 1;
  if (read_headers(sock, &rate, &bits_per_sample, &bytes_left))
    return 1;
  cards = init_alsa(rate, bits_per_sample);
  if (!cards)
    return 1;
  return play(sock, cards, bits_per_sample / 8, bytes_left);
}
