#include <cdda_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline void create_html(char *msg) {
  cdrom_drive *d;
  unsigned long msg_end;
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(
      msg,
      "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">");
  strcat(msg, "<title>cdcontrol</title>");
  strcat(msg, "<link rel=stylesheet href=style_cd_control.css>");
  strcat(msg, "<script src=script_cd_control.js></script>");
  strcat(msg, "</head>");
  strcat(msg, "<body>");
  strcat(msg, "<script>showtracks()</script>");
  d = cdda_identify("/dev/sr0", CDDA_MESSAGE_FORGETIT, NULL);
  if (d && !cdda_open(d)) {
    strcat(msg, "<div>");
    for (int i = 1; i <= d->tracks; i++)
      if (cdda_track_audiop(d, i)) {
        strcat(msg, "<p onclick=fetch(\"cdplay?");
        msg_end = strlen(msg);
        sprintf(msg + msg_end, "%d", i);
        strcat(msg, "\")>");
        msg_end = strlen(msg);
        sprintf(msg + msg_end, "%d", i);
        strcat(msg, "</p>");
      }
    strcat(msg, "</div>");
  }
  strcat(msg, "</body>");
  strcat(msg, "</html>");
}

static inline void create_header(char *hdr, unsigned long msg_len) {
  unsigned long hdr_end;
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
  create_html(msg);
  create_header(hdr, strlen(msg));
  write_size = write(sock, hdr, strlen(hdr));
  write_size += write(sock, msg, strlen(msg));
  if (write_size == strlen(hdr) + strlen(msg))
    return 0;
  else
    return 1;
}