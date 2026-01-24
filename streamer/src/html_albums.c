#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int list_albums(char *msg) { return 1; }

int create_html(char *msg) {
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(
      msg,
      "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">");
  strcat(msg, "<title>albums</title>");
  strcat(msg, "</head>");
  strcat(msg, "<body>");
  if (list_albums(msg))
    return 1;
  strcat(msg, "</body>");
  strcat(msg, "</html>");
  return 0;
}

void create_header(char *hdr, unsigned long msg_len) {
  size_t hdr_end;
  strcpy(hdr, "HTTP/1.1 200 OK\r\n");
  strcat(hdr, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(hdr, "Cache-control: no-cache\r\n");
  strcat(hdr, "X-Content-Type-Options: nosniff\r\n");
  hdr_end = strlen(hdr);
  sprintf(hdr + hdr_end, "Content-Length: %lu\r\n\r\n", msg_len);
}

int main(int prm_n, char *prm[]) {
  int sock;
  ssize_t write_size;
  char *hdr;
  char *msg;
  sock = strtol(prm[1], NULL, 10);
  hdr = malloc(getpagesize());
  msg = malloc(getpagesize() * 10000);
  if (create_html(msg))
    execl(resp_err, "resp_err", prm[1], NULL);
  create_header(hdr, strlen(msg));
  write_size = write(sock, hdr, strlen(hdr));
  write_size += write(sock, msg, strlen(msg));
  if (write_size == strlen(hdr) + strlen(msg))
    return 0;
  else
    return 1;
}
