#include <FLAC/metadata.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct tracks_list_t {
  struct tracks_list_t *nx;
  struct tracks_list_t *pv;
  char *file;
  long number;
} tracks_list;

tracks_list *get_tracks() {
  tracks_list *tracks = NULL;
  tracks_list *tracks_last = NULL;
  FLAC__StreamMetadata *tags =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
  DIR *dp;
  struct dirent *ep;
  int i;
  dp = opendir(".");
  if (dp) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG && FLAC__metadata_get_tags(ep->d_name, &tags))
        for (i = 0; i < tags->data.vorbis_comment.num_comments; i++)
          if (!strncmp("TRACKNUMBER=",
                       (char *)tags->data.vorbis_comment.comments[i].entry,
                       strlen("TRACKNUMBER="))) {
            tracks_list *tracks_next = malloc(sizeof(tracks_list));
            tracks_next->file = malloc(strlen(ep->d_name) + 1);
            strcpy(tracks_next->file, ep->d_name);
            tracks_next->nx = NULL;
            tracks_next->number =
                strtol((char *)(tags->data.vorbis_comment.comments[i].entry +
                                strlen("TRACKNUMBER=")),
                       NULL, 10);
            if (tracks) {
              tracks_last->nx = tracks_next;
              tracks_next->pv = tracks_last;
              tracks_last = tracks_next;
            } else {
              tracks_next->pv = NULL;
              tracks = tracks_next;
              tracks_last = tracks;
            }
          }
    closedir(dp);
  }
  return tracks;
}

tracks_list *sort_tracks(tracks_list *tracks) {
  tracks_list *first = tracks;
  tracks_list *slow = first->nx;
  while (slow) {
    tracks_list *fast = first;
    while (fast != slow) {
      if (fast->number > slow->number) {
        tracks_list *tmp = slow->pv;
        slow->pv->nx = slow->nx;
        if (slow->nx)
          slow->nx->pv = slow->pv;
        if (fast->pv)
          fast->pv->nx = slow;
        else
          first = slow;
        slow->pv = fast->pv;
        fast->pv = slow;
        slow->nx = fast;
        slow = tmp;
	break;
      }
      fast = fast->nx;
    }
    slow = slow->nx;
  }
  return first;
}

int send_tracks(tracks_list *tracks) {
  char data[4096];
  strcpy(data, tracks->file);
  while (tracks->nx) {
    strcat(data, "\r\n");
    tracks = tracks->nx;
    strcat(data, tracks->file);
  }
  if (strlen(data) < 4096) {
    printf("HTTP/1.1 200 OK\r\n%s%lu\r\n%s\r\n\r\n%s",
           "Content-Length: ", strlen(data),
           "Content-Type: text/plain; charset=utf-8", data);
    return 0;
  }
  return 1;
}

int main(int prm_n, char *prm[]) {
  char *album = strstr(prm[1], "album=");
  if (album) {
    tracks_list *tracks = NULL;
    char *end = strchr(album, '&');
    if (end)
      *end = '\0';
    album += 6;
    if (!chdir(album) && (tracks = get_tracks())) {
      return send_tracks(sort_tracks(tracks));
    }
  }
  return 1;
}
