#include <FLAC/metadata.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static inline int get_picture(FLAC__StreamMetadata **picture) {
  DIR *dp;
  struct dirent *ep;
  dp = opendir(".");
  if (dp) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG) {
        if (FLAC__metadata_get_picture(ep->d_name, picture, -1, NULL, NULL,
                                       (uint32_t)(-1), (uint32_t)(-1),
                                       (uint32_t)(-1), (uint32_t)(-1))) {
          closedir(dp);
          return 0;
        }
      }
    closedir(dp);
  }
  return 1;
}

int main(int prm_n, char *prm[]) {
  if (chdir(prm[2]))
    execl(resp_err, "resp_err", prm[1], NULL);
  FLAC__StreamMetadata *picture =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
  if (get_picture(&picture))
    execl(resp_err, "resp_err", prm[1], NULL);
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize(), write_size;
  char *rsp = malloc(rsp_size);
  sprintf(rsp, "%s\r\n%s%u\r\n%s\r\n%s%s\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
          "Content-Length: ", picture->data.picture.data_length,
          "Cache-control: max-age=31536000, immutable",
          "Content-Type: ", picture->data.picture.mime_type,
          "X-Content-Type-Options: nosniff");
  write_size = write(sock, rsp, strlen(rsp));
  write_size += write(sock, picture->data.picture.data,
                      picture->data.picture.data_length);
  if (write_size == strlen(rsp) + picture->data.picture.data_length)
    return 0;
  else
    return 1;
}
