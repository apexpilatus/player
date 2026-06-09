#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FLAC__uint64 total_samples;
char *header;

int read_hdr() {
  ssize_t msg_size = getpagesize();
  ssize_t read_size = 0;
  char *hdr = malloc(msg_size);
  while (read_size < msg_size && fread(hdr + read_size, 1, 1, stdin) == 1) {
    read_size++;
    if (read_size < msg_size)
      hdr[read_size] = '\0';
    if (read_size > 3 && !strcmp(hdr + read_size - 4, "\r\n\r\n"))
      break;
  }
  if (read_size == msg_size || read_size < 9 || strstr(hdr, "404 shit happens"))
    return 1;
  printf("hdr size - %ld\n%s", read_size, hdr);
  header = hdr;
  return 0;
}

void error_callback(const FLAC__StreamDecoder *decoder,
                    FLAC__StreamDecoderErrorStatus status, void *client_data) {
  printf("flac error\n");
}

void metadata_callback(const FLAC__StreamDecoder *decoder,
                       const FLAC__StreamMetadata *metadata,
                       void *client_data) {
  total_samples = metadata->data.stream_info.total_samples;
  printf("%u - %u\n", metadata->data.stream_info.bits_per_sample,
         metadata->data.stream_info.sample_rate);
}

FLAC__StreamDecoderWriteStatus
write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
               const FLAC__int32 *const buffer[], void *client_data) {
  total_samples -= frame->header.blocksize;
  printf("%ld;", total_samples);
  usleep(5000);
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

int extract_track() {
  FLAC__StreamDecoder *decoder = NULL;
  FLAC__StreamDecoderInitStatus init_status;
  decoder = FLAC__stream_decoder_new();
  init_status = FLAC__stream_decoder_init_FILE(
      decoder, stdin, write_callback, metadata_callback, error_callback, NULL);
  if (init_status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)) {
      return 1;
    }
    FLAC__stream_decoder_finish(decoder);
  } else {
    return 1;
  }
  return 0;
}

int main(void) {
  printf("c start\n");
  if (!read_hdr()) {
    printf("header ok\n");
    if (!extract_track()) {
      printf("\nextracted\n");
    } else {
      printf("not extracted");
    }
  } else {
    printf("bad header");
  }
  printf("c after while\n");
  printf("c end\n");
  return 0;
}
