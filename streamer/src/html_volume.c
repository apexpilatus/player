#include <alsa/conf.h>

void list_controls(char *msg) {
  int card_number = -1;
  char alsa_name[10];
  snd_mixer_t *mxr;
  snd_mixer_elem_t *melem;
  long min_vol;
  long max_vol;
  long curr_vol;
  size_t msg_end;
  char *human_name;
  if (snd_mixer_open(&mxr, 0) || snd_mixer_selem_register(mxr, NULL, NULL))
    return;
  while (!snd_card_next(&card_number) && card_number != -1) {
    sprintf(alsa_name, "hw:%d", card_number);
    if (!(snd_mixer_attach(mxr, alsa_name) || snd_mixer_load(mxr))) {
      strcat(msg, "<p><label for=");
      strcat(msg, alsa_name);
      strcat(msg, ">");
      snd_card_get_name(card_number, &human_name);
      strcat(msg, human_name);
      free(human_name);
      strcat(msg, "</label><br>");
      melem = snd_mixer_first_elem(mxr);
      for (; melem && !snd_mixer_selem_has_playback_volume_joined(melem);
           melem = snd_mixer_elem_next(melem))
        ;
      if (melem && !(snd_mixer_selem_get_playback_volume_range(melem, &min_vol,
                                                               &max_vol) ||
                     snd_mixer_selem_get_playback_volume(
                         melem, SND_MIXER_SCHN_UNKNOWN, &curr_vol))) {
        strcat(msg, "<input type=range ");
        msg_end = strlen(msg);
        sprintf(
            msg + msg_end,
            "id=%s name=%s min=%ld max=%ld value=%ld oninput=setlevel(\"%s\")",
            alsa_name, alsa_name, min_vol, max_vol, curr_vol, alsa_name);
        strcat(msg, " title=volume>");
      }
      strcat(msg, "</p>");
      snd_mixer_free(mxr);
      snd_mixer_detach(mxr, alsa_name);
    }
  }
}

void create_html(char *msg) {
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(
      msg,
      "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">");
  strcat(msg, "<title>volume</title>");
  strcat(msg, "<link rel=stylesheet href=style_volume.css>");
  strcat(msg, "<link rel=icon href=apple-touch-icon.png>");
  strcat(msg, "<script src=script_volume.js></script>");
  strcat(msg, "</head>");
  strcat(msg, "<body>");
  strcat(msg, "<button type=button onclick=poweroff()>&#9769;</button>");
  strcat(msg, "<form>");
  list_controls(msg);
  strcat(msg, "</form>");
  strcat(msg, "</body>");
  strcat(msg, "</html>");
}

void create_header(char *hdr, unsigned long msg_len) {
  size_t hdr_end;
  strcpy(hdr, "HTTP/1.1 200 OK\r\n");
  strcat(hdr, "Content-Type: text/html; charset=utf-8\r\n");
  strcat(hdr, "Cache-control: no-cache\r\n");
  strcat(hdr, "X-Content-Type-Options: nosniff\r\n");
  hdr_end = strlen(hdr);
  sprintf(hdr + hdr_end, "Content-Length: %lu\r\n\r\n", msg_len);
}

int main(int prm_n, char *prm[]) {
  int sock;
  ssize_t write_size;
  char *hdr;
  char *msg;
  sock = strtol(prm[1], NULL, 10);
  hdr = malloc(getpagesize());
  msg = malloc(getpagesize() * 10000);
  create_html(msg);
  create_header(hdr, strlen(msg));
  write_size = write(sock, hdr, strlen(hdr));
  write_size += write(sock, msg, strlen(msg));
  if (write_size == strlen(hdr) + strlen(msg))
    return 0;
  else
    return 1;
}
