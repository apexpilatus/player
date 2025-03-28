#include <dirent.h>
#include <fcntl.h>
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

typedef struct track_list_t {
  struct track_list_t *next;
  char *file_name;
  char *track_number;
} track_list;

typedef struct card_list_t {
  struct card_list_t *next;
  snd_pcm_t *pcm;
  uint32_t off;
} card_list;

static long bytes_per_sample;

static void sort_tracks(track_list *track_first) {
  char *file_name_tmp;
  char *track_number_tmp;
  for (track_list *go_slow = track_first; go_slow && go_slow->next;
       go_slow = go_slow->next)
    for (track_list *go_fast = go_slow->next; go_fast; go_fast = go_fast->next)
      if (strtol(go_slow->track_number, NULL, 10) >
          strtol(go_fast->track_number, NULL, 10)) {
        file_name_tmp = go_fast->file_name;
        track_number_tmp = go_fast->track_number;
        go_fast->file_name = go_slow->file_name;
        go_fast->track_number = go_slow->track_number;
        go_slow->file_name = file_name_tmp;
        go_slow->track_number = track_number_tmp;
      }
}

static track_list *get_tracks_in_dir(char *url) {
  char *album_dir;
  char *start_track;
  DIR *dp;
  struct dirent *ep;
  track_list *track_first = NULL;
  track_list *track_tmp = NULL;
  FLAC__StreamMetadata *tags =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
  album_dir = strchr(url, '?');
  if (!album_dir)
    goto exit;
  start_track = strchr(++album_dir, '&');
  if (start_track) {
    *start_track = '\0';
    start_track++;
  }
  if (chdir(album_dir))
    goto exit;
  dp = opendir(".");
  if (dp) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG && FLAC__metadata_get_tags(ep->d_name, &tags)) {
        for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++)
          if (!strncmp("TRACKNUMBER=",
                       (char *)tags->data.vorbis_comment.comments[i].entry,
                       strlen("TRACKNUMBER="))) {
            if (!start_track ||
                strtol((char *)(tags->data.vorbis_comment.comments[i].entry +
                                strlen("TRACKNUMBER=")),
                       NULL, 10) >= strtol(start_track, NULL, 10)) {
              if (!track_first) {
                track_tmp = malloc(sizeof(track_list));
                track_first = track_tmp;
              } else {
                track_tmp->next = malloc(sizeof(track_list));
                track_tmp = track_tmp->next;
              }
              memset(track_tmp, 0, sizeof(track_list));
              track_tmp->track_number =
                  malloc(tags->data.vorbis_comment.comments[i].length + 1);
              strcpy(track_tmp->track_number,
                     (char *)(tags->data.vorbis_comment.comments[i].entry +
                              strlen("TRACKNUMBER=")));
              track_tmp->file_name = malloc(strlen(ep->d_name) + 1);
              strcpy(track_tmp->file_name, ep->d_name);
            }
            break;
          }
      }
    closedir(dp);
  }
  sort_tracks(track_first);
exit:
  return track_first;
}

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

static int play_album(track_list *tracks,
                      FLAC__StreamDecoderWriteCallback write_callback,
                      card_list *cards) {
  FLAC__StreamDecoder *decoder = NULL;
  decoder = FLAC__stream_decoder_new();
  FLAC__stream_decoder_set_md5_checking(decoder, false);
  FLAC__stream_decoder_set_metadata_ignore_all(decoder);
  while (tracks) {
    FLAC__StreamDecoderInitStatus init_status;
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
    snd_pcm_t *pcm_p = NULL;
    uint32_t off = 0;
    snd_pcm_hw_params_t *pcm_hw;
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
  sprintf(cmd + cmd_end, "\"|nc -w 1 %s 9696 1>/dev/null 2>/dev/null",
          client_address);
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char *rsp = malloc(getpagesize());
  char *cmd = malloc(getpagesize());
  card_list *cards;
  track_list *tracks;
  int fd;
  pid_t pid = getpid();
  tracks = get_tracks_in_dir(prm[2]);
  if (!tracks)
    execl(resp_err, "resp_err", prm[1], NULL);
  fd = open(play_pid_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0)
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(fd, &pid, sizeof(pid_t));
  close(fd);
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  create_cmd(cmd, tracks, prm[3]);
  if (!system(cmd)) {
    if (utime(".", NULL))
      execl(resp_err, "resp_err", prm[1], NULL);
    write_size = write(sock, rsp, strlen(rsp));
    if (write_size != strlen(rsp))
      return 1;
    else
      return 0;
  }
  cards = init_alsa(tracks);
  if (!cards)
    execl(resp_err, "resp_err", prm[1], NULL);
  if (utime(".", NULL))
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
  close(sock);
  return play_album(tracks, write_callback, cards);
}
