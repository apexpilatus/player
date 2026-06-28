#include <alsa/conf.h>

int get_cards() {
  int card_number = -1;
  printf("HTTP/1.1 200");
  while (!snd_card_next(&card_number) && card_number != -1)
    printf(" %d", card_number);
  printf("\r\n%s\r\n%s\r\n%s\r\n\r\n", "Content-Type: text/html; charset=utf-8",
         "Cache-control: no-cache", "X-Content-Type-Options: nosniff");
  return 0;
}

int get_volume(char *card_id) {
  snd_mixer_t *mxr;
  char *card_name;
  snd_mixer_elem_t *melem;
  long min_vol;
  long max_vol;
  long curr_vol;
  if (!(snd_mixer_open(&mxr, 0) || snd_mixer_selem_register(mxr, NULL, NULL))) {
    if (snd_card_get_index(card_id + 3) >= 0 &&
        !snd_card_get_name(snd_card_get_index(card_id + 3), &card_name)) {
      if (!(snd_mixer_attach(mxr, card_id) || snd_mixer_load(mxr))) {
        melem = snd_mixer_first_elem(mxr);
        for (; melem && !snd_mixer_selem_has_playback_volume_joined(melem);
             melem = snd_mixer_elem_next(melem))
          ;
        if (melem && !(snd_mixer_selem_get_playback_volume_range(
                           melem, &min_vol, &max_vol) ||
                       snd_mixer_selem_get_playback_volume(
                           melem, SND_MIXER_SCHN_UNKNOWN, &curr_vol))) {
          printf("HTTP/1.1 200 %ld;%ld;%ld\r\n%s%lu\r\n%s\r\n\r\n%s", min_vol,
                 curr_vol, max_vol, "Content-Length: ", strlen(card_name),
                 "Content-Type: text/plain", card_name);
          return 0;
        }
      }
    }
  }
  return 1;
}

int main(int prm_n, char *prm[]) {
  if (prm_n > 1)
    return get_volume(prm[1]);
  else
    return get_cards();
}
