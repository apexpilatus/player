#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock;
  ssize_t rsp_size, write_size;
  char *rsp, *msg, *album;
  album = strchr(prm[2], '?');
  if (!album || chdir(++album))
    execl(resp_err, "resp_err", prm[1], NULL);
  sock = strtol(prm[1], NULL, 10);
  rsp_size = getpagesize();
  rsp = malloc(rsp_size);
  msg = malloc(rsp_size * 10000);
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(msg, "<link rel=stylesheet href=style_tracks.css>");
  strcat(msg, "<script src=script_tracks.js></script>");
  strcat(msg, "</head>");
  strcat(msg, "<body>");
  strcat(msg, "<script>showtracks()</script>");
  strcat(msg, "<p>");
  strcat(msg, album);
  strcat(msg, "</p>");
  strcat(msg, "</body>");
  strcat(msg, "</html>");
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  write_size = strlen(rsp);
  sprintf(rsp + write_size, "Content-Length: %lu\r\n\r\n", strlen(msg));
  write_size = write(sock, rsp, strlen(rsp));
  write_size += write(sock, msg, strlen(msg));
  if (write_size == strlen(rsp) + strlen(msg))
    return 0;
  else
    return 1;
}
