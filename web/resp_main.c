#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void list_albums(char *msg) {
  char *src_path = malloc(getpagesize());
  DIR *dp_music;
  struct dirent *src_ep;
  dp_music = opendir(music);
  if (dp_music) {
    while ((src_ep = readdir(dp_music)))
      if (src_ep->d_type == DT_DIR && strcmp(src_ep->d_name, ".") &&
          strcmp(src_ep->d_name, "..") &&
          strcmp(src_ep->d_name, "lost+found")) {
        DIR *dp_src;
        struct dirent *albm_ep;
        sprintf(src_path, "%s/%s", music, src_ep->d_name);
        dp_src = opendir(src_path);
        if (dp_src) {
          while ((albm_ep = readdir(dp_src)))
            if (albm_ep->d_type == DT_DIR && strcmp(albm_ep->d_name, ".") &&
                strcmp(albm_ep->d_name, "..") &&
                strcmp(albm_ep->d_name, "lost+found")) {
              strcat(msg, "<img src=\"");
              strcat(msg, src_path);
              strcat(msg, "/");
              strcat(msg, albm_ep->d_name);
              strcat(msg, "\" alt=picture>");
            }
          closedir(dp_src);
        }
      }
    closedir(dp_music);
  }
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize(), write_size;
  char *rsp, *msg;
  rsp = malloc(rsp_size);
  msg = malloc(rsp_size * 10000);
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(
      msg,
      "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">");
  strcat(msg, "<title>player</title>");
  strcat(msg, "</head>");
  strcat(msg, "<body>");
  list_albums(msg);
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
