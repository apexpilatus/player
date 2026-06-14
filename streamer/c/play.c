#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>
#include <alsa/conf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct card_list_t {
  struct card_list_t *next;
  snd_pcm_t *pcm;
  uint32_t off;
} card_list;

typedef struct {
  uint32_t not_started;
  uint32_t rate;
  uint32_t bytes_per_sample;
  FLAC__uint64 total_samples;
  card_list *cards;
} extract_params;

// int read_hdr() {
//   ssize_t msg_size = getpagesize();
//   ssize_t read_size = 0;
//   char hdr[msg_size];
//   while (read_size < msg_size && fread(hdr + read_size, 1, 1, stdin) == 1) {
//     read_size++;
//     if (read_size < msg_size)
//       hdr[read_size] = '\0';
//     if (read_size > 3 && !strcmp(hdr + read_size - 4, "\r\n\r\n"))
//       break;
//   }
//   printf("\nhere\n");
//   if (read_size == msg_size || read_size < 9 || strstr(hdr, "404 shit
//   happens"))
//     return 1;
//   printf("hdr size - %ld\n%s", read_size, hdr);
//   return 0;
// }

card_list *init_alsa(uint32_t rate, uint32_t bits_per_sample,
                     uint32_t channels) {
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

void error_callback(const FLAC__StreamDecoder *decoder,
                    FLAC__StreamDecoderErrorStatus status, void *client_data) {
  printf("flac error\n");
  exit(1);
}

void metadata_callback(const FLAC__StreamDecoder *decoder,
                       const FLAC__StreamMetadata *metadata,
                       void *client_data) {
  extract_params *params = client_data;
  params->total_samples = metadata->data.stream_info.total_samples;
  printf("%lu\n", params->total_samples);

  params->rate = metadata->data.stream_info.sample_rate;
  params->bytes_per_sample = metadata->data.stream_info.bits_per_sample / 8;
  params->cards = init_alsa(metadata->data.stream_info.sample_rate,
                            metadata->data.stream_info.bits_per_sample,
                            metadata->data.stream_info.channels);
  if (!params->cards) {
    printf("cannot init alsa\n");
    exit(1);
  }
  params->not_started = 1;
}

// int play(card_list *cards_first, size_t bytes_per_sample) {
//   data_list volatile *data_cur;
//   card_list *cards_tmp;
//   snd_pcm_sframes_t avail_frames;
//   int cursor;
//   const snd_pcm_channel_area_t *areas;
//   snd_pcm_uframes_t offset;
//   snd_pcm_uframes_t frames = 1;
//   unsigned char channel;
//   char *buf_tmp;
//   snd_pcm_sframes_t commitres = 0;
//   while (in_work && buf_len(data_first) < 200)
//     usleep(sleep_timeout);
//   data_cur = data_first;
//   while (data_cur) {
//     cards_tmp = cards_first;
//     while (cards_tmp) {
//       while ((avail_frames = snd_pcm_avail(cards_tmp->pcm)) <
//              data_cur->data_size / (channels * bytes_per_sample))
//         if (avail_frames < 0)
//           return 1;
//         else
//           usleep(sleep_timeout);
//       cards_tmp = cards_tmp->next;
//     }
//     cursor = 0;
//     while (cursor < data_cur->data_size) {
//       cards_tmp = cards_first;
//       while (cards_tmp) {
//         if (snd_pcm_avail_update(cards_tmp->pcm) < 0 ||
//             snd_pcm_mmap_begin(cards_tmp->pcm, &areas, &offset, &frames) < 0)
//           return 1;
//         for (channel = 0; channel < channels; channel++) {
//           buf_tmp = areas[channel].addr + (areas[channel].first / 8) +
//                     (offset * areas[channel].step / 8) + cards_tmp->off;
//           memcpy(buf_tmp,
//                  (char *)data_cur->buf + (channel * bytes_per_sample) +
//                  cursor, bytes_per_sample);
//         }
//         commitres = snd_pcm_mmap_commit(cards_tmp->pcm, offset, frames);
//         if (commitres < 0 || commitres != frames)
//           return 1;
//         cards_tmp = cards_tmp->next;
//       }
//       cursor += commitres * channels * bytes_per_sample;
//     }
//     cards_tmp = cards_first;
//     while (cards_tmp) {
//       if (snd_pcm_state(cards_tmp->pcm) == SND_PCM_STATE_PREPARED &&
//           snd_pcm_start(cards_tmp->pcm))
//         return 1;
//       cards_tmp = cards_tmp->next;
//     }
//     data_cur = data_cur->next;
//     to_del++;
//     if (clean_done) {
//       to_del -= deleted;
//       clean_done = 0;
//     }
//   }
//   while (snd_pcm_state(cards_first->pcm) == SND_PCM_STATE_RUNNING)
//     usleep(sleep_timeout);
//   return 0;
// }

// write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
//                const FLAC__int32 *const buffer[], void *client_data) {
//   extract_params *params = client_data;
//   if (frame->header.blocksize * 2 * params->bytes_per_sample <=
//       params->bytes_skip) {
//     params->bytes_skip -=
//         frame->header.blocksize * 2 * params->bytes_per_sample;
//     return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
//   }
//   int i;
//   for (i = 0; i < frame->header.blocksize; i++) {
//     int j;
//     for (j = 0; j < params->bytes_per_sample; j++)
//       if (!params->bytes_skip) {
//         if (fwrite((char *)(buffer[0] + i) + j, 1, 1, stdout) != 1)
//           return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//         params->bytes_left--;
//         if (params->bytes_left == 0)
//           return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//       } else
//         params->bytes_skip--;
//     for (j = 0; j < params->bytes_per_sample; j++)
//       if (!params->bytes_skip) {
//         if (fwrite((char *)(buffer[1] + i) + j, 1, 1, stdout) != 1)
//           return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//         params->bytes_left--;
//         if (params->bytes_left == 0)
//           return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//       } else
//         params->bytes_skip--;
//   }
//   return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
// }

FLAC__StreamDecoderWriteStatus
write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
               const FLAC__int32 *const buffer[], void *client_data) {
  extract_params *params = client_data;
  int i;
  snd_pcm_sframes_t avail_frames;
  card_list *cards_tmp;
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset;
  snd_pcm_uframes_t frames = 1;
  unsigned char channel;
  char *buf_tmp;
  snd_pcm_sframes_t commitres = 0;

  params->total_samples -= frame->header.blocksize;
  printf("%ld - ", params->total_samples);

  for (i = 0; i < frame->header.blocksize; i++) {
    while ((avail_frames = snd_pcm_avail(params->cards->pcm)) < frames)
      if (avail_frames < 0)
        return 1;
      else
        usleep(1000);
    cards_tmp = params->cards;
    while (cards_tmp) {
      if (snd_pcm_avail_update(cards_tmp->pcm) < 0 ||
          snd_pcm_mmap_begin(cards_tmp->pcm, &areas, &offset, &frames) < 0)
        return 1;
      for (channel = 0; channel < frame->header.channels; channel++) {
        buf_tmp = areas[channel].addr + (areas[channel].first / 8) +
                  (offset * areas[channel].step / 8) + cards_tmp->off;
        memcpy(buf_tmp, (char *)(buffer[channel] + i),
               params->bytes_per_sample);
      }
      commitres = snd_pcm_mmap_commit(cards_tmp->pcm, offset, frames);
      if (commitres < 0 || commitres != frames)
        return 1;
      cards_tmp = cards_tmp->next;
    }
    if (params->not_started && i > 100) {
      cards_tmp = params->cards;
      while (cards_tmp) {
        if (snd_pcm_state(cards_tmp->pcm) == SND_PCM_STATE_PREPARED &&
            snd_pcm_start(cards_tmp->pcm))
          return 1;
        cards_tmp = cards_tmp->next;
      }
      params->not_started = 0;
    }
  }
  printf("ok;");
  if (params->total_samples < 3000) {
    printf("end of stream");
    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
  } else
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

int main(void) {
  extract_params params;
  FLAC__StreamDecoder *decoder = NULL;
  decoder = FLAC__stream_decoder_new();
  if (FLAC__stream_decoder_init_FILE(
          decoder, stdin, write_callback, metadata_callback, error_callback,
          &params) == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    if (!FLAC__stream_decoder_process_until_end_of_stream(decoder))
      return 1;
    FLAC__stream_decoder_finish(decoder);
  } else
    return 1;
  printf("c_end\n");
  return 0;
}
