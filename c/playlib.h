typedef struct lst{
  char *name;
  struct lst *next;
} file_lst;

int play_album(file_lst *files, FLAC__StreamDecoderWriteStatus (*write_callback)(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data), snd_pcm_t *pcm_p);
