#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

typedef struct track_list_t {
  struct track_list_t *next;
  char *file_name;
  char *track_number;
} track_list;

typedef struct {
  unsigned int bytes_left;
  unsigned int bytes_per_sample;
  unsigned int bytes_skip;
} extract_params;

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

track_list *get_tracks_in_dir(char *params) {
  char *album = strstr(params, "album=");
  char *start_track = strstr(params, "track=");
  DIR *dp;
  struct dirent *ep;
  track_list *track_first = NULL;
  track_list *track_tmp = NULL;
  FLAC__StreamMetadata *tags =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
  if (album) {
    char *end = strchr(album, '&');
    if (end)
      *end = '\0';
    album += 6;
  } else
    goto exit;
  if (start_track) {
    char *end = strchr(start_track, '&');
    if (end)
      *end = '\0';
    start_track += 6;
  }
  if (chdir(album))
    goto exit;
  dp = opendir(".");
  if (dp) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG && FLAC__metadata_get_tags(ep->d_name, &tags)) {
        int i;
        for (i = 0; i < tags->data.vorbis_comment.num_comments; i++)
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
  if (track_first && !start_track)
    while (track_first->next)
      track_first = track_first->next;
exit:
  return track_first;
}

int write_header(unsigned int size, FLAC__StreamMetadata *stream_inf) {
  unsigned int int_var;
  short short_var;
  size_t write_size;
  write_size = fwrite("RIFF", 1, 4, stdout);
  int_var =
      size + (stream_inf->data.stream_info.bits_per_sample == 16 ? 36 : 72);
  write_size += fwrite(&int_var, 1, 4, stdout);
  write_size += fwrite("WAVEfmt ", 1, 8, stdout);
  int_var = (stream_inf->data.stream_info.bits_per_sample == 16 ? 16 : 40);
  write_size += fwrite(&int_var, 1, 4, stdout);
  short_var =
      (stream_inf->data.stream_info.bits_per_sample == 16 ? 0x0001 : 0xFFFE);
  write_size += fwrite(&short_var, 1, 2, stdout);
  short_var = 2;
  write_size += fwrite(&short_var, 1, 2, stdout);
  int_var = stream_inf->data.stream_info.sample_rate;
  write_size += fwrite(&int_var, 1, 4, stdout);
  int_var *= stream_inf->data.stream_info.bits_per_sample / 8;
  int_var *= 2;
  write_size += fwrite(&int_var, 1, 4, stdout);
  short_var = stream_inf->data.stream_info.bits_per_sample / 8 * 2;
  write_size += fwrite(&short_var, 1, 2, stdout);
  short_var = stream_inf->data.stream_info.bits_per_sample;
  write_size += fwrite(&short_var, 1, 2, stdout);
  if (stream_inf->data.stream_info.bits_per_sample != 16) {
    short_var = 22;
    write_size += fwrite(&short_var, 1, 2, stdout);
    short_var = stream_inf->data.stream_info.bits_per_sample;
    write_size += fwrite(&short_var, 1, 2, stdout);
    int_var = 0;
    write_size += fwrite(&int_var, 1, 4, stdout);
    short_var = 0x0001;
    write_size += fwrite(&short_var, 1, 2, stdout);
    write_size +=
        fwrite("\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71", 1,
               14, stdout);
  }
  write_size += fwrite("data", 1, 4, stdout);
  write_size += fwrite(&size, 1, 4, stdout);
  if (write_size !=
      (stream_inf->data.stream_info.bits_per_sample == 16 ? 44 : 68))
    return 1;
  else
    return 0;
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
  int i;
  for (i = 0; i < frame->header.blocksize; i++) {
    int j;
    for (j = 0; j < params->bytes_per_sample; j++)
      if (!params->bytes_skip) {
        if (fwrite((char *)(buffer[0] + i) + j, 1, 1, stdout) != 1)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        params->bytes_left--;
        if (params->bytes_left == 0)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      } else
        params->bytes_skip--;
    for (j = 0; j < params->bytes_per_sample; j++)
      if (!params->bytes_skip) {
        if (fwrite((char *)(buffer[1] + i) + j, 1, 1, stdout) != 1)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
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

int extract_tracks(track_list *tracks, void *params) {
  FLAC__StreamDecoder *decoder = NULL;
  FLAC__StreamDecoderInitStatus init_status;
  decoder = FLAC__stream_decoder_new();
  FLAC__stream_decoder_set_md5_checking(decoder, false);
  FLAC__stream_decoder_set_metadata_ignore_all(decoder);
  while (tracks) {
    init_status = FLAC__stream_decoder_init_file(
        decoder, tracks->file_name, write_callback, metadata_callback,
        error_callback, params);
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

int err(void) {
  printf("%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 404 shit happens",
         "Cache-control: no-cache", "X-Content-Type-Options: nosniff");
  return 1;
}

int main(int prm_n, char *prm[]) {
  char *end;
  unsigned int min_range = 0;
  unsigned int max_range;
  unsigned int header_size;
  FLAC__StreamMetadata *stream_inf =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
  unsigned int flac_blocks_size;
  extract_params params;
  track_list *tracks = get_tracks_in_dir(prm[1]);
  if (!(tracks && (flac_blocks_size = get_album_size(tracks, stream_inf))))
    return err();
  params.bytes_per_sample = stream_inf->data.stream_info.bits_per_sample / 8;
  header_size = stream_inf->data.stream_info.bits_per_sample == 16 ? 44 : 68;
  if ((end = strchr(prm[2], '-')) && strlen(++end) > 0)
    max_range = strtol(end, NULL, 10);
  else {
    max_range =
        (flac_blocks_size * 2 * params.bytes_per_sample) - 1 + header_size;
  }
  if ((end = strchr(prm[2], '-'))) {
    *end = '\0';
    min_range = strtol(prm[2], NULL, 10);
  }
  if (min_range > 0 && min_range < header_size)
    return err();
  params.bytes_left = max_range - min_range + 1;
  printf("%s\r\n%s%u\r\nContent-Range: bytes %u-%u/%u\r\n%s\r\n\r\n",
         "HTTP/1.1 200 OK", "Content-Length: ", params.bytes_left, min_range,
         max_range,
         (flac_blocks_size * 2 * params.bytes_per_sample) + header_size,
         "Content-Type: audio/wav");
  if (min_range == 0) {
    if (write_header(flac_blocks_size * 2 * params.bytes_per_sample,
                     stream_inf))
      return 1;
    else {
      params.bytes_skip = 0;
      if (params.bytes_left > header_size)
        params.bytes_left -= header_size;
      else
        return 0;
    }
  } else
    params.bytes_skip = min_range - header_size;
  while (tracks) {
    if (!FLAC__metadata_get_streaminfo(tracks->file_name, stream_inf))
      return 1;
    if (params.bytes_skip >= stream_inf->data.stream_info.total_samples * 2 *
                                 params.bytes_per_sample) {
      params.bytes_skip -= stream_inf->data.stream_info.total_samples * 2 *
                           params.bytes_per_sample;
      tracks = tracks->next;
    } else
      break;
  }
  return extract_tracks(tracks, &params);
}
