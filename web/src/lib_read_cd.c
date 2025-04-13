#include "lib_read_cd.h"
#include <cdda_interface.h>
#include <cdda_paranoia.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

data_list volatile *volatile data_first;
long volatile first_sector;
long volatile last_sector;
char volatile in_work = 1;

static void callback(long inpos, int function) {}

int cd_reader(void *prm) {
  cdrom_drive *d = prm;
  data_list volatile *data_new = NULL;
  long cursor;
  cdrom_paranoia *p = paranoia_init(d);
  pid_t pid = getpid();
  paranoia_modeset(p, PARANOIA_MODE_FULL ^ PARANOIA_MODE_NEVERSKIP);
  paranoia_seek(p, cursor = first_sector, SEEK_SET);
  while (cursor <= last_sector) {
    int16_t *readbuf = paranoia_read_limited(p, callback, 5);
    if (readbuf == NULL) {
      paranoia_free(p);
      cdda_close(d);
      kill(pid, SIGTERM);
    } else {
      if (!data_new) {
        data_first = malloc(sizeof(data_list));
        data_first->next = NULL;
        data_first->buf = malloc(CD_FRAMESIZE_RAW);
        data_new = data_first;
      } else {
        data_new->next = malloc(sizeof(data_list));
        data_new->next->next = NULL;
        data_new = data_new->next;
        data_new->buf = malloc(CD_FRAMESIZE_RAW);
      }
      memcpy((char *)data_new->buf, readbuf, CD_FRAMESIZE_RAW);
    }
    cursor++;
  }
  paranoia_free(p);
  cdda_close(d);
  in_work = 0;
  return 0;
}

int filled_buf_check(data_list *data) {
  int count = 0;
  while (data) {
    count++;
    if (count > 200)
      return 0;
    data = (data_list *)data->next;
  }
  return 1;
}