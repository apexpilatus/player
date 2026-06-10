#include <FLAC/metadata.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char *range;

int err(void) {
  printf("%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 404 shit happens",
         "Cache-control: no-cache", "X-Content-Type-Options: nosniff");
  return 1;
}

int send_track(char *file_name) {
  long min_range = 0;
  long max_range;
  struct stat stat_buf;
  if (!stat(file_name, &stat_buf)) {
    char *data = malloc(stat_buf.st_size);
    int fd;
    ssize_t read_size;
    if ((fd = open(file_name, O_RDONLY)) >= 0) {
      char *end;
      read_size = read(fd, data, stat_buf.st_size);
      close(fd);
      if ((end = strchr(range, '-')) && strlen(++end) > 0)
        max_range = strtol(end, NULL, 10);
      else {
        max_range = read_size - 1;
      }
      if ((end = strchr(range, '-'))) {
        *end = '\0';
        min_range = strtol(range, NULL, 10);
      }
      if (read_size == stat_buf.st_size) {
        long content_length = max_range - min_range + 1;
        printf("%s\r\n%s%ld\r\n%s%ld-%ld/%ld\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
               "Content-Length: ", content_length, "Content-Range: bytes ",
               min_range, max_range, read_size, "Content-Type: audio/flac");
        fwrite(data + min_range, content_length, 1, stdout);
        return 0;
      }
    }
  }
  return err();
}

int get_track(long track) {
  FLAC__StreamMetadata *tags =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
  DIR *dp;
  struct dirent *ep;
  dp = opendir(".");
  if (dp) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG && FLAC__metadata_get_tags(ep->d_name, &tags)) {
        int i;
        for (i = 0; i < tags->data.vorbis_comment.num_comments; i++)
          if (!strncmp("TRACKNUMBER=",
                       (char *)tags->data.vorbis_comment.comments[i].entry,
                       strlen("TRACKNUMBER="))) {
            if (strtol((char *)(tags->data.vorbis_comment.comments[i].entry +
                                strlen("TRACKNUMBER=")),
                       NULL, 10) == track)
              return send_track(ep->d_name);
          }
      }
    closedir(dp);
  }
  return err();
}

int main(int prm_n, char *prm[]) {
  char *album = strstr(prm[1], "album=");
  char *track = strstr(prm[1], "track=");
  if (album) {
    char *end = strchr(album, '&');
    if (end)
      *end = '\0';
    album += 6;
  }
  if (track) {
    char *end = strchr(track, '&');
    if (end)
      *end = '\0';
    track += 6;
  }
  if (!(chdir(album) || prm_n < 3)) {
    range = prm[2];
    return get_track(track ? strtol(track, NULL, 10) : 1);
  }
  return err();
}
