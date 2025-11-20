#include <FLAC/metadata.h>
#include <cdda_interface.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct meta_list_t {
  struct meta_list_t *next;
  FLAC__uint64 size;
  char *artist;
  char *album;
  char *title;
  char *track;
  char *rate;
} meta_list;

void sort_tracks(meta_list *list_first) {
  char *title_tmp;
  char *track_tmp;
  for (meta_list *go_slow = list_first; go_slow && go_slow->next;
       go_slow = go_slow->next)
    for (meta_list *go_fast = go_slow->next; go_fast; go_fast = go_fast->next)
      if (go_slow->track && go_fast->track &&
          strtol(go_slow->track, NULL, 10) > strtol(go_fast->track, NULL, 10)) {
        title_tmp = go_fast->title;
        track_tmp = go_fast->track;
        go_fast->title = go_slow->title;
        go_fast->track = go_slow->track;
        go_slow->title = title_tmp;
        go_slow->track = track_tmp;
      }
}

void cpy_tags(meta_list *list, FLAC__StreamMetadata *tags) {
  for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++)
    if (!strncmp("ARTIST=", (char *)tags->data.vorbis_comment.comments[i].entry,
                 strlen("ARTIST="))) {
      list->artist = malloc(tags->data.vorbis_comment.comments[i].length + 1);
      strcpy(list->artist,
             (char *)(tags->data.vorbis_comment.comments[i].entry +
                      strlen("ARTIST=")));
    } else if (!strncmp("ALBUM=",
                        (char *)tags->data.vorbis_comment.comments[i].entry,
                        strlen("ALBUM="))) {
      list->album = malloc(tags->data.vorbis_comment.comments[i].length + 1);
      strcpy(list->album, (char *)(tags->data.vorbis_comment.comments[i].entry +
                                   strlen("ALBUM=")));
    } else if (!strncmp("TITLE=",
                        (char *)tags->data.vorbis_comment.comments[i].entry,
                        strlen("TITLE="))) {
      list->title = malloc(tags->data.vorbis_comment.comments[i].length + 1);
      strcpy(list->title, (char *)(tags->data.vorbis_comment.comments[i].entry +
                                   strlen("TITLE=")));
    } else if (!strncmp("TRACKNUMBER=",
                        (char *)tags->data.vorbis_comment.comments[i].entry,
                        strlen("TRACKNUMBER="))) {
      list->track = malloc(tags->data.vorbis_comment.comments[i].length + 1);
      strcpy(list->track, (char *)(tags->data.vorbis_comment.comments[i].entry +
                                   strlen("TRACKNUMBER=")));
    }
}

void list_tracks(char *msg) {
  DIR *dp;
  struct dirent *ep;
  meta_list *list_first = NULL;
  meta_list *list_tmp = NULL;
  FLAC__StreamMetadata *tags =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
  FLAC__StreamMetadata *rate =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
  char *album_dir = getcwd(NULL, 0);
  dp = opendir(".");
  if (dp) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG && FLAC__metadata_get_tags(ep->d_name, &tags)) {
        list_first = malloc(sizeof(meta_list));
        memset(list_first, 0, sizeof(meta_list));
        cpy_tags(list_first, tags);
        if (FLAC__metadata_get_streaminfo(ep->d_name, rate)) {
          list_first->rate = malloc(30);
          sprintf(list_first->rate, "%u/%g",
                  rate->data.stream_info.bits_per_sample,
                  rate->data.stream_info.sample_rate / 1000.0);
          list_first->size = rate->data.stream_info.total_samples * 2 *
                             (rate->data.stream_info.bits_per_sample / 8);
        }
        break;
      }
    list_tmp = list_first;
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG && FLAC__metadata_get_tags(ep->d_name, &tags)) {
        list_tmp->next = malloc(sizeof(meta_list));
        memset(list_tmp->next, 0, sizeof(meta_list));
        list_tmp = list_tmp->next;
        cpy_tags(list_tmp, tags);
        if (FLAC__metadata_get_streaminfo(ep->d_name, rate)) {
          list_tmp->size = rate->data.stream_info.total_samples * 2 *
                           (rate->data.stream_info.bits_per_sample / 8);
        }
      }
    closedir(dp);
  }
  strcat(msg, "<div id=albumtitle>");
  if (list_first)
    if (list_first->artist) {
      strcat(msg, "<div id=artist>");
      strcat(msg, list_first->artist);
      strcat(msg, "</div>");
      if (list_first->album && strcmp(list_first->artist, list_first->album)) {
        strcat(msg, "<div id=album>");
        strcat(msg, list_first->album);
        strcat(msg, "</div>");
      }
      if (list_first->rate) {
        strcat(msg, "<div id=rate>");
        strcat(msg, list_first->rate);
        strcat(msg, "</div>");
      }
    }
  strcat(msg, "</div>");
  sort_tracks(list_first);
  strcat(msg, "<table>");
  list_tmp = list_first;
  while (list_tmp) {
    strcat(msg, "<tr onclick=playflac(\"");
    strcat(msg, album_dir);
    if (list_tmp->track) {
      strcat(msg, "&");
      strcat(msg, list_tmp->track);
    }
    strcat(msg, "\")>");
    strcat(msg, "<td class=tracknumber>");
    if (list_tmp->track) {
      if (strlen(list_tmp->track) == 1)
        strcat(msg, "&nbsp;&nbsp;");
      strcat(msg, list_tmp->track);
    } else
      strcat(msg, "!!!!!!!! NO TRACK !!!!!!!!!");
    strcat(msg, "</td>");
    strcat(msg, "<td class=tracktitle>");
    if (list_tmp->title)
      strcat(msg, list_tmp->title);
    strcat(msg, "</td>");
    strcat(msg, "</tr>");
    list_tmp = list_tmp->next;
  }
  strcat(msg, "</table>");
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char *url = prm[2];
  ssize_t write_size;
  char hdr[getpagesize()];
  char msg[getpagesize() * 1000];
  cdrom_drive *d = cdda_identify("/dev/sr0", CDDA_MESSAGE_FORGETIT, NULL);
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(
      msg,
      "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">");
  strcat(msg, "<title>player</title>");
  strcat(msg, "<link rel=stylesheet href=style_main.css>");
  if (!strcmp("/", url)) {
    strcat(msg, "<link id=icon rel=icon href=apple-touch-icon.png>");
    strcat(msg, "</head>");
    strcat(msg, "<body>");
    strcat(msg, "<iframe id=albums title=albums src=/albums></iframe>");
  } else {
    char *album = strstr(url, "album=");
    char *scroll = strstr(url, "scroll=");
    if (album) {
      char *end = strchr(album, '&');
      if (end)
        *end = '\0';
      strcat(msg, "<link id=icon rel=icon href=");
      strcat(msg, album + 6);
      strcat(msg, ">");
      strcat(msg, "</head>");
      strcat(msg, "<body>");
      strcat(msg, "<div class=tracks>");
      if (!chdir(album + 6))
        list_tracks(msg);
      strcat(msg, "</div>");
      if (end)
        *end = '&';
    } else {
      strcat(msg, "<link id=icon rel=icon href=apple-touch-icon.png>");
      strcat(msg, "</head>");
      strcat(msg, "<body>");
      if (d && !cdda_open(d)) {
        size_t msg_end;
        strcat(msg, "<div class=cdcontrol>");
        for (int i = 1; i <= d->tracks; i++)
          if (cdda_track_audiop(d, i)) {
            strcat(msg, "<b onclick=playcd(");
            msg_end = strlen(msg);
            sprintf(msg + msg_end, "%d", i);
            strcat(msg, ")>");
            msg_end = strlen(msg);
            sprintf(msg + msg_end, "%d", i);
            strcat(msg, "</b>");
          }
        strcat(msg, "</div>");
      }
    }
    strcat(msg, "<p hidden id=topalbum></p>");
    strcat(msg, "<audio id=player autoplay></audio>");
    if (scroll) {
      char *end = strchr(scroll, '&');
      if (end)
        *end = '\0';
      strcat(msg, "<iframe id=albums title=albums src=/albums?");
      strcat(msg, scroll);
      strcat(msg, "></iframe>");
      if (end)
        *end = '&';
    }
    /*  if (album) {
        if (*(album + 1) != '/')
          strcat(msg,
                 "<link id=icon rel=icon href=apple-touch-icon.png>");
        else {
          char *end = strchr(album, '&');
          if (end)
            *end = '\0';
          strcat(msg, "<link id=icon rel=icon href=");
          strcat(msg, album + 1);
          strcat(msg, ">");
          if (end)
            *end = '&';
        }
      } else
        strcat(msg,
               "<link id=icon rel=icon href=apple-touch-icon.png>");*/
  }
  /*strcat(msg, "</head>");
  strcat(msg, "<body>");
  if (strncmp("/inbrowser", url, strlen("/inbrowser")))
    strcat(msg, "<audio id=player title=manual onended=loaddefault()></audio>");
  else {
    if (album) {
      strcat(
          msg,
          "<audio id=player title=autoplay onended=loaddefault() src=stream_");
      if (*(album + 1) != '/')
        strcat(msg, "cd");
      else
        strcat(msg, "album");
      strcat(msg, album);
      strcat(msg, "></audio>");
    } else
      strcat(msg,
             "<audio id=player title=manual onended=loaddefault()></audio>");
  }
  strcat(msg, "<p hidden id=topalbum></p>");
  strcat(msg, "<p hidden id=selectedalbum></p>");
  strcat(msg, "<p id=position onclick=updateposition()>0</p>");
  strcat(msg, "<iframe id=albums title=albums
  onscroll=updateposition()></iframe>"); strcat(msg, "<iframe id=control
  title=control class="); if (strncmp("/inbrowser", url, strlen("/inbrowser"))
  || (album && *(album + 1) == '/')) strcat(msg, "tracks"); else strcat(msg,
  "cdcontrol src=cdcontrol"); strcat(msg, "></iframe>"); strcat(msg, "<button
  type=button id=poweroff onclick=poweroff()>"
              "&#9635;</button>");
  if (cdda_identify("/dev/sr0", CDDA_MESSAGE_FORGETIT, NULL))
    strcat(msg, "<button type=button id=getcd onclick=getcd()>"
                "&#9673</button>");
  strcat(msg, "<button hidden type=button id=scrollup "
              "onclick=scrollup()>&#9650</button>");
  strcat(msg, "<button hidden type=button id=scrolldown "
              "onclick=scrolldown()>&#9660</button>");
  strcat(msg, "<script src=script_main.js></script>");
  if (strncmp("/inbrowser", url, strlen("/inbrowser")) ||
      (album && *(album + 1) == '/'))
    strcat(msg, "<script>getalbums()</script>");
  else
    strcat(msg, "<script>scrollup()</script>");*/
  if (d)
    strcat(msg, "<button type=button id=getcd onclick=getcd()>&#9673</button>");
  strcat(msg,
         "<button type=button id=poweroff onclick=poweroff()>&#9635;</button>");
  strcat(msg, "<script src=script_main.js></script>");
  strcat(msg, "</body>");
  strcat(msg, "</html>");
  strcpy(hdr, "HTTP/1.1 200 OK\r\n");
  strcat(hdr, "Content-Type: text/html; charset=utf-8\r\n");
  write_size = strlen(hdr);
  sprintf(hdr + write_size, "Content-Length: %lu\r\n\r\n", strlen(msg));
  write_size = write(sock, hdr, strlen(hdr));
  write_size += write(sock, msg, strlen(msg));
  if (write_size == strlen(hdr) + strlen(msg))
    return 0;
  else
    return 1;
}
