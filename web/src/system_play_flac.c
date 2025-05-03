#include "lib_flac_tracks.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

// clang-format off
#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>
#include <alsa/pcm.h>

#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>
// clang-format on

typedef struct card_list_t {
  struct card_list_t *next;
  snd_pcm_t *pcm;
  uint32_t off;
} card_list;

static int bytes_per_sample;

FLAC__StreamDecoderWriteStatus
write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
               const FLAC__int32 *const buffer[], void *client_data) {
  card_list *cards_first = (card_list *)client_data;
  card_list *cards_tmp;
  snd_pcm_sframes_t avail_frames;
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset;
  snd_pcm_uframes_t cpy_count;
  uint32_t blocksize = frame->header.blocksize;
  char channel;
  char channels = 2;
  char *buf_tmp[channels];
  cards_tmp = cards_first;
  while (cards_tmp) {
    while ((avail_frames = snd_pcm_avail_update(cards_tmp->pcm)) < blocksize)
      if (avail_frames < 0)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      else
        usleep(50);
    cards_tmp = cards_tmp->next;
  }
  while (blocksize > 0) {
    snd_pcm_uframes_t frames = 1;
    snd_pcm_sframes_t commitres;
    cards_tmp = cards_first;
    while (cards_tmp) {
      if (snd_pcm_mmap_begin(cards_tmp->pcm, &areas, &offset, &frames) < 0)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      for (channel = 0; channel < channels; channel++) {
        buf_tmp[channel] = areas[channel].addr + (areas[channel].first / 8) +
                           (offset * areas[channel].step / 8) + cards_tmp->off;
        memcpy(buf_tmp[channel],
               buffer[channel] + frame->header.blocksize - blocksize,
               bytes_per_sample);
      }
      commitres = snd_pcm_mmap_commit(cards_tmp->pcm, offset, frames);
      if (commitres < 0 || commitres != frames)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      cards_tmp = cards_tmp->next;
    }
    blocksize -= commitres;
  }
  cards_tmp = cards_first;
  while (cards_tmp) {
    if (snd_pcm_state(cards_tmp->pcm) == SND_PCM_STATE_PREPARED &&
        snd_pcm_start(cards_tmp->pcm))
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    cards_tmp = cards_tmp->next;
  }
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder,
                       const FLAC__StreamMetadata *metadata,
                       void *client_data) {}

void error_callback(const FLAC__StreamDecoder *decoder,
                    FLAC__StreamDecoderErrorStatus status, void *client_data) {}

static int play_album(track_list *tracks, card_list *cards) {
  FLAC__StreamDecoder *decoder = NULL;
  FLAC__StreamDecoderInitStatus init_status;
  decoder = FLAC__stream_decoder_new();
  FLAC__stream_decoder_set_md5_checking(decoder, false);
  FLAC__stream_decoder_set_metadata_ignore_all(decoder);
  while (tracks) {
    init_status = FLAC__stream_decoder_init_file(
        decoder, tracks->file_name, write_callback, metadata_callback,
        error_callback, cards);
    if (init_status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
      if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)) {
        return 1;
      }
      FLAC__stream_decoder_finish(decoder);
    } else {
      return 1;
    }
    tracks = tracks->next;
  }
  return 0;
}

static card_list *init_alsa(track_list *tracks) {
  int card_number = -1;
  char card_name[10];
  card_list *card_first = NULL;
  card_list *card_tmp = NULL;
  FLAC__StreamMetadata *rate =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
  if (!FLAC__metadata_get_streaminfo(tracks->file_name, rate))
    return NULL;
  bytes_per_sample = rate->data.stream_info.bits_per_sample / 8;
  while (!snd_card_next(&card_number) && card_number != -1) {
    uint32_t off = 0;
    snd_pcm_t *pcm_p = NULL;
    snd_pcm_hw_params_t *pcm_hw = NULL;
    snd_pcm_hw_params_malloc(&pcm_hw);
    sprintf(card_name, "hw:%d,0", card_number);
    if (snd_pcm_open(&pcm_p, card_name, SND_PCM_STREAM_PLAYBACK, 0))
      goto next;
    snd_pcm_hw_params_any(pcm_p, pcm_hw);
    if (snd_pcm_hw_params_test_rate(pcm_p, pcm_hw,
                                    rate->data.stream_info.sample_rate, 0))
      goto next;
    snd_pcm_hw_params_set_rate(pcm_p, pcm_hw,
                               rate->data.stream_info.sample_rate, 0);
    if (rate->data.stream_info.bits_per_sample == 16) {
      if (snd_pcm_hw_params_test_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S16))
        goto next;
      snd_pcm_hw_params_set_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S16);
    } else if (rate->data.stream_info.bits_per_sample == 24) {
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

static void create_cmd(char *cmd, track_list *tracks, char *client_address) {
  size_t cmd_end;
  strcat(cmd, "echo \"");
  while (tracks) {
    strcat(cmd, "$(pwd)/");
    strcat(cmd, tracks->file_name);
    if (tracks->next)
      strcat(cmd, "|");
    tracks = tracks->next;
  }
  cmd_end = strlen(cmd);
  sprintf(cmd + cmd_end, "\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
          client_address, android_client_port);
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char *rsp = malloc(getpagesize());
  char *cmd = malloc(getpagesize());
  char *url = malloc(getpagesize());
  struct hostent *host;
  char streamer_address[INET_ADDRSTRLEN];
  card_list *cards;
  track_list *tracks;
  strcpy(url, prm[2]);
  tracks = get_tracks_in_dir(url);
  if (!tracks)
    execl(resp_err, "resp_err", prm[1], NULL);
  create_cmd(cmd, tracks, prm[3]);
  if (!system(cmd)) {
    if (utime(".", NULL))
      execl(resp_err, "resp_err", prm[1], NULL);
    goto ok;
  }
  if ((host = gethostbyname(streamer_host))) {
    inet_ntop(AF_INET, (struct in_addr *)host->h_addr, streamer_address,
              INET_ADDRSTRLEN);
    sprintf(cmd,
            "echo \"/stream_album%s\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
            strchr(prm[2], '?'), streamer_address, streamer_port);
    if (!system(cmd))
      goto ok;
  }
  execl(resp_err, "resp_err", prm[1], NULL);

  cards = init_alsa(tracks);
  if (!cards)
    execl(resp_err, "resp_err", prm[1], NULL);
  if (utime(".", NULL))
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
  close(sock);
  return play_album(tracks, cards);

ok:
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
  else
    return 0;
}
