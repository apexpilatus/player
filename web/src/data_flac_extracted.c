#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>
// clang-format off
#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>
// clang-format on

typedef struct track_list_t {
  struct track_list_t *next;
  char *file_name;
  char *track_number;
} track_list;

typedef struct data_list_t {
  struct data_list_t volatile *next;
  char volatile *buf;
  int data_size;
} data_list;

typedef struct {
  unsigned int bytes_left;
  unsigned int bytes_per_sample;
  unsigned int bytes_skip;
} extract_params;

typedef struct {
  extract_params params;
  track_list *tracks;
} thread_params;

const unsigned int data_buf_size = 18000;
data_list volatile *volatile data_first;
data_list volatile *volatile data_new;
char volatile in_work = 1;

void sort_tracks(track_list *track_first) {
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

track_list *get_tracks_in_dir(char *url) {
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

int write_header(int fd, unsigned int size, FLAC__StreamMetadata *stream_inf) {
  unsigned int int_var;
  short short_var;
  ssize_t write_size = 0;
  write_size = write(fd, "RIFF", 4);
  int_var =
      size + (stream_inf->data.stream_info.bits_per_sample == 16 ? 36 : 72);
  write_size += write(fd, &int_var, 4);
  write_size += write(fd, "WAVEfmt ", 8);
  int_var = (stream_inf->data.stream_info.bits_per_sample == 16 ? 16 : 40);
  write_size += write(fd, &int_var, 4);
  short_var =
      (stream_inf->data.stream_info.bits_per_sample == 16 ? 0x0001 : 0xFFFE);
  write_size += write(fd, &short_var, 2);
  short_var = 2;
  write_size += write(fd, &short_var, 2);
  int_var = stream_inf->data.stream_info.sample_rate;
  write_size += write(fd, &int_var, 4);
  int_var *= stream_inf->data.stream_info.bits_per_sample / 8;
  int_var *= 2;
  write_size += write(fd, &int_var, 4);
  short_var = stream_inf->data.stream_info.bits_per_sample / 8 * 2;
  write_size += write(fd, &short_var, 2);
  short_var = stream_inf->data.stream_info.bits_per_sample;
  write_size += write(fd, &short_var, 2);
  if (stream_inf->data.stream_info.bits_per_sample != 16) {
    short_var = 22;
    write_size += write(fd, &short_var, 2);
    short_var = stream_inf->data.stream_info.bits_per_sample;
    write_size += write(fd, &short_var, 2);
    int_var = 0;
    write_size += write(fd, &int_var, 4);
    short_var = 0x0001;
    write_size += write(fd, &short_var, 2);
    write_size += write(
        fd, "\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71", 14);
  }
  write_size += write(fd, "data", 4);
  write_size += write(fd, &size, 4);
  if (write_size !=
      (stream_inf->data.stream_info.bits_per_sample == 16 ? 44 : 68))
    return 1;
  else
    return 0;
}

data_list volatile *volatile new_data(data_list volatile *volatile data_pv) {
  data_list volatile *volatile data_new = malloc(sizeof(data_list));
  data_new->next = NULL;
  data_new->buf = malloc(data_buf_size);
  data_new->data_size = 0;
  data_pv->next = data_new;
  return data_new;
}

int buf_len(data_list volatile *data) {
  int count = 0;
  while (data) {
    count++;
    data = data->next;
  }
  return count;
}

void metadata_callback(const FLAC__StreamDecoder *decoder,
                       const FLAC__StreamMetadata *metadata,
                       void *client_data) {}

void error_callback(const FLAC__StreamDecoder *decoder,
                    FLAC__StreamDecoderErrorStatus status, void *client_data) {}

FLAC__StreamDecoderWriteStatus
write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
               const FLAC__int32 *const buffer[], void *client_data) {
  extract_params *params = client_data;
  if (frame->header.blocksize * 2 * params->bytes_per_sample <=
      params->bytes_skip) {
    params->bytes_skip -=
        frame->header.blocksize * 2 * params->bytes_per_sample;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
  }
  for (int i = 0; i < frame->header.blocksize; i++) {
    int j;
    for (j = 0; j < params->bytes_per_sample; j++)
      if (!params->bytes_skip) {
        if (data_new->data_size == data_buf_size)
          data_new = new_data(data_new);
        data_new->buf[data_new->data_size] = *((char *)(buffer[0] + i) + j);
        data_new->data_size++;
        params->bytes_left--;
        if (params->bytes_left == 0)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      } else
        params->bytes_skip--;
    for (j = 0; j < params->bytes_per_sample; j++)
      if (!params->bytes_skip) {
        if (data_new->data_size == data_buf_size)
          data_new = new_data(data_new);
        data_new->buf[data_new->data_size] = *((char *)(buffer[1] + i) + j);
        data_new->data_size++;
        params->bytes_left--;
        if (params->bytes_left == 0)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      } else
        params->bytes_skip--;
  }
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

unsigned int get_album_size(track_list *tracks,
                            FLAC__StreamMetadata *stream_inf) {
  unsigned int ret = 0;
  while (tracks) {
    if (!FLAC__metadata_get_streaminfo(tracks->file_name, stream_inf))
      return 0;
    ret += stream_inf->data.stream_info.total_samples;
    tracks = tracks->next;
  }
  return ret;
}

int extract_tracks(void *client_data) {
  thread_params *params = client_data;
  FLAC__StreamDecoder *decoder = NULL;
  FLAC__StreamDecoderInitStatus init_status;
  decoder = FLAC__stream_decoder_new();
  FLAC__stream_decoder_set_md5_checking(decoder, false);
  FLAC__stream_decoder_set_metadata_ignore_all(decoder);
  while (params->tracks) {
    init_status = FLAC__stream_decoder_init_file(
        decoder, params->tracks->file_name, write_callback, metadata_callback,
        error_callback, &params->params);
    if (init_status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
      if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)) {
        FLAC__stream_decoder_finish(decoder);
        break;
      }
      FLAC__stream_decoder_finish(decoder);
    } else {
      kill(getpid(), SIGTERM);
    }
    params->tracks = params->tracks->next;
  }
  in_work = 0;
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char *end;
  ssize_t write_size;
  char rsp[getpagesize()];
  unsigned int min_range = 0;
  unsigned int max_range;
  unsigned int header_size;
  thrd_t thr;
  FLAC__StreamMetadata *stream_inf =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
  unsigned int flac_blocks_size;
  thread_params params;
  data_list volatile *data_cur;
  params.tracks = get_tracks_in_dir(prm[2]);
  data_first = malloc(sizeof(data_list));
  data_new = data_first;
  data_new->next = NULL;
  data_new->buf = malloc(data_buf_size);
  data_new->data_size = 0;
  if (!(params.tracks &&
        (flac_blocks_size = get_album_size(params.tracks, stream_inf))))
    execl(resp_err, "resp_err", prm[1], NULL);
  params.params.bytes_per_sample =
      stream_inf->data.stream_info.bits_per_sample / 8;
  header_size = stream_inf->data.stream_info.bits_per_sample == 16 ? 44 : 68;
  if ((end = strchr(prm[3], '-')) && strlen(++end) > 0)
    max_range = strtol(end, NULL, 10);
  else {
    max_range = (flac_blocks_size * 2 * params.params.bytes_per_sample) - 1 +
                header_size;
  }
  if ((end = strchr(prm[3], '-'))) {
    *end = '\0';
    min_range = strtol(prm[3], NULL, 10);
  }
  if (min_range > 0 && min_range < header_size)
    execl(resp_err, "resp_err", prm[1], NULL);
  params.params.bytes_left = max_range - min_range + 1;
  if (min_range == 0 && max_range < header_size - 1) {
    char buf[params.params.bytes_left];
    sprintf(rsp, "%s\r\n%s%u\r\nContent-Range: bytes %u-%u/%u\r\n%s\r\n\r\n",
            "HTTP/1.1 200 OK", "Content-Length: ", params.params.bytes_left,
            min_range, max_range,
            (flac_blocks_size * 2 * params.params.bytes_per_sample) +
                header_size,
            "Content-Type: audio/wav");
    write_size = write(sock, rsp, strlen(rsp));
    write_size += write(sock, buf, params.params.bytes_left);
    if (write_size == strlen(rsp) + params.params.bytes_left)
      return 0;
    else
      return 1;
  }
  sprintf(rsp, "%s\r\n%s%u\r\nContent-Range: bytes %u-%u/%u\r\n%s\r\n\r\n",
          "HTTP/1.1 200 OK", "Content-Length: ", params.params.bytes_left,
          min_range, max_range,
          (flac_blocks_size * 2 * params.params.bytes_per_sample) + header_size,
          "Content-Type: audio/wav");
  if (write(sock, rsp, strlen(rsp)) != strlen(rsp))
    return 1;
  if (min_range == 0) {
    if (write_header(sock,
                     flac_blocks_size * 2 * params.params.bytes_per_sample,
                     stream_inf))
      return 1;
    else {
      params.params.bytes_skip = 0;
      params.params.bytes_left -= header_size;
    }
  } else
    params.params.bytes_skip = min_range - header_size;
  while (params.tracks) {
    if (!FLAC__metadata_get_streaminfo(params.tracks->file_name, stream_inf))
      execl(resp_err, "resp_err", prm[1], NULL);
    if (params.params.bytes_skip >= stream_inf->data.stream_info.total_samples *
                                        2 * params.params.bytes_per_sample) {
      params.params.bytes_skip -= stream_inf->data.stream_info.total_samples *
                                  2 * params.params.bytes_per_sample;
      params.tracks = params.tracks->next;
    } else
      break;
  }
  if (thrd_create(&thr, extract_tracks, &params) != thrd_success ||
      thrd_detach(thr) != thrd_success)
    return 1;
  data_cur = data_first;
  while (data_cur) {
    while (in_work && buf_len(data_cur) < 400)
      usleep(100000);
    for (write_size = 0; write_size < data_cur->data_size;) {
      ssize_t write_size_pv = write_size;
      write_size = write(sock, (char *)(data_cur->buf + write_size),
                         data_cur->data_size - write_size);
      if (write_size < 0)
        return 1;
      write_size += write_size_pv;
    }
    data_cur = data_cur->next;
  }
}
