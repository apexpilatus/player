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

static uint32_t off;
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
  snd_pcm_t *pcm_p = (snd_pcm_t *)client_data;
  snd_pcm_sframes_t avail_frames;
  snd_pcm_sframes_t commitres;
  const snd_pcm_channel_area_t *areas;
  snd_pcm_uframes_t offset;
  snd_pcm_uframes_t frames;
  snd_pcm_uframes_t cpy_count;
  uint32_t blocksize = frame->header.blocksize;
  char channel;
  char channels = 2;
  char *buf_tmp[channels];
  while ((avail_frames = snd_pcm_avail_update(pcm_p)) < blocksize)
    if (avail_frames < 0)
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    else
      usleep(50);
  while (blocksize > 0) {
    frames = 1;
    if (snd_pcm_mmap_begin(pcm_p, &areas, &offset, &frames) < 0)
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    for (channel = 0; channel < channels; channel++) {
      buf_tmp[channel] = areas[channel].addr + (areas[channel].first / 8) +
                         (offset * areas[channel].step / 8) + off;
      memcpy(buf_tmp[channel],
             buffer[channel] + frame->header.blocksize - blocksize,
             bytes_per_sample);
    }
    commitres = snd_pcm_mmap_commit(pcm_p, offset, frames);
    if (commitres < 0 || commitres != frames)
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    blocksize -= commitres;
  }
  if (snd_pcm_state(pcm_p) == SND_PCM_STATE_PREPARED && snd_pcm_start(pcm_p))
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder,
                       const FLAC__StreamMetadata *metadata,
                       void *client_data) {}

void error_callback(const FLAC__StreamDecoder *decoder,
                    FLAC__StreamDecoderErrorStatus status, void *client_data) {}

static int play_album(track_list *tracks,
                      FLAC__StreamDecoderWriteCallback write_callback,
                      snd_pcm_t *pcm_p) {
  FLAC__StreamDecoder *decoder = NULL;
  decoder = FLAC__stream_decoder_new();
  FLAC__stream_decoder_set_md5_checking(decoder, false);
  FLAC__stream_decoder_set_metadata_ignore_all(decoder);
  while (tracks) {
    FLAC__StreamDecoderInitStatus init_status;
    init_status = FLAC__stream_decoder_init_file(
        decoder, tracks->file_name, write_callback, metadata_callback,
        error_callback, pcm_p);
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
  snd_pcm_drain(pcm_p);
  return 0;
}

static int init_alsa(snd_pcm_t **pcm_p, track_list *tracks) {
  int card = -1;
  char card_name[10];
  FLAC__StreamMetadata *rate =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
  snd_pcm_hw_params_t *pcm_hw;
  if (snd_card_next(&card) || card == -1)
    return 1;
  sprintf(card_name, "hw:%d,0", card);
  if (snd_pcm_open(pcm_p, card_name, SND_PCM_STREAM_PLAYBACK, 0))
    return 1;
  if (!FLAC__metadata_get_streaminfo(tracks->file_name, rate))
    return 1;
  snd_pcm_hw_params_malloc(&pcm_hw);
  snd_pcm_hw_params_any(*pcm_p, pcm_hw);
  if (snd_pcm_hw_params_test_rate(*pcm_p, pcm_hw,
                                  rate->data.stream_info.sample_rate, 0))
    return 1;
  snd_pcm_hw_params_set_rate(*pcm_p, pcm_hw, rate->data.stream_info.sample_rate,
                             0);
  if (rate->data.stream_info.bits_per_sample == 16) {
    if (snd_pcm_hw_params_test_format(*pcm_p, pcm_hw, SND_PCM_FORMAT_S16))
      return 1;
    snd_pcm_hw_params_set_format(*pcm_p, pcm_hw, SND_PCM_FORMAT_S16);
  } else if (rate->data.stream_info.bits_per_sample == 24) {
    if (!snd_pcm_hw_params_test_format(*pcm_p, pcm_hw,
                                       SND_PCM_FORMAT_S24_3LE)) {
      snd_pcm_hw_params_set_format(*pcm_p, pcm_hw, SND_PCM_FORMAT_S24_3LE);
    } else if (!snd_pcm_hw_params_test_format(*pcm_p, pcm_hw,
                                              SND_PCM_FORMAT_S32)) {
      snd_pcm_hw_params_set_format(*pcm_p, pcm_hw, SND_PCM_FORMAT_S32);
      off = 1;
    } else
      return 1;
  } else
    return 1;
  bytes_per_sample = rate->data.stream_info.bits_per_sample / 8;
  snd_pcm_hw_params_set_access(*pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
  if (snd_pcm_hw_params(*pcm_p, pcm_hw) || snd_pcm_prepare(*pcm_p))
    return 1;
  return 0;
}

static void create_cmd(char *cmd, track_list *tracks) {
  strcpy(cmd, "streamer=$(grep streamer /etc/hosts|awk '{print$2}');");
  strcat(cmd, "if [ -z \"$streamer\" ];then exit 1;fi;echo \"");
  while (tracks) {
    strcat(cmd, "$(pwd)/");
    strcat(cmd, tracks->file_name);
    if (tracks->next)
      strcat(cmd, "|");
    tracks = tracks->next;
  }
  strcat(cmd, "\"|nc -w 1 $streamer 9696");
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char *rsp = malloc(getpagesize());
  char *cmd = malloc(getpagesize());
  snd_pcm_t *pcm_p;
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
  create_cmd(cmd, tracks);
  if (!system(cmd)) {
    write_size = write(sock, rsp, strlen(rsp));
    if (write_size != strlen(rsp))
      return 1;
    else
      return 0;
  }
  if (init_alsa(&pcm_p, tracks))
    execl(resp_err, "resp_err", prm[1], NULL);
  if (utime(".", NULL))
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
  close(sock);
  return play_album(tracks, write_callback, pcm_p);
}
