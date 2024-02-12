#include <FLAC/metadata.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct meta_list_t {
  struct meta_list_t *next;
  char *artist;
  char *album;
  char *title;
  char *track;
  char *rate;
} meta_list;

static inline void sort_tags(meta_list *list_first) {
  for (meta_list *go_slow = list_first; go_slow && go_slow->next;
       go_slow = go_slow->next)
    for (meta_list *go_fast = go_slow->next; go_fast; go_fast = go_fast->next)
      printf("go_slow - %s;go_fast - %s\n", go_slow->track, go_fast->track);
}

static inline void cpy_tags(meta_list *list, FLAC__StreamMetadata *tags) {
  for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++) {
    if (!strncmp("ARTIST", (char *)tags->data.vorbis_comment.comments[i].entry,
                 strlen("ARTIST"))) {
      list->artist = malloc(tags->data.vorbis_comment.comments[i].length + 1);
      strcpy(list->artist,
             (char *)(tags->data.vorbis_comment.comments[i].entry +
                      strlen("ARTIST=")));
    } else if (!strncmp("ALBUM",
                        (char *)tags->data.vorbis_comment.comments[i].entry,
                        strlen("ALBUM"))) {
      list->album = malloc(tags->data.vorbis_comment.comments[i].length + 1);
      strcpy(list->album, (char *)(tags->data.vorbis_comment.comments[i].entry +
                                   strlen("ALBUM=")));
    } else if (!strncmp("TITLE",
                        (char *)tags->data.vorbis_comment.comments[i].entry,
                        strlen("TITLE"))) {
      list->title = malloc(tags->data.vorbis_comment.comments[i].length + 1);
      strcpy(list->title, (char *)(tags->data.vorbis_comment.comments[i].entry +
                                   strlen("TITLE=")));
    } else if (!strncmp("TRACKNUMBER",
                        (char *)tags->data.vorbis_comment.comments[i].entry,
                        strlen("TRACKNUMBER"))) {
      list->track = malloc(tags->data.vorbis_comment.comments[i].length + 1);
      strcpy(list->track, (char *)(tags->data.vorbis_comment.comments[i].entry +
                                   strlen("TRACKNUMBER=")));
    }
  }
}

static inline void list_tracks(char *msg) {
  DIR *dp;
  struct dirent *ep;
  meta_list *list_first, *list_tmp;
  FLAC__StreamMetadata *tags =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
  FLAC__StreamMetadata *rate =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
  dp = opendir(".");
  if (dp) {
    strcat(msg, "<div class=title>");
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG) {
        if (FLAC__metadata_get_tags(ep->d_name, &tags)) {
          list_first = malloc(sizeof(meta_list));
          list_tmp = list_first;
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
          if (FLAC__metadata_get_streaminfo(ep->d_name, rate)) {
            list_first->rate = malloc(30);
            sprintf(list_first->rate, "%u/%g",
                    rate->data.stream_info.bits_per_sample,
                    rate->data.stream_info.sample_rate / 1000.0);
            strcat(msg, "<div class=rate>");
            strcat(msg, list_first->rate);
            strcat(msg, "</div>");
          }
        }
        break;
      }
    strcat(msg, "</div>");
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG)
        if (FLAC__metadata_get_tags(ep->d_name, &tags)) {
          list_tmp->next = malloc(sizeof(meta_list));
          list_tmp = list_tmp->next;
          memset(list_tmp, 0, sizeof(meta_list));
          cpy_tags(list_tmp, tags);
        }
    strcat(msg, "<table>");
    sort_tags(list_first);
    list_tmp = list_first;
    while (list_tmp) {
      strcat(msg, "<tr>");
      strcat(msg, "<td class=tracknumber>");
      if (list_tmp->track)
        strcat(msg, list_tmp->track);
      strcat(msg, "</td>");
      strcat(msg, "<td class=tracktitle>");
      if (list_tmp->title)
        strcat(msg, list_tmp->title);
      strcat(msg, "</td>");
      strcat(msg, "</tr>");
      list_tmp = list_tmp->next;
    }
    strcat(msg, "</table>");
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
  strcat(
      msg,
      "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">");
  strcat(msg, "<title>tracks</title>");
  strcat(msg, "<link rel=stylesheet href=style_tracks.css>");
  strcat(msg, "<script src=script_tracks.js></script>");
  strcat(msg, "</head>");
  strcat(msg, "<body>");
  list_tracks(msg);
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
