#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern unsigned char index_html[];
extern unsigned char style_main_css[];
extern unsigned char script_main_js[];
extern unsigned char style_tracks_css[];
extern unsigned char script_tracks_js[];
extern unsigned char style_albums_css[];
extern unsigned char script_albums_js[];
extern unsigned char favicon_ico[];
extern unsigned char favicon152_png[];
extern unsigned char favicon180_png[];
extern unsigned int index_html_len;
extern unsigned int style_main_css_len;
extern unsigned int script_main_js_len;
extern unsigned int style_tracks_css_len;
extern unsigned int script_tracks_js_len;
extern unsigned int style_albums_css_len;
extern unsigned int script_albums_js_len;
extern unsigned int favicon_ico_len;
extern unsigned int favicon152_png_len;
extern unsigned int favicon180_png_len;

unsigned char *data;
unsigned int data_len;

static inline int select_data(char *url, char *rsp) {
  unsigned long hdr_end;
  printf("%s\n", url);
  if (!strcmp("/", url)) {
    data = index_html;
    data_len = index_html_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/html; charset=utf-8",
            "Cache-control: no-cache");
  } else if (!strcmp("/favicon.ico", url)) {
    data = favicon_ico;
    data_len = favicon_ico_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: image/x-icon",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/apple-touch-icon-precomposed.png", url)) {
    data = favicon152_png;
    data_len = favicon152_png_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: image/png",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/apple-touch-icon.png", url)) {
    data = favicon180_png;
    data_len = favicon180_png_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: image/png",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/style_main.css", url)) {
    data = style_main_css;
    data_len = style_main_css_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/css; charset=utf-8",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/style_tracks.css", url)) {
    data = style_tracks_css;
    data_len = style_tracks_css_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/css; charset=utf-8",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/style_albums.css", url)) {
    data = style_albums_css;
    data_len = style_albums_css_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/css; charset=utf-8",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/script_main.js", url)) {
    data = script_main_js;
    data_len = script_main_js_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/javascript; charset=utf-8",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/script_tracks.js", url)) {
    data = script_tracks_js;
    data_len = script_tracks_js_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/javascript; charset=utf-8",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/script_albums.js", url)) {
    data = script_albums_js;
    data_len = script_albums_js_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/javascript; charset=utf-8",
            "Cache-control: max-age=31536000, immutable");
  } else
    return 1;
  hdr_end = strlen(rsp);
  sprintf(rsp + hdr_end, "%s%u\r\n%s\r\n\r\n", "Content-Length: ", data_len,
          "X-Content-Type-Options: nosniff");
  return 0;
}

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t write_size;
  char *rsp = malloc(getpagesize());
  if (select_data(prm[2], rsp))
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = write(sock, rsp, strlen(rsp));
  write_size += write(sock, data, data_len);
  if (write_size == strlen(rsp) + data_len)
    return 0;
  else
    return 1;
}
