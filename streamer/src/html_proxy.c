#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

ssize_t read_aswer(int sock, char *msg) {
  ssize_t read_size = 0;
  ssize_t data_size = 0;
  char *end;
  char *length;
  long bytes_left;
  while (read(sock, msg + read_size, 1) == 1) {
    read_size++;
    msg[read_size] = '\0';
    if (read_size > 3 && !strcmp(msg + read_size - 4, "\r\n\r\n"))
      break;
  }
  if (strcmp(msg + read_size - 4, "\r\n\r\n"))
    return -1;
  length = strstr(msg, "Content-Length:");
  if (length) {
    length += 15;
    while (*length == ' ')
      length++;
  } else
    return read_size;
  if ((end = strchr(length, ' ')) || (end = strchr(length, '\r')) ||
      (end = strchr(length, '\n'))) {
    char end_pv = *end;
    *end = '\0';
    bytes_left = strtol(length, NULL, 10);
    *end = end_pv;
  } else
    return -1;
  while ((data_size = read(sock, msg + read_size, bytes_left)) < bytes_left) {
    if (data_size < 0)
      return -1;
    bytes_left -= data_size;
    read_size += data_size;
  }
  read_size += data_size;
  return read_size;
}

ssize_t get_html(char *msg, char *req) {
  int sock = socket(PF_INET6, SOCK_STREAM, 0);
  struct sockaddr_in6 addr;
  struct hostent *host = gethostbyname2(store_host, AF_INET6);
  if (!host)
    return -1;
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = *(struct in6_addr *)host->h_addr;
  addr.sin6_flowinfo = 0;
  addr.sin6_scope_id = 2;
  addr.sin6_port = htons(store_port);
  if (0 > connect(sock, (struct sockaddr *)&addr, sizeof(addr)))
    return -1;
  strcpy(msg, "GET ");
  strcat(msg, req);
  strcat(msg, " \r\n\r\n");
  if (write(sock, msg, strlen(msg)) != strlen(msg))
    return -1;
  return read_aswer(sock, msg);
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  ssize_t length;
  char *msg;
  msg = malloc(getpagesize() * 11000);
  if ((length = get_html(msg, prm[2])) < 0)
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(sock, msg, length);
  if (write_size == strlen(msg))
    return 0;
  else
    return 1;
}
