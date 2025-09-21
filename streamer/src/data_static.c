#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern unsigned char static_style_volume_css[];
extern unsigned char static_script_volume_js[];
extern unsigned char static_favicon_ico[];
extern unsigned char static_favicon152_png[];
extern unsigned char static_favicon180_png[];
extern unsigned int static_style_volume_css_len;
extern unsigned int static_script_volume_js_len;
extern unsigned int static_favicon_ico_len;
extern unsigned int static_favicon152_png_len;
extern unsigned int static_favicon180_png_len;
unsigned char *data;
unsigned int data_len;

int select_data(char *url, char *rsp) {
  size_t hdr_end;
  if (!strcmp("/favicon.ico", url)) {
    data = static_favicon_ico;
    data_len = static_favicon_ico_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: image/x-icon",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/apple-touch-icon-precomposed.png", url)) {
    data = static_favicon152_png;
    data_len = static_favicon152_png_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: image/png",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/apple-touch-icon.png", url)) {
    data = static_favicon180_png;
    data_len = static_favicon180_png_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: image/png",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/style_volume.css", url)) {
    data = static_style_volume_css;
    data_len = static_style_volume_css_len;
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/css; charset=utf-8",
            "Cache-control: max-age=31536000, immutable");
  } else if (!strcmp("/script_volume.js", url)) {
    data = static_script_volume_js;
    data_len = static_script_volume_js_len;
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
