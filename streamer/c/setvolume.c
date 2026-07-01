#include <alsa/conf.h>

int init_mixer(char *card_name, snd_mixer_elem_t **melem, long *min_vol,
               long *max_vol) {
  snd_mixer_t *mxr;
  if (snd_mixer_open(&mxr, 0) == 0 &&
      snd_mixer_selem_register(mxr, NULL, NULL) == 0 &&
      snd_mixer_attach(mxr, card_name) == 0 && snd_mixer_load(mxr) == 0 &&
      (*melem = snd_mixer_first_elem(mxr))) {
    for (; *melem && !snd_mixer_selem_has_playback_volume_joined(*melem);
         *melem = snd_mixer_elem_next(*melem))
      ;
    if (*melem && snd_mixer_selem_get_playback_volume_range(*melem, min_vol,
                                                            max_vol) == 0)
      return 0;
  }
  return 1;
}

int main(int prm_n, char *prm[]) {
  long min_vol;
  long max_vol;
  long vol;
  snd_mixer_elem_t *melem;
  char *name = strstr(prm[1], "name=");
  char *value = strstr(prm[1], "value=");
  if (name && value) {
    char *end = strchr(name, '&');
    if (end)
      *end = '\0';
    name += 5;
    end = strchr(value, '&');
    if (end)
      *end = '\0';
    value += 6;
    if (init_mixer(name, &melem, &min_vol, &max_vol) == 0 &&
        (vol = strtol(value, NULL, 10)) <= max_vol && vol >= min_vol &&
        !snd_mixer_selem_set_playback_volume(melem, SND_MIXER_SCHN_UNKNOWN,
                                             vol)) {
      printf("HTTP/1.1 200 %ld\r\n", vol);
      printf("Content-Type: text/html; charset=utf-8\r\n");
      printf("Cache-control: no-cache\r\n");
      printf("X-Content-Type-Options: nosniff\r\n\r\n");
      return 0;
    }
  }
  return 1;
}
