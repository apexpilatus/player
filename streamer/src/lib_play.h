typedef struct data_list_t {
  struct data_list_t volatile *next;
  char volatile *buf;
  int data_size;
} data_list;

extern const unsigned int data_buf_size;
extern const unsigned int alsa_buf_size;

extern int buf_len(data_list volatile *data);
