#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef struct albums_list_t {
  struct albums_list_t *next;
  char *path;
  time_t mtime;
} albums_list;

static void sort_albums(albums_list *album_first) {
  char *path_tmp;
  time_t mtime_tmp;
  for (albums_list *go_slow = album_first; go_slow && go_slow->next;
       go_slow = go_slow->next)
    for (albums_list *go_fast = go_slow->next; go_fast;
         go_fast = go_fast->next) {
      if (difftime(go_fast->mtime, go_slow->mtime) > 0) {
        path_tmp = go_fast->path;
        mtime_tmp = go_fast->mtime;
        go_fast->path = go_slow->path;
        go_fast->mtime = go_slow->mtime;
        go_slow->path = path_tmp;
        go_slow->mtime = mtime_tmp;
      }
    }
}

static albums_list *get_albums() {
  char *src_path = malloc(getpagesize());
  albums_list *album_first = NULL;
  albums_list *album_tmp = NULL;
  DIR *dp_music;
  struct dirent *src_ep;
  struct stat stat_buf;
  dp_music = opendir(music_path);
  if (dp_music) {
    while ((src_ep = readdir(dp_music)))
      if (src_ep->d_type == DT_DIR && strcmp(src_ep->d_name, ".") &&
          strcmp(src_ep->d_name, "..") &&
          strcmp(src_ep->d_name, "lost+found")) {
        DIR *dp_src;
        struct dirent *albm_ep;
        sprintf(src_path, "%s/%s", music_path, src_ep->d_name);
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
              if (!stat(album_tmp->path, &stat_buf))
                album_tmp->mtime = stat_buf.st_mtime;
            }
          closedir(dp_src);
        }
      }
    closedir(dp_music);
  }
  sort_albums(album_first);
  return album_first;
}

static void list_albums(char *msg) {
  albums_list *albums = get_albums();
  if (albums && difftime(time(NULL), albums->mtime) >= 0) {
    strcat(msg, "<script>updatetop(\"");
    strcat(msg, albums->path);
    strcat(msg, "\")</script>");
    while (albums) {
      strcat(msg, "<img src=\"");
      strcat(msg, albums->path);
      strcat(msg, "\" onclick=gettracks(\"");
      strcat(msg, albums->path);
      strcat(msg, "\") alt=picture>");
      albums = albums->next;
    }
  }
}

static void create_html(char *msg, char *bottom) {
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(msg, "<meta name=viewport content=\"width=device-width, "
              "initial-scale=1.0\">");
  strcat(msg, "<title>albums</title>");
  strcat(msg, "<link rel=stylesheet href=style_albums.css>");
  strcat(msg, "<script src=script_albums.js></script>");
  strcat(msg, "</head>");
  strcat(msg, bottom ? "<body onload=showscroll(\"down\")>"
                     : "<body onload=showscroll(\"up\")>");
  list_albums(msg);
  strcat(msg, "</body>");
  strcat(msg, "</html>");
}

static void create_header(char *hdr, unsigned long msg_len) {
  size_t hdr_end;
  strcpy(hdr, "HTTP/1.1 200 OK\r\n");
  strcat(hdr, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(hdr, "Cache-control: no-cache\r\n");
  strcat(hdr, "X-Content-Type-Options: nosniff\r\n");
  hdr_end = strlen(hdr);
  sprintf(hdr + hdr_end, "Content-Length: %lu\r\n\r\n", msg_len);
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char *hdr;
  char *msg;
  hdr = malloc(getpagesize());
  msg = malloc(getpagesize() * 10000);
  create_html(msg, strchr(prm[2], '?'));
  create_header(hdr, strlen(msg));
  write_size = write(sock, hdr, strlen(hdr));
  write_size += write(sock, msg, strlen(msg));
  if (write_size == strlen(hdr) + strlen(msg))
    return 0;
  else
    return 1;
}
