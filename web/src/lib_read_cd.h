typedef struct data_list_t {
  struct data_list_t volatile *next;
  char volatile *buf;
} data_list;

extern data_list volatile *volatile data_first;
extern int volatile first_track;
extern char volatile in_work;

int cd_reader(void *prm);
int filled_buf_check(data_list *data);