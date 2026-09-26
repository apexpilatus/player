#include <FLAC/metadata.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int send_meta(FLAC__StreamMetadata *tags, char *tag) {
  int i;
  for (i = 0; i < tags->data.vorbis_comment.num_comments; i++)
    if (!strncmp(tag, (char *)tags->data.vorbis_comment.comments[i].entry,
                 strlen(tag))) {
      printf("HTTP/1.1 200 OK\r\n%s%lu\r\n%s\r\n\r\n%s", "Content-Length: ",
             strlen((char *)tags->data.vorbis_comment.comments[i].entry) -
                 strlen(tag),
             "Content-Type: text/plain; charset=utf-8",
             tags->data.vorbis_comment.comments[i].entry + strlen(tag));
      return 0;
    }
  return 1;
}

int main(int prm_n, char *prm[]) {
  char *album = strstr(prm[1], "album=");
  char *file = strstr(prm[1], "file=");
  char *tag = strstr(prm[1], "tag=");
  if (album && file && tag) {
    char *end = strchr(album, '&');
    if (end)
      *end = '\0';
    album += 6;
    end = strchr(tag, '&');
    if (end)
      *end = '\0';
    tag += 4;
    end = strchr(file, '&');
    if (end)
      *end = '\0';
    file += 5;
    if (!chdir(album)) {
      FLAC__StreamMetadata *tags =
          FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
      return !FLAC__metadata_get_tags(file, &tags) || send_meta(tags, tag);
    }
  }
  return 1;
}
