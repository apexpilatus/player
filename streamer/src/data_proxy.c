#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

typedef struct {
  int sock;
  unsigned int bytes_left;
} read_params;

typedef struct data_list_t {
  struct data_list_t volatile *next;
  char volatile *buf;
  int data_size;
} data_list;

data_list volatile *volatile data_first;
unsigned int volatile to_del;
unsigned char volatile clean_done;
unsigned int volatile deleted;
char volatile in_work = 1;

int buf_len(data_list volatile *data) {
  int count = 0;
  while (data) {
    count++;
    data = data->next;
  }
  return count;
}

int data_reader(void *prm) {
  read_params *params = prm;
  ssize_t read_size;
  data_list volatile *data_new = NULL;
  data_list volatile *data_free = NULL;
  unsigned int i;
  const unsigned int data_buf_size = getpagesize();
  while (params->bytes_left) {
    if (buf_len(data_first) > 1000) {
      if (to_del > 100 && !clean_done) {
        for (i = to_del, deleted = 0; i; deleted++, i--) {
          data_free = data_first;
          data_first = data_first->next;
          free((char *)data_free->buf);
          free((data_list *)data_free);
        }
        clean_done = 1;
      } else
        usleep(65000);
      continue;
    }
    if (!data_new) {
      data_first = malloc(sizeof(data_list));
      data_new = data_first;
    } else {
      data_new->next = malloc(sizeof(data_list));
      data_new = data_new->next;
    }
    data_new->next = NULL;
    data_new->buf = malloc(data_buf_size);
    data_new->data_size = 0;
    while (data_new->data_size != data_buf_size && params->bytes_left != 0) {
      read_size =
          read(params->sock, (char *)data_new->buf + data_new->data_size,
               data_buf_size - data_new->data_size);
      if (read_size < 0)
        kill(getpid(), SIGTERM);
      data_new->data_size += read_size;
      params->bytes_left -= read_size;
    }
  }
  in_work = 0;
  return 0;
}

int write_data(int sock) {
  data_list volatile *data_cur;
  ssize_t write_size;
  while (in_work && buf_len(data_first) < 100)
    usleep(100000);
  data_cur = data_first;
  while (data_cur) {
    while (in_work && buf_len(data_cur) < 100)
      usleep(100000);
    write_size = write(sock, (char *)data_cur->buf, data_cur->data_size);
    if (write_size != data_cur->data_size)
      return 1;
    data_cur = data_cur->next;
    to_del++;
    if (clean_done) {
      to_del -= deleted;
      clean_done = 0;
    }
  }
  return 0;
}

ssize_t read_aswer(int sock, char *hdr, read_params *params) {
  ssize_t read_size = 0;
  char *end;
  char *length;
  while (read(sock, hdr + read_size, 1) == 1) {
    read_size++;
    hdr[read_size] = '\0';
    if (read_size > 3 && !strcmp(hdr + read_size - 4, "\r\n\r\n"))
      break;
  }
  if (strcmp(hdr + read_size - 4, "\r\n\r\n"))
    return -1;
  length = strstr(hdr, "Content-Length:");
  if (length) {
    length += 15;
    while (*length == ' ')
      length++;
  } else {
    params->bytes_left = 0;
    return read_size;
  }
  if ((end = strchr(length, ' ')) || (end = strchr(length, '\r')) ||
      (end = strchr(length, '\n'))) {
    char end_pv = *end;
    *end = '\0';
    params->bytes_left = strtol(length, NULL, 10);
    *end = end_pv;
  } else
    return -1;
  return read_size;
}

ssize_t get_hdr(char *hdr, char *req, read_params *params) {
  struct sockaddr_in6 addr;
  struct hostent *host = gethostbyname2(store_host, AF_INET6);
  params->sock = socket(PF_INET6, SOCK_STREAM, 0);
  if (!host)
    return -1;
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = *(struct in6_addr *)host->h_addr;
  addr.sin6_flowinfo = 0;
  addr.sin6_scope_id = 2;
  addr.sin6_port = htons(store_port);
  if (0 > connect(params->sock, (struct sockaddr *)&addr, sizeof(addr)))
    return -1;
  strcpy(hdr, "GET ");
  strcat(hdr, req);
  strcat(hdr, " \r\n\r\n");
  if (write(params->sock, hdr, strlen(hdr)) != strlen(hdr))
    return -1;
  return read_aswer(params->sock, hdr, params);
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  ssize_t length;
  char *hdr;
  thrd_t thr;
  read_params params;
  hdr = malloc(getpagesize() * 100);
  if ((length = get_hdr(hdr, prm[2], &params)) < 0)
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(sock, hdr, length);
  if (write_size != strlen(hdr))
    return 1;
  if (params.bytes_left > 0) {
    if (thrd_create(&thr, data_reader, &params) != thrd_success ||
        thrd_detach(thr) != thrd_success)
      return 1;
    return write_data(sock);
  } else
    return 0;
}
