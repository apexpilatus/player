#include "lib_play.h"

const unsigned int data_buf_size = 6000;
const unsigned int alsa_buf_size = data_buf_size * 6;

int buf_len(data_list volatile *data) {
  int count = 0;
  while (data) {
    count++;
    data = data->next;
  }
  return count;
}