#include <FLAC/stream_decoder.h>
#include <alsa/conf.h>
#include <sys/wait.h>

#define play_pid_path "pid"

typedef struct card_list_t {
  struct card_list_t *next;
  snd_pcm_t *pcm;
  uint32_t off;
} card_list;

typedef struct {
  uint32_t not_started;
  uint32_t rate;
  uint32_t bytes_per_sample;
  uint32_t channels;
  FLAC__uint64 total_samples;
  long stream_length;
  card_list *cards;
} extract_params;

long get_length() {
  ssize_t msg_size = 20;
  ssize_t read_size = 0;
  char length[msg_size];
  while (read_size < msg_size && fread(length + read_size, 1, 1, stdin) == 1) {
    read_size++;
    if (read_size < msg_size)
      length[read_size] = '\0';
    if (length[read_size - 1] == '\n')
      break;
  }
  if (read_size == msg_size || read_size < 2)
    return -1;
  return strtol(length, NULL, 10);
}

void ret(int status) {
  int fd = open(play_pid_path, O_RDONLY);
  if (fd >= 0) {
    pid_t run_pid;
    ssize_t read_size = read(fd, &run_pid, sizeof(pid_t));
    close(fd);
    if (read_size == sizeof(pid_t) && run_pid == getpid())
      unlink(play_pid_path);
  }
  exit(status);
}

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

int clear_alsa(card_list *cards) {
  if (snd_pcm_drain(cards->pcm))
    return 1;
  while (cards) {
    card_list *card_to_del = cards;
    if (snd_pcm_close(cards->pcm))
      return 1;
    cards = cards->next;
    free(card_to_del);
  }
  return 0;
}

void error_callback(const FLAC__StreamDecoder *decoder,
                    FLAC__StreamDecoderErrorStatus status, void *client_data) {
  ret(1);
}

void metadata_callback(const FLAC__StreamDecoder *decoder,
                       const FLAC__StreamMetadata *metadata,
                       void *client_data) {
  extract_params *params = client_data;
  params->total_samples = metadata->data.stream_info.total_samples;
  if (metadata->data.stream_info.sample_rate != params->rate ||
      metadata->data.stream_info.channels != params->channels ||
      metadata->data.stream_info.bits_per_sample / 8 !=
          params->bytes_per_sample) {
    ssize_t write_size;
    pid_t pid;
    params->rate = metadata->data.stream_info.sample_rate;
    params->channels = metadata->data.stream_info.channels;
    params->bytes_per_sample = metadata->data.stream_info.bits_per_sample / 8;
    if (params->cards) {
      if (clear_alsa(params->cards))
        ret(1);
    } else {
      int fd =
          open(play_pid_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
      if (fd < 0)
        exit(1);
      pid = getpid();
      write_size = write(fd, &pid, sizeof(pid_t));
      close(fd);
      if (write_size != sizeof(pid_t)) {
        unlink(play_pid_path);
        exit(1);
      }
    }
    params->cards = init_alsa(metadata->data.stream_info.sample_rate,
                              metadata->data.stream_info.bits_per_sample,
                              metadata->data.stream_info.channels);
    if (!params->cards)
      ret(1);
    params->not_started = 1;
  }
}

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
  if (params->total_samples == 0)
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  cards_tmp = params->cards;
  while (cards_tmp) {
    while ((avail_frames = snd_pcm_avail(cards_tmp->pcm)) <
           frame->header.blocksize)
      if (avail_frames < 0)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      else
        usleep(1000);
    cards_tmp = cards_tmp->next;
  }
  for (i = 0; i < frame->header.blocksize; i++) {
    cards_tmp = params->cards;
    while (cards_tmp) {
      if (snd_pcm_avail_update(cards_tmp->pcm) < 0 ||
          snd_pcm_mmap_begin(cards_tmp->pcm, &areas, &offset, &frames) < 0)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      for (channel = 0; channel < frame->header.channels; channel++) {
        buf_tmp = areas[channel].addr + (areas[channel].first / 8) +
                  (offset * areas[channel].step / 8) + cards_tmp->off;
        memcpy(buf_tmp, (char *)(buffer[channel] + i),
               params->bytes_per_sample);
      }
      commitres = snd_pcm_mmap_commit(cards_tmp->pcm, offset, frames);
      if (commitres < 0 || commitres != frames)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      cards_tmp = cards_tmp->next;
    }
    if (params->not_started && i > 100) {
      cards_tmp = params->cards;
      while (cards_tmp) {
        if (snd_pcm_state(cards_tmp->pcm) == SND_PCM_STATE_PREPARED &&
            snd_pcm_start(cards_tmp->pcm))
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        cards_tmp = cards_tmp->next;
      }
      params->not_started = 0;
    }
  }
  params->total_samples -= frame->header.blocksize;
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder,
                                            FLAC__byte buffer[], size_t *bytes,
                                            void *client_data) {
  extract_params *params = client_data;
  if (*bytes > params->stream_length)
    *bytes = params->stream_length;
  if (fread(buffer, 1, *bytes, stdin) != *bytes)
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
  params->stream_length -= *bytes;
  if (params->stream_length == 0) {
    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
  }
  return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

int main(void) {
  extract_params params = {.rate = 0, .bytes_per_sample = 0};
  FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
  int fd = open(play_pid_path, O_RDONLY);
  if (fd >= 0) {
    pid_t run_pid;
    ssize_t read_size = read(fd, &run_pid, sizeof(pid_t));
    close(fd);
    if (read_size == sizeof(pid_t)) {
      kill(run_pid, SIGTERM);
      usleep(50000);
    }
    unlink(play_pid_path);
  }
  while ((params.stream_length = get_length()) > 0) {
    if (FLAC__stream_decoder_init_stream(
            decoder, read_callback, NULL, NULL, NULL, NULL, write_callback,
            metadata_callback, error_callback,
            &params) == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
      if (!FLAC__stream_decoder_process_until_end_of_stream(decoder))
        ret(1);
      FLAC__stream_decoder_finish(decoder);
    } else
      ret(1);
  }
  if (params.cards)
    snd_pcm_drain(params.cards->pcm);
  ret(0);
}
