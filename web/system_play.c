#include <FLAC/metadata.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// clang-format off
#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>
// clang-format on

typedef struct track_list_t {
  struct track_list_t *next;
  char *file_name;
  char *track_number;
} track_list;

static inline void sort_tracks(track_list *track_first) {
  char *file_name_tmp, *track_number_tmp;
  for (track_list *go_slow = track_first; go_slow && go_slow->next;
       go_slow = go_slow->next)
    for (track_list *go_fast = go_slow->next; go_fast; go_fast = go_fast->next)
      if (strtol(go_slow->track_number, NULL, 10) >
          strtol(go_fast->track_number, NULL, 10)) {
        file_name_tmp = go_fast->file_name;
        track_number_tmp = go_fast->track_number;
        go_fast->file_name = go_slow->file_name;
        go_fast->track_number = go_slow->track_number;
        go_slow->file_name = file_name_tmp;
        go_slow->track_number = track_number_tmp;
      }
}

static inline track_list *get_tracks(char *start_track) {
  DIR *dp;
  struct dirent *ep;
  track_list *track_first = NULL, *track_tmp = NULL;
  FLAC__StreamMetadata *tags =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
  dp = opendir(".");
  if (dp) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG && FLAC__metadata_get_tags(ep->d_name, &tags)) {
        for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++)
          if (!strncmp("TRACKNUMBER",
                       (char *)tags->data.vorbis_comment.comments[i].entry,
                       strlen("TRACKNUMBER"))) {
            if (strtol((char *)(tags->data.vorbis_comment.comments[i].entry +
                                strlen("TRACKNUMBER=")),
                       NULL, 10) >= strtol(start_track, NULL, 10)) {
              if (!track_first) {
                track_tmp = malloc(sizeof(track_list));
                track_first = track_tmp;
              } else {
                track_tmp->next = malloc(sizeof(track_list));
                track_tmp = track_tmp->next;
              }
              memset(track_tmp, 0, sizeof(track_list));
              track_tmp->track_number =
                  malloc(tags->data.vorbis_comment.comments[i].length + 1);
              strcpy(track_tmp->track_number,
                     (char *)(tags->data.vorbis_comment.comments[i].entry +
                              strlen("TRACKNUMBER=")));
              track_tmp->file_name = malloc(strlen(ep->d_name) + 1);
              strcpy(track_tmp->file_name, ep->d_name);
            }
            break;
          }
      }
    closedir(dp);
  }
  return track_first;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize(), write_size;
  char *rsp = malloc(rsp_size);
  char *album_dir, *start_track;
  album_dir = strchr(prm[2], '?');
  if (!album_dir)
    execl(resp_err, "resp_err", prm[1], NULL);
  start_track = strchr(++album_dir, '&');
  if (!start_track)
    execl(resp_err, "resp_err", prm[1], NULL);
  *start_track = '\0';
  start_track++;
  if (chdir(album_dir))
    execl(resp_err, "resp_err", prm[1], NULL);
  track_list *tracks = get_tracks(start_track);
  if (!tracks)
    execl(resp_err, "resp_err", prm[1], NULL);
  sort_tracks(tracks);

  strcpy(rsp, "HTTP/1.1 200 OK\r\n\r\n");
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
}
