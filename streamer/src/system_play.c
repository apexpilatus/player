#include <alsa/conf.h>
#include <arpa/inet.h>
#include <signal.h>
#include <threads.h>

#define store_port 80

typedef struct card_list_t {
  struct card_list_t *next;
  snd_pcm_t *pcm;
  long buf_size;
  uint32_t off;
} card_list;

typedef struct data_list_t {
  struct data_list_t volatile *next;
  char volatile *buf;
  int data_size;
} data_list;

data_list volatile *volatile data_first;
unsigned char volatile pause_download;
const unsigned int alsa_buf_size = 12000;
const unsigned int data_buf_size = alsa_buf_size / 2;
unsigned int volatile bytes_left;
char volatile in_work = 1;

int data_reader(void *prm) {
  int sock = *((int *)prm);
  ssize_t read_size;
  data_list volatile *data_new = NULL;
  while (bytes_left) {
    if (pause_download) {
      usleep(5000);
      continue;
    }
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
    while (data_new->data_size != data_buf_size && bytes_left != 0) {
      read_size = read(sock, (char *)data_new->buf + data_new->data_size,
                       data_buf_size - data_new->data_size);
      if (read_size < 0)
        kill(getpid(), SIGTERM);
      data_new->data_size += read_size;
      bytes_left -= read_size;
    }
  }
  in_work = 0;
  return 0;
}

int filled_buf_check(data_list volatile *data, int limit) {
  int count = 0;
  while (data) {
    count++;
    if (count > limit)
      return 0;
    data = data->next;
  }
  return 1;
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
    snd_pcm_hw_params_get_buffer_size(pcm_hw,
                                      (snd_pcm_uframes_t *)&card_tmp->buf_size);
  next:
    snd_pcm_hw_params_free(pcm_hw);
  }
  return card_first;
}

int play(int sock, card_list *cards_first, size_t bytes_per_sample) {
  data_list volatile *data_cur;
  data_list volatile *data_free;
  unsigned char written = 0;
  card_list *cards_tmp;
  snd_pcm_sframes_t avail_frames;
  int cursor;
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset;
  snd_pcm_uframes_t frames = 1;
  unsigned char channel;
  unsigned char channels = 2;
  char *buf_tmp[channels];
  snd_pcm_sframes_t commitres = 0;
  while (in_work && filled_buf_check(data_first, 200))
    usleep(1000);
  data_cur = data_first;
  data_free = data_first;
  while (data_cur) {
    cards_tmp = cards_first;
    while (cards_tmp) {
      while ((avail_frames = snd_pcm_avail(cards_tmp->pcm)) <
             data_cur->data_size)
        if (avail_frames < 0)
          return 1;
        else
          usleep(5000);
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
          buf_tmp[channel] = areas[channel].addr + (areas[channel].first / 8) +
                             (offset * areas[channel].step / 8) +
                             cards_tmp->off;
          memcpy(buf_tmp[channel],
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
    if (written < 200)
      written++;
    else {
      data_free = data_first;
      data_first = data_first->next;
      free((char *)data_free->buf);
      free((data_list *)data_free);
      pause_download = !filled_buf_check(data_first, 1000);
    }
  }
  return 0;
}

int read_headers(int sock, unsigned int *rate,
                 unsigned short *bits_per_sample) {
  int read_size = 0;
  int msg_size = getpagesize() * 100;
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
    bytes_left = *((unsigned int *)(msg + 4));
  } else {
    for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
      read_size++;
      if (read_size == 32)
        break;
    }
    if (read_size < 32)
      return 1;
    bytes_left = *((unsigned int *)(msg + 28));
  }
  return 0;
}

int send_request(int sock, char *prm[]) {
  char msg[getpagesize()];
  struct sockaddr_in6 addr;
  addr.sin6_family = AF_INET6;
  if (!inet_pton(AF_INET6, prm[3], &addr.sin6_addr))
    return 1;
  addr.sin6_flowinfo = 0;
  addr.sin6_scope_id = strtol(prm[4], NULL, 10);
  addr.sin6_port = htons(store_port);
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
  close(sock);
  sock = socket(PF_INET6, SOCK_STREAM, 0);
  if (send_request(sock, prm))
    return 1;
  if (read_headers(sock, &rate, &bits_per_sample))
    return 1;
  cards = init_alsa(rate, bits_per_sample);
  if (!cards)
    return 1;
  if (thrd_create(&thr, data_reader, &sock) != thrd_success ||
      thrd_detach(thr) != thrd_success)
    return 1;
  return play(sock, cards, bits_per_sample / 8);
}
