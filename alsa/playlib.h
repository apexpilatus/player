typedef struct lst
{
  char *name;
  struct lst *next;
} file_lst;

extern char *album;
extern char *track;
extern char *card_name;
extern char *data_half;

int play_album(file_lst *files, FLAC__StreamDecoderWriteCallback write_callback, snd_pcm_t *pcm_p);
void cp_little_endian(char *buf, char *data, int samplesize);
file_lst *get_file_lst(char *dirname);
int get_params(file_lst *files, unsigned int *rate, unsigned short *sample_size);
int get_shared_vars(void);
