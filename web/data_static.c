#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern unsigned char index_html[], style_main_css[], script_main_js[],
    style_tracks_css[], script_tracks_js[], style_albums_css[],
    script_albums_js[], favicon32_png[], favicon192_png[];
extern unsigned int index_html_len, style_main_css_len, script_main_js_len,
    style_tracks_css_len, script_tracks_js_len, style_albums_css_len,
    script_albums_js_len, favicon32_png_len, favicon192_png_len;

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize(), write_size;
  char *rsp = malloc(rsp_size), *url = prm[2];
  unsigned char *data;
  unsigned int data_len;
  if (!strcmp("/", url)) {
    data = index_html;
    data_len = index_html_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: text/html; "
                "charset=utf-8\r\nCache-control: no-cache\r\n");
  } else if (!strcmp("/favicon32.png", url)) {
    data = favicon32_png;
    data_len = favicon32_png_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-control: "
                "max-age=31536000, immutable\r\n");
  } else if (!strcmp("/favicon192.png", url)) {
    data = favicon192_png;
    data_len = favicon192_png_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-control: "
                "max-age=31536000, immutable\r\n");
  } else if (!strcmp("/style_main.css", url)) {
    data = style_main_css;
    data_len = style_main_css_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: text/css; "
                "charset=utf-8\r\nCache-control: "
                "max-age=31536000, immutable\r\n");
  } else if (!strcmp("/script_main.js", url)) {
    data = script_main_js;
    data_len = script_main_js_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; "
                "charset=utf-8\r\nCache-control: "
                "max-age=31536000, immutable\r\n");
  } else if (!strcmp("/style_tracks.css", url)) {
    data = style_tracks_css;
    data_len = style_tracks_css_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: text/css; "
                "charset=utf-8\r\nCache-control: "
                "max-age=31536000, immutable\r\n");
  } else if (!strcmp("/script_tracks.js", url)) {
    data = script_tracks_js;
    data_len = script_tracks_js_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; "
                "charset=utf-8\r\nCache-control: "
                "max-age=31536000, immutable\r\n");
  } else if (!strcmp("/style_albums.css", url)) {
    data = style_albums_css;
    data_len = style_albums_css_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: text/css; "
                "charset=utf-8\r\nCache-control: "
                "max-age=31536000, immutable\r\n");
  } else if (!strcmp("/script_albums.js", url)) {
    data = script_albums_js;
    data_len = script_albums_js_len;
    strcpy(rsp, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript; "
                "charset=utf-8\r\nCache-control: "
                "max-age=31536000, immutable\r\n");
  } else
    execl(resp_err, "resp_err", prm[1], NULL);
  write_size = strlen(rsp);
  sprintf(rsp + write_size,
          "Content-Length: %u\r\nX-Content-Type-Options: nosniff\r\n\r\n",
          data_len);
  write_size = write(sock, rsp, strlen(rsp));
  write_size += write(sock, data, data_len);
  if (write_size == strlen(rsp) + data_len)
    return 0;
  else
    return 1;
}
