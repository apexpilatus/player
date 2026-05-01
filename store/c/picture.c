#include <FLAC/metadata.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  FLAC__StreamMetadata *picture =
      FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
  DIR *dp;
  struct dirent *ep;
  if (prm_n > 1 && !chdir(prm[1]) && (dp = opendir("."))) {
    while ((ep = readdir(dp)))
      if (ep->d_type == DT_REG) {
        if (FLAC__metadata_get_picture(ep->d_name, &picture, -1, NULL, NULL,
                                       (uint32_t)(-1), (uint32_t)(-1),
                                       (uint32_t)(-1), (uint32_t)(-1))) {
          closedir(dp);
          printf("HTTP/1.1 200 OK\r\n%s%u\r\n%s\r\n%s%s\r\n%s\r\n\r\n",
                 "Content-Length: ", picture->data.picture.data_length,
                 "Cache-control: max-age=31536000, immutable",
                 "Content-Type: ", picture->data.picture.mime_type,
                 "X-Content-Type-Options: nosniff");
          if (fwrite(picture->data.picture.data,
                     picture->data.picture.data_length, 1, stdout) == 1)
            return 0;
          else
            return 1;
        }
      }
    closedir(dp);
  }
  return 1;
}