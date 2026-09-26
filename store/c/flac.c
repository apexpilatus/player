#include <FLAC/metadata.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int err(void) {
  printf("%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 404 shit happens",
         "Cache-control: no-cache", "X-Content-Type-Options: nosniff");
  return 1;
}

int send_track(char *file_name, char *range) {
  long min_range;
  long max_range;
  struct stat stat_buf;
  if (!stat(file_name, &stat_buf)) {
    int fd;
    if ((fd = open(file_name, O_RDONLY)) >= 0) {
      char *end;
      void *shd_addr =
          mmap(NULL, stat_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
      close(fd);
      if ((end = strchr(range, '-')) && strlen(++end) > 0)
        max_range = strtol(end, NULL, 10);
      else
        max_range = stat_buf.st_size - 1;
      if ((end = strchr(range, '-'))) {
        *end = '\0';
        min_range = strtol(range, NULL, 10);
      } else
        min_range = 0;
      if (shd_addr != MAP_FAILED) {
        long content_length = max_range - min_range + 1;
        printf("%s\r\n%s%ld\r\n%s%ld-%ld/%ld\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
               "Content-Length: ", content_length, "Content-Range: bytes ",
               min_range, max_range, stat_buf.st_size,
               "Content-Type: audio/flac");
        fwrite(shd_addr + min_range, content_length, 1, stdout);
        return 0;
      }
    }
  }
  return err();
}

int main(int prm_n, char *prm[]) {
  char *album = strstr(prm[1], "album=");
  char *file = strstr(prm[1], "file=");
  if (album && file) {
    char *end = strchr(album, '&');
    if (end)
      *end = '\0';
    album += 6;
    end = strchr(file, '&');
    if (end)
      *end = '\0';
    file += 5;
    if (!(chdir(album))) {
      return send_track(file, prm[2]);
    }
  }
  return err();
}
