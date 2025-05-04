#include "lib_flac_tracks.h"
#include <string.h>
#include <unistd.h>
#include <utime.h>

// clang-format off
#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>
// clang-format on

static int min_range;
static int max_range;
static int bytes_left;
static int bytes_per_sample;
static int header_size;
static int skip_count;

static int write_header(int fd, int size, FLAC__StreamMetadata *stream_inf) {
  int int_var;
  short short_var;
  ssize_t write_size = 0;
  write_size = write(fd, "RIFF", 4);
  int_var =
      size + (stream_inf->data.stream_info.bits_per_sample == 16 ? 36 : 72);
  write_size += write(fd, &int_var, 4);
  write_size += write_size = write(fd, "WAVEfmt ", 8);
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

void metadata_callback(const FLAC__StreamDecoder *decoder,
                       const FLAC__StreamMetadata *metadata,
                       void *client_data) {}

void error_callback(const FLAC__StreamDecoder *decoder,
                    FLAC__StreamDecoderErrorStatus status, void *client_data) {}

FLAC__StreamDecoderWriteStatus
write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
               const FLAC__int32 *const buffer[], void *client_data) {
  if ((frame->header.blocksize * 2 * bytes_per_sample) + skip_count <=
      min_range) {
    skip_count += frame->header.blocksize * 2 * bytes_per_sample;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
  }
  for (int i = 0; i < frame->header.blocksize; i++) {
    int j;
    for (j = 0; j < bytes_per_sample; j++)
      if (skip_count == min_range) {
        if (write(*((int *)client_data), (char *)(buffer[0] + i) + j, 1) != 1)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        bytes_left -= 1;
        if (bytes_left == 0)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      } else
        skip_count++;
    for (j = 0; j < bytes_per_sample; j++)
      if (skip_count == min_range) {
        if (write(*((int *)client_data), (char *)(buffer[1] + i) + j, 1) != 1)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        bytes_left -= 1;
        if (bytes_left == 0)
          return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      } else
        skip_count++;
  }
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static int get_album_size(track_list *tracks,
                          FLAC__StreamMetadata *stream_inf) {
  int ret = 0;
  while (tracks) {
    if (!FLAC__metadata_get_streaminfo(tracks->file_name, stream_inf))
      return 0;
    ret += stream_inf->data.stream_info.total_samples;
    tracks = tracks->next;
  }
  return ret;
}

static int extract_tracks(track_list *tracks, void *client_data) {
  FLAC__StreamDecoder *decoder = NULL;
  FLAC__StreamDecoderInitStatus init_status;
  decoder = FLAC__stream_decoder_new();
  FLAC__stream_decoder_set_md5_checking(decoder, false);
  FLAC__stream_decoder_set_metadata_ignore_all(decoder);
  while (tracks) {
    init_status = FLAC__stream_decoder_init_file(
        decoder, tracks->file_name, write_callback, metadata_callback,
        error_callback, client_data);
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

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  int flac_blocks_size = 0;
  char *end;
  ssize_t write_size;
  char rsp[getpagesize()];
  FLAC__StreamMetadata *stream_inf =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
  track_list *tracks = get_tracks_in_dir(prm[2]);
  if (!tracks)
    execl(resp_err, "resp_err", prm[1], NULL);
  if (!FLAC__metadata_get_streaminfo(tracks->file_name, stream_inf))
    execl(resp_err, "resp_err", prm[1], NULL);
  bytes_per_sample = stream_inf->data.stream_info.bits_per_sample / 8;
  header_size = stream_inf->data.stream_info.bits_per_sample == 16 ? 44 : 68;
  if ((end = strchr(prm[3], '-')) && strlen(++end) > 0)
    max_range = strtol(end, NULL, 10);
  else {
    if ((flac_blocks_size = get_album_size(tracks, stream_inf)) == 0)
      execl(resp_err, "resp_err", prm[1], NULL);
    max_range = (flac_blocks_size * 2 * bytes_per_sample) - 1;
  }
  if (end = strchr(prm[3], '-')) {
    *end = '\0';
    min_range = strtol(prm[3], NULL, 10);
  }
  if (min_range > 0 && min_range < header_size)
    execl(resp_err, "resp_err", prm[1], NULL);
  bytes_left = max_range - min_range + 1;
  if (min_range == 0 && max_range < header_size - 1) {
    char buf[bytes_left];
    if (!flac_blocks_size)
      if ((flac_blocks_size = get_album_size(tracks, stream_inf)) == 0)
        execl(resp_err, "resp_err", prm[1], NULL);
    if (utime(".", NULL))
      execl(resp_err, "resp_err", prm[1], NULL);
    sprintf(rsp, "%s\r\n%s%d\r\nContent-Range: bytes %d-%d/%d\r\n%s\r\n\r\n",
            "HTTP/1.1 200 OK", "Content-Length: ", bytes_left, min_range,
            max_range, (flac_blocks_size * 2 * bytes_per_sample) + header_size,
            "Content-Type: audio/wav");
    write_size = write(sock, rsp, strlen(rsp));
    write_size += write(sock, buf, bytes_left);
    if (write_size == strlen(rsp) + bytes_left)
      return 0;
    else
      return 1;
  }
  sprintf(rsp, "%s\r\n%s%d\r\nContent-Range: bytes %d-%d/%d\r\n%s\r\n\r\n",
          "HTTP/1.1 200 OK", "Content-Length: ", bytes_left, min_range,
          max_range, 0, "Content-Type: audio/wav");
  if (write(sock, rsp, strlen(rsp)) != strlen(rsp))
    return 1;
  if (min_range == 0) {
    if (!flac_blocks_size)
      if ((flac_blocks_size = get_album_size(tracks, stream_inf)) == 0)
        execl(resp_err, "resp_err", prm[1], NULL);
    if (write_header(sock, flac_blocks_size * 2 * bytes_per_sample, stream_inf))
      return 1;
    else {
      bytes_left -= header_size;
      min_range += header_size;
    }
  }
  skip_count += header_size;
  return extract_tracks(tracks, &sock);
}