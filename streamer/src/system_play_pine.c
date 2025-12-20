#include "lib_play.h"
#include <alsa/conf.h>
#include <arpa/inet.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <signal.h>
#include <threads.h>

data_list volatile *volatile data_first;
char volatile in_work = 1;
unsigned int volatile to_del;
unsigned int volatile deleted;
unsigned char volatile clean_done;

typedef struct reader_params_t {
  int sock;
  unsigned int bytes_left;
  unsigned int rate;
  unsigned short bits_per_sample;
  unsigned int channels;
} read_params;

int data_reader(void *prm) {
  AVCodec const *decode_codec = NULL;
  AVCodecContext *decode_context = NULL;
  SwrContext *swr = NULL;
  read_params *params = prm;
  ssize_t read_size;
  data_list volatile *data_new = NULL;
  data_list volatile *data_free = NULL;
  unsigned int i;
  if (params->bits_per_sample == 24) {
    decode_codec = avcodec_find_decoder_by_name("pcm_s24le");
    decode_context = avcodec_alloc_context3(decode_codec);
    av_channel_layout_default(&decode_context->ch_layout, params->channels);
    decode_context->sample_rate = params->rate;
    avcodec_open2(decode_context, decode_codec, NULL);
    swr_alloc_set_opts2(&swr, &decode_context->ch_layout, AV_SAMPLE_FMT_S16,
                        params->rate == 44100 ? params->rate : 48000,
                        &decode_context->ch_layout, AV_SAMPLE_FMT_S32,
                        params->rate, 0, NULL);
    swr_init(swr);
  }
  while (params->bytes_left) {
    if (data_first && buf_len(data_first) > 1000) {
      if (to_del > 100 && !clean_done) {
        for (i = to_del, deleted = 0; i; deleted++, i--) {
          data_free = data_first;
          data_first = data_first->next;
          free((char *)data_free->buf);
          free((data_list *)data_free);
        }
        clean_done = 1;
      } else
        usleep(65000);
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
    if (params->bits_per_sample == 24) {
      AVPacket *pkt = NULL;
      AVFrame *ff_frame = NULL;
      uint8_t buf[data_buf_size];
      int bytes_per_sample = 2;
      int data_size = 0;
      while (data_size != data_buf_size && params->bytes_left != 0) {
        read_size =
            read(params->sock, buf + data_size, data_buf_size - data_size);
        if (read_size < 0)
          kill(getpid(), SIGTERM);
        data_size += read_size;
        params->bytes_left -= read_size;
      }
      pkt = av_packet_alloc();
      pkt->data = buf;
      pkt->size = data_size;
      ff_frame = av_frame_alloc();
      avcodec_send_packet(decode_context, pkt);
      avcodec_receive_frame(decode_context, ff_frame);
      data_new->data_size =
          swr_convert(swr, (uint8_t **)&data_new->buf,
                      swr_get_out_samples(swr, ff_frame->nb_samples),
                      (const uint8_t *const *)ff_frame->data,
                      ff_frame->nb_samples) *
          params->channels * bytes_per_sample;
      av_packet_free(&pkt);
      av_frame_free(&ff_frame);
    } else {
      while (data_new->data_size != data_buf_size && params->bytes_left != 0) {
        read_size =
            read(params->sock, (char *)data_new->buf + data_new->data_size,
                 data_buf_size - data_new->data_size);
        if (read_size < 0)
          kill(getpid(), SIGTERM);
        data_new->data_size += read_size;
        params->bytes_left -= read_size;
      }
    }
  }
  in_work = 0;
  return 0;
}

snd_pcm_t *init_alsa(unsigned int rate, unsigned int channels) {
  int card_number = -1;
  char card_name[10];
  if (!snd_card_next(&card_number) && card_number != -1) {
    snd_pcm_t *pcm_p = NULL;
    snd_pcm_hw_params_t *pcm_hw = NULL;
    snd_pcm_hw_params_malloc(&pcm_hw);
    sprintf(card_name, "hw:%d,0", card_number);
    if (snd_pcm_open(&pcm_p, card_name, SND_PCM_STREAM_PLAYBACK, 0))
      goto err;
    snd_pcm_hw_params_any(pcm_p, pcm_hw);
    snd_pcm_hw_params_set_rate(pcm_p, pcm_hw, rate == 44100 ? rate : 48000, 0);
    snd_pcm_hw_params_set_channels(pcm_p, pcm_hw, channels);
    snd_pcm_hw_params_set_buffer_size(pcm_p, pcm_hw, alsa_buf_size);
    snd_pcm_hw_params_test_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S16);
    snd_pcm_hw_params_set_access(pcm_p, pcm_hw,
                                 SND_PCM_ACCESS_MMAP_INTERLEAVED);
    if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p))
      goto err;
    snd_pcm_hw_params_free(pcm_hw);
    return pcm_p;
  }
err:
  return NULL;
}

int play(snd_pcm_t *card, unsigned int channels) {
  int bytes_per_sample = 2;
  data_list volatile *data_cur;
  snd_pcm_sframes_t avail_frames;
  int cursor;
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset;
  snd_pcm_uframes_t frames = 1;
  unsigned char channel;
  char *buf_tmp;
  snd_pcm_sframes_t commitres = 0;
  while (in_work && buf_len(data_first) < 300)
    usleep(100000);
  data_cur = data_first;
  while (data_cur) {
    while ((avail_frames = snd_pcm_avail(card)) <
           data_cur->data_size / (channels * bytes_per_sample))
      if (avail_frames < 0)
        return 1;
      else
        usleep(65000);
    cursor = 0;
    while (cursor < data_cur->data_size) {
      if (snd_pcm_avail_update(card) < 0 ||
          snd_pcm_mmap_begin(card, &areas, &offset, &frames) < 0)
        return 1;
      for (channel = 0; channel < channels; channel++) {
        buf_tmp = areas[channel].addr + (areas[channel].first / 8) +
                  (offset * areas[channel].step / 8);
        memcpy(buf_tmp,
               (char *)data_cur->buf + (channel * bytes_per_sample) + cursor,
               bytes_per_sample);
      }
      commitres = snd_pcm_mmap_commit(card, offset, frames);
      if (commitres < 0 || commitres != frames)
        return 1;
      cursor += commitres * channels * bytes_per_sample;
    }
    if (snd_pcm_state(card) == SND_PCM_STATE_PREPARED && snd_pcm_start(card))
      return 1;
    data_cur = data_cur->next;
    to_del++;
    if (clean_done) {
      to_del -= deleted;
      clean_done = 0;
    }
  }
  return 0;
}

int main(int prm_n, char *prm[]) {
  read_params params;
  thrd_t thr;
  snd_pcm_t *card;
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
  if (read_headers(params.sock, &params.rate, &params.bits_per_sample,
                   &params.bytes_left))
    return 1;
  params.channels = 2;
  card = init_alsa(params.rate, params.channels);
  if (!card)
    return 1;
  if (thrd_create(&thr, data_reader, &params) != thrd_success ||
      thrd_detach(thr) != thrd_success)
    return 1;
  return play(card, params.channels);
}
