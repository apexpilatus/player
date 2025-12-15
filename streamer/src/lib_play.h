typedef struct data_list_t {
  struct data_list_t volatile *next;
  char volatile *buf;
  int data_size;
} data_list;

extern const unsigned int data_buf_size;
extern const unsigned int alsa_buf_size;

extern int send_request(int sock, char *prm[]);
extern int buf_len(data_list volatile *data);
extern int read_headers(int sock, unsigned int *rate,
                        unsigned short *bits_per_sample,
                        unsigned int *bytes_left);
