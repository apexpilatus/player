#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct albums_list_t {
  struct albums_list_t *next;
  char *path;
  unsigned long int st_atime_nsec;
} albums_list;

static inline albums_list *get_albums() {
  char *src_path = malloc(getpagesize());
  albums_list *album_first = NULL, *album_tmp = NULL;
  DIR *dp_music;
  struct dirent *src_ep;
  struct stat stat_buf;
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
              if (!album_first) {
                album_tmp = malloc(sizeof(albums_list));
                album_first = album_tmp;
              } else {
                album_tmp->next = malloc(sizeof(albums_list));
                album_tmp = album_tmp->next;
              }
              memset(album_tmp, 0, sizeof(albums_list));
              album_tmp->path =
                  malloc(strlen(src_path) + strlen(albm_ep->d_name) + 2);
              sprintf(album_tmp->path, "%s/%s", src_path, albm_ep->d_name);
              if (stat(album_tmp->path, &stat_buf))
              album_tmp->st_atime_nsec = stat_buf.st_atime_nsec;
            else
            album_tmp->st_atime_nsec = 0;
            }
          closedir(dp_src);
        }
      }
    closedir(dp_music);
  }
  return album_first;
}

static inline void list_albums(char *msg) {
  albums_list *albums = get_albums();
  while (albums) {
    strcat(msg, "<img src=\"");
    strcat(msg, albums->path);
    strcat(msg, "\" onclick=gettracks(\"");
    strcat(msg, albums->path);
    strcat(msg, "\") alt=picture>");
    albums = albums->next;
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
  strcat(msg, "<title>albums</title>");
  strcat(msg, "<link rel=stylesheet href=style_albums.css>");
  strcat(msg, "<script src=script_albums.js></script>");
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
