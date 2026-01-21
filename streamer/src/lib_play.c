#include "lib_play.h"
#include <netdb.h>
#include <string.h>
#include <unistd.h>

const unsigned int data_buf_size = 18000;
const unsigned int alsa_buf_size = data_buf_size * 6;

int send_request(int sock, char *prm[]) {
  char msg[getpagesize()];
  struct sockaddr_in6 addr;
  struct hostent *host;
  addr.sin6_family = AF_INET6;
  if (!(host = gethostbyname2(store_host, AF_INET6)))
    return 1;
  addr.sin6_addr = *(struct in6_addr *)host->h_addr;
  addr.sin6_flowinfo = 0;
  addr.sin6_scope_id = 2;
  addr.sin6_port = htons(store_port);
  if (0 > connect(sock, (struct sockaddr *)&addr, sizeof(addr)))
    return 1;
  strcpy(msg, "GET ");
  strcat(msg, prm[2]);
  strcat(msg, " \r\n\r\n");
  if (write(sock, msg, strlen(msg)) != strlen(msg))
    return 1;
  return 0;
}

int buf_len(data_list volatile *data) {
  int count = 0;
  while (data) {
    count++;
    data = data->next;
  }
  return count;
}

int read_headers(int sock, unsigned int *rate, unsigned short *bits_per_sample,
                 unsigned int *bytes_left) {
  int read_size = 0;
  int msg_size = getpagesize() * 100;
  char msg[msg_size];
  while (read_size < msg_size && read(sock, msg + read_size, 1) == 1) {
    read_size++;
    if (read_size < msg_size)
      msg[read_size] = '\0';
    if (read_size > 3 && !strcmp(msg + read_size - 4, "\r\n\r\n")) {
      break;
    }
  }
  if (read_size == msg_size ||
      strncmp(msg, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK")))
    return 1;
  for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
    read_size++;
    if (read_size == 28)
      break;
  }
  if (read_size < 28)
    return 1;
  *rate = *((int *)(msg + 24));
  for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
    read_size++;
    if (read_size == 8)
      break;
  }
  if (read_size < 8)
    return 1;
  *bits_per_sample = *((short *)(msg + 6));
  if (*bits_per_sample == 16) {
    for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
      read_size++;
      if (read_size == 8)
        break;
    }
    if (read_size < 8)
      return 1;
    *bytes_left = *((unsigned int *)(msg + 4));
  } else {
    for (read_size = 0; read(sock, msg + read_size, 1) == 1;) {
      read_size++;
      if (read_size == 32)
        break;
    }
    if (read_size < 32)
      return 1;
    *bytes_left = *((unsigned int *)(msg + 28));
  }
  return 0;
}
