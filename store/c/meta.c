#include <FLAC/metadata.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int get_tags(FLAC__StreamMetadata **tags, long track) {
  DIR *dp;
  struct dirent *ep;
  dp = opendir(".");
  if (dp) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG && FLAC__metadata_get_tags(ep->d_name, tags)) {
        int i;
        for (i = 0; i < (*tags)->data.vorbis_comment.num_comments; i++)
          if (!strncmp("TRACKNUMBER=",
                       (char *)(*tags)->data.vorbis_comment.comments[i].entry,
                       strlen("TRACKNUMBER="))) {
            if (strtol((char *)((*tags)->data.vorbis_comment.comments[i].entry +
                                strlen("TRACKNUMBER=")),
                       NULL, 10) == track)
              return 0;
          }
      }
    closedir(dp);
  }
  return 1;
}

int send_meta(FLAC__StreamMetadata *tags, char *meta) {
  int i;
  for (i = 0; i < tags->data.vorbis_comment.num_comments; i++)
    if (!strncmp(meta, (char *)tags->data.vorbis_comment.comments[i].entry,
                 strlen(meta))) {
      printf("HTTP/1.1 200 OK\r\n%s%lu\r\n%s\r\n\r\n%s", "Content-Length: ",
             strlen((char *)tags->data.vorbis_comment.comments[i].entry) -
                 strlen(meta),
             "Content-Type: text/plain; charset=utf-8",
             tags->data.vorbis_comment.comments[i].entry + strlen(meta));
      return 0;
    }
  return 1;
}

int main(int prm_n, char *prm[]) {
  char *album = strstr(prm[1], "album=");
  char *track = strstr(prm[1], "track=");
  char *meta = strstr(prm[1], "meta=");
  if (album && meta) {
    char *end = strchr(album, '&');
    if (end)
      *end = '\0';
    album += 6;
    end = strchr(meta, '&');
    if (end)
      *end = '\0';
    meta += 5;
    if (track) {
      end = strchr(track, '&');
      if (end)
        *end = '\0';
      track += 6;
    }
    if (!chdir(album)) {
      FLAC__StreamMetadata *tags =
          FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
      return get_tags(&tags, track ? strtol(track, NULL, 10) : 1) ||
             send_meta(tags, meta);
    }
  }
  return 1;
}