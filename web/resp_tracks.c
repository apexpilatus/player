#include <FLAC/metadata.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct meta_list_t {
  struct meta_list *next;
  char *artist;
  char *album;
  char *title;
  char *track;
  char *rate;
} meta_list;

void cpy_tags(meta_list *list, FLAC__StreamMetadata *tags) {
  for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++) {
    if (!strncmp("ARTIST", (char *)tags->data.vorbis_comment.comments[i].entry,
                 strlen("ARTIST"))) {
      list->artist = malloc(tags->data.vorbis_comment.comments[i].length);
      strcpy(list->artist,
             (char *)(tags->data.vorbis_comment.comments[i].entry +
                      strlen("ARTIST=")));
    } else if (!strncmp("ALBUM",
                        (char *)tags->data.vorbis_comment.comments[i].entry,
                        strlen("ALBUM"))) {
      list->album = malloc(tags->data.vorbis_comment.comments[i].length);
      strcpy(list->album, (char *)(tags->data.vorbis_comment.comments[i].entry +
                                   strlen("ALBUM=")));
    }
  }
}

void list_albums(char *msg) {
  DIR *dp;
  struct dirent *ep;
  meta_list *list_first;
  FLAC__StreamMetadata *tags =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
  dp = opendir(".");
  if (dp) {
    strcat(msg, "<div class=title>");
    for (ep = readdir(dp); ep->d_type != DT_REG; ep = readdir(dp))
      ;
    if (FLAC__metadata_get_tags(ep->d_name, &tags)) {
      list_first = malloc(sizeof(meta_list));
      memset(list_first, 0, sizeof(meta_list));
      cpy_tags(list_first, tags);
      if (list_first->artist) {
        strcat(msg, "<div class=artist>");
        strcat(msg, list_first->artist);
        strcat(msg, "</div>");
      }
      if (list_first->album) {
        strcat(msg, "<div class=album>");
        strcat(msg, list_first->album);
        strcat(msg, "</div>");
      }
    }
    strcat(msg, "</div>");
    closedir(dp);
  }
}

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
  strcat(msg, "<button type=button id=poweroff onclick=fetch(\"poweroff\")>"
              "&#9940;</button>");
  strcat(msg, "<button type=button id=hidetracks onclick=hidetracks()>"
              "&#9932</button>");
  strcat(msg, "<button type=button id=volume onclick=hidetracks()>"
              "&#9738</button>");
  list_albums(msg);
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
