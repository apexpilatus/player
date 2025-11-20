#include <cdda_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  char *url = prm[2];
  ssize_t write_size;
  char hdr[getpagesize()];
  char msg[getpagesize() * 1000];
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(
      msg,
      "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">");
  strcat(msg, "<title>player</title>");
  strcat(msg, "<link rel=stylesheet href=style_main.css>");
  if (!strcmp("/", url)) {
    strcat(msg, "<link id=icon rel=icon href=apple-touch-icon.png>");
    strcat(msg, "</head>");
    strcat(msg, "<body>");
    strcat(msg, "<iframe id=albums title=albums src=/albums></iframe>");
  } else {
    char *album = strstr(url, "album=");
    char *scroll = strstr(url, "scroll=");
    if (album) {
      char *end = strchr(album, '&');
      if (end)
        *end = '\0';
      strcat(msg, "<link id=icon rel=icon href=");
      strcat(msg, album + 6);
      strcat(msg, ">");
      if (end)
        *end = '&';
    } else {
      strcat(msg, "<link id=icon rel=icon href=apple-touch-icon.png>");
    }
    strcat(msg, "</head>");
    strcat(msg, "<body>");
    if (scroll) {
      char *end = strchr(album, '&');
      if (end)
        *end = '\0';
      strcat(msg, "<iframe id=albums title=albums src=/albums?");
      strcat(msg, scroll);
      strcat(msg, "></iframe>");
      if (end)
        *end = '&';
    }
    /*  if (album) {
        if (*(album + 1) != '/')
          strcat(msg,
                 "<link id=icon rel=icon href=apple-touch-icon.png>");
        else {
          char *end = strchr(album, '&');
          if (end)
            *end = '\0';
          strcat(msg, "<link id=icon rel=icon href=");
          strcat(msg, album + 1);
          strcat(msg, ">");
          if (end)
            *end = '&';
        }
      } else
        strcat(msg,
               "<link id=icon rel=icon href=apple-touch-icon.png>");*/
  }
  /*strcat(msg, "</head>");
  strcat(msg, "<body>");
  if (strncmp("/inbrowser", url, strlen("/inbrowser")))
    strcat(msg, "<audio id=player title=manual onended=loaddefault()></audio>");
  else {
    if (album) {
      strcat(
          msg,
          "<audio id=player title=autoplay onended=loaddefault() src=stream_");
      if (*(album + 1) != '/')
        strcat(msg, "cd");
      else
        strcat(msg, "album");
      strcat(msg, album);
      strcat(msg, "></audio>");
    } else
      strcat(msg,
             "<audio id=player title=manual onended=loaddefault()></audio>");
  }
  strcat(msg, "<p hidden id=topalbum></p>");
  strcat(msg, "<p hidden id=selectedalbum></p>");
  strcat(msg, "<p id=position onclick=updateposition()>0</p>");
  strcat(msg, "<iframe id=albums title=albums
  onscroll=updateposition()></iframe>"); strcat(msg, "<iframe id=control
  title=control class="); if (strncmp("/inbrowser", url, strlen("/inbrowser"))
  || (album && *(album + 1) == '/')) strcat(msg, "tracks"); else strcat(msg,
  "cdcontrol src=cdcontrol"); strcat(msg, "></iframe>"); strcat(msg, "<button
  type=button id=poweroff onclick=poweroff()>"
              "&#9635;</button>");
  if (cdda_identify("/dev/sr0", CDDA_MESSAGE_FORGETIT, NULL))
    strcat(msg, "<button type=button id=getcd onclick=getcd()>"
                "&#9673</button>");
  strcat(msg, "<button hidden type=button id=scrollup "
              "onclick=scrollup()>&#9650</button>");
  strcat(msg, "<button hidden type=button id=scrolldown "
              "onclick=scrolldown()>&#9660</button>");
  strcat(msg, "<script src=script_main.js></script>");
  if (strncmp("/inbrowser", url, strlen("/inbrowser")) ||
      (album && *(album + 1) == '/'))
    strcat(msg, "<script>getalbums()</script>");
  else
    strcat(msg, "<script>scrollup()</script>");*/
  strcat(msg, "</body>");
  strcat(msg, "</html>");
  strcpy(hdr, "HTTP/1.1 200 OK\r\n");
  strcat(hdr, "Content-Type: text/html; charset=utf-8\r\n");
  write_size = strlen(hdr);
  sprintf(hdr + write_size, "Content-Length: %lu\r\n\r\n", strlen(msg));
  write_size = write(sock, hdr, strlen(hdr));
  write_size += write(sock, msg, strlen(msg));
  if (write_size == strlen(hdr) + strlen(msg))
    return 0;
  else
    return 1;
}
