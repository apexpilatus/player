#include <cdda_interface.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

char *to_play(void) {
  char *ret = malloc(getpagesize());
  DIR *dp_music;
  struct dirent *albm_ep;
  struct stat stat_buf;
  time_t mtime = 0;
  dp_music = opendir(music_path);
  if (dp_music) {
    while ((albm_ep = readdir(dp_music)))
      if (albm_ep->d_type == DT_DIR && strcmp(albm_ep->d_name, ".") &&
          strcmp(albm_ep->d_name, "..")) {
        char path[getpagesize()];
        sprintf(path, "%s/%s", music_path, albm_ep->d_name);
        if (!stat(path, &stat_buf)) {
          if (stat_buf.st_mtime > mtime) {
            mtime = stat_buf.st_mtime;
            strcpy(ret, path);
          }
        } else
          return NULL;
      }
    closedir(dp_music);
  } else
    return NULL;
  if (difftime(time(NULL), mtime) < 5.0)
    return ret;
  return NULL;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char hdr[getpagesize()];
  char msg[getpagesize() * 1000];
  char *album = to_play();
  strcpy(msg, "<!DOCTYPE html>");
  strcat(msg, "<html lang=en>");
  strcat(msg, "<head>");
  strcat(msg, "<meta charset=utf-8>");
  strcat(
      msg,
      "<meta name=viewport content=\"width=device-width, initial-scale=1.0\">");
  strcat(msg, "<title>player</title>");
  strcat(msg, "<link rel=stylesheet href=style_main.css>");
  strcat(msg, "<link id=icon rel=icon href=");
  strcat(msg, album ? album : "apple-touch-icon.png");
  strcat(msg, " title=");
  strcat(msg, album ? album : "apple-touch-icon");
  strcat(msg, ">");
  strcat(msg, "</head>");
  strcat(msg, "<body>");
  strcat(msg, "<audio id=player autoplay onplaying=updatetop()></audio>");
  strcat(msg, "<p hidden id=topalbum></p>");
  strcat(msg, "<p hidden id=selectedalbum>");
  strcat(msg, album ? album : "");
  strcat(msg, "</p>");
  strcat(msg, "<iframe id=albums title=albums></iframe>");
  strcat(msg, "<iframe id=control title=control></iframe>");
  strcat(msg, "<button type=button id=poweroff onclick=poweroff()>"
              "&#9635;</button>");
  if (cdda_identify("/dev/sr0", CDDA_MESSAGE_FORGETIT, NULL))
    strcat(msg, "<button type=button id=getcd onclick=getcd()>"
                "&#9673</button>");
  strcat(msg, "<button hidden type=button id=scrollup "
              "onclick=scrollup()>&#9650</button>");
  strcat(msg, "<button hidden type=button id=scrolldown "
              "onclick=scrolldown()>&#9660</button>");
  strcat(msg, "<script src=script_main.js></script>");
  strcat(msg, "<script>getalbums()</script>");
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
