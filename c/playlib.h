typedef struct lst{
  char *name;
  struct lst *next;
} file_lst;

int play_album(file_lst *files, FLAC__StreamDecoderWriteCallback write_callback, snd_pcm_t *pcm_p);
void cp_little_endian(unsigned char *buf, FLAC__uint32 data, int samplesize);
file_lst* get_file_lst(char *dirname);
int get_params(file_lst *files, unsigned int *rate, unsigned short *sample_size);
