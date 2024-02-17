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

int main(int prm_n, char *prm[]) {
  int card_number = -1, sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize(), write_size;
  char card_name[10], *rsp = malloc(rsp_size), *url = prm[2];
  long min_vol, max_vol, curr_vol;
  snd_mixer_t *mxr;
  snd_mixer_elem_t *melem;
  if (snd_card_next(&card_number) || card_number == -1 ||
      snd_mixer_open(&mxr, 0))
    execl(resp_err, "resp_err", prm[1], NULL);
  sprintf(card_name, "hw:%d", card_number);
  if (snd_mixer_attach(mxr, card_name) ||
      snd_mixer_selem_register(mxr, NULL, NULL) || snd_mixer_load(mxr) ||
      !(melem = snd_mixer_first_elem(mxr)))
    execl(resp_err, "resp_err", prm[1], NULL);
  for (; melem && !snd_mixer_selem_has_playback_volume_joined(melem);
       melem = snd_mixer_elem_next(melem))
    ;
  if (!melem)
    execl(resp_err, "resp_err", prm[1], NULL);
  if (!(strcmp("/getvolume", url))) {
    snd_mixer_selem_get_playback_volume_range(melem, &min_vol, &max_vol);
    snd_mixer_selem_get_playback_volume(melem, SND_MIXER_SCHN_UNKNOWN,
                                        &curr_vol);
    sprintf(rsp, "HTTP/1.1 200 %ld_%ld_%ld\r\n\r\n", min_vol, curr_vol,
            max_vol);
  } else {
    strcpy(rsp, "HTTP/1.1 200 OK\r\n\r\n");
  }
  printf("min - %ld;vol - %ld;max - %ld\n", min_vol, curr_vol, max_vol);
  snd_mixer_selem_set_playback_volume(melem, SND_MIXER_SCHN_UNKNOWN,
                                      --curr_vol);

  write_size = write(sock, rsp, strlen(rsp));
  if (write_size == strlen(rsp))
    return 0;
  else
    return 1;
}
