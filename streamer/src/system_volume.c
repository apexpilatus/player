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

static int init_mixer(char *card_name, snd_mixer_elem_t **melem, long *min_vol,
                      long *max_vol) {
  snd_mixer_t *mxr;
  if (snd_mixer_open(&mxr, 0) || snd_mixer_selem_register(mxr, NULL, NULL))
    return 1;
  if (snd_mixer_attach(mxr, card_name) || snd_mixer_load(mxr) ||
      !(*melem = snd_mixer_first_elem(mxr)))
    return 1;
  for (; *melem && !snd_mixer_selem_has_playback_volume_joined(*melem);
       *melem = snd_mixer_elem_next(*melem))
    ;
  if (!(*melem))
    return 1;
  if (snd_mixer_selem_get_playback_volume_range(*melem, min_vol, max_vol))
    return 1;
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char *rsp = malloc(getpagesize());
  char *url_vol;
  char *url_card_name;
  long min_vol;
  long max_vol;
  long curr_vol;
  snd_mixer_elem_t *melem;
  if (!((url_card_name = strchr(prm[2], '?')) &&
        (url_vol = strchr(++url_card_name, '&'))))
    execl(resp_err, "resp_err", prm[1], NULL);
  *url_vol = '\0';
  if (init_mixer(url_card_name, &melem, &min_vol, &max_vol))
    execl(resp_err, "resp_err", prm[1], NULL);
  if (!((curr_vol = strtol(++url_vol, NULL, 10)) <= max_vol &&
        curr_vol >= min_vol) ||
      snd_mixer_selem_set_playback_volume(melem, SND_MIXER_SCHN_UNKNOWN,
                                          curr_vol))
    execl(resp_err, "resp_err", prm[1], NULL);
  strcpy(rsp, "HTTP/1.1 200 OK\r\n");
  strcat(rsp, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(rsp, "Cache-control: no-cache\r\n");
  strcat(rsp, "X-Content-Type-Options: nosniff\r\n\r\n");
  write_size = write(sock, rsp, strlen(rsp));
  if (write_size == strlen(rsp))
    return 0;
  else
    return 1;
}
