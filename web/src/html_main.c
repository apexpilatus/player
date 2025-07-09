#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  struct hostent *host;
  char streamer_address[INET_ADDRSTRLEN];
  char cmd[getpagesize()];
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
  strcat(msg, "</head>");
  strcat(msg, "<body>");
  strcat(msg, "<p hidden id=top></p>");
  strcat(msg, "<p hidden id=current></p>");
  strcat(msg, "<iframe id=albums title=albums></iframe>");
  strcat(msg, "<iframe id=control title=control></iframe>");
  strcat(msg, "<button type=button id=poweroff onclick=poweroff()>"
              "&#9769;</button>");
  if ((host = gethostbyname(streamer_host))) {
    inet_ntop(AF_INET, (struct in_addr *)host->h_addr, streamer_address,
              INET_ADDRSTRLEN);
    sprintf(cmd, "echo \"\r\n\r\"|nc -w 1 %s %s 1>/dev/null 2>/dev/null",
            streamer_address, streamer_port);
    if (!system(cmd)) {
      strcat(msg, "<button type=button id=volume onclick=getvolume(\"");
      strcat(msg, streamer_address);
      strcat(msg, ":");
      strcat(msg, streamer_port);
      strcat(msg, "\")>&#9738</button>");
    }
  }
  strcat(msg, "<button type=button id=getcd onclick=getcd()>"
              "&#9737</button>");
  strcat(msg, "<button hidden type=button id=scrollup "
              "onclick=getalbums()>&#8648</button>");
  strcat(msg, "<button hidden type=button id=scrolldown "
              "onclick=scrolldown()>&#8650</button>");
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
