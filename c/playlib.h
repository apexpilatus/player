typedef struct lst{
  char *name;
  struct lst *next;
} file_lst;
typedef FLAC__StreamDecoderWriteStatus (*wr_cb_func)(const FLAC__StreamDecoder *, const FLAC__Frame *, const FLAC__int32 * const [], void *);

int play_album(file_lst *files, wr_cb_func write_callback, snd_pcm_t *pcm_p);
void cp_little_endian(unsigned char *buf, FLAC__uint32 data, int samplesize);
file_lst* get_file_lst(char *dirname);
int get_params(file_lst *files, unsigned int *rate, unsigned short *sample_size);
