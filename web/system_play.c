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

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize(), write_size;
  char *rsp = malloc(rsp_size);
  char *album_dir, *start_track;
  int card = -2;
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
  printf("%d\n", snd_card_next(&card));
  // track_lst *tracks = get_tracks();

  strcpy(rsp, "HTTP/1.1 200 OK\r\n\r\n");
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size != strlen(rsp))
    return 1;
}
