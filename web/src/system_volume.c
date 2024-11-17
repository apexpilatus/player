#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// clang-format off
#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>
#include <alsa/pcm.h>
#include <alsa/mixer.h>
// clang-format on

static int init_mixer(snd_mixer_elem_t **melem, long *min_vol, long *max_vol,
                      long *curr_vol) {
  int card_number = -1;
  char card_name[10];
  snd_mixer_t *mxr;
  if (snd_card_next(&card_number) || card_number == -1 ||
      snd_mixer_open(&mxr, 0))
    return 1;
  sprintf(card_name, "hw:%d", card_number);
  if (snd_mixer_attach(mxr, card_name) ||
      snd_mixer_selem_register(mxr, NULL, NULL) || snd_mixer_load(mxr) ||
      !(*melem = snd_mixer_first_elem(mxr)))
    return 1;
  for (; *melem && !snd_mixer_selem_has_playback_volume_joined(*melem);
       *melem = snd_mixer_elem_next(*melem))
    ;
  if (!(*melem))
    return 1;
  if (snd_mixer_selem_get_playback_volume_range(*melem, min_vol, max_vol) ||
      snd_mixer_selem_get_playback_volume(*melem, SND_MIXER_SCHN_UNKNOWN,
                                          curr_vol))
    return 1;
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize();
  ssize_t write_size;
  char *rsp = malloc(rsp_size);
  char *url = prm[2];
  char *url_vol;
  long min_vol;
  long max_vol;
  long curr_vol;
  int fd;
  pid_t pid = getpid();
  while ((fd = open(mix_pid_path, O_WRONLY | O_CREAT | O_EXCL,
                    S_IRUSR | S_IWUSR)) < 0)
    ;
  write_size = write(fd, &pid, sizeof(pid_t));
  close(fd);
  snd_mixer_elem_t *melem;
  if (init_mixer(&melem, &min_vol, &max_vol, &curr_vol))
    execl(resp_err, "resp_err", prm[1], NULL);
  if (!(strcmp("/getvolume", url)))
    sprintf(rsp, "HTTP/1.1 200 %ld_%ld_%ld\r\n", min_vol, curr_vol, max_vol);
  else {
    url_vol = strchr(url, '&');
    if (!(url_vol && (curr_vol = strtol(++url_vol, NULL, 10)) <= max_vol &&
          curr_vol >= min_vol) ||
        snd_mixer_selem_set_playback_volume(melem, SND_MIXER_SCHN_UNKNOWN,
                                            curr_vol))
      execl(resp_err, "resp_err", prm[1], NULL);
    strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  }
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size == strlen(rsp))
    return 0;
  else
    return 1;
}
