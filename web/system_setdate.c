#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  ssize_t rsp_size = getpagesize();
  ssize_t write_size;
  char *rsp = malloc(rsp_size);
  char *browser_date;
  browser_date = strchr(prm[2], '?');
  if (browser_date && strlen(++browser_date) > 3) {
    sprintf(rsp, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n", "HTTP/1.1 200 OK",
            "Content-Type: text/html; charset=utf-8", "Cache-control: no-cache",
            "X-Content-Type-Options: nosniff");
    *(browser_date + strlen(browser_date) - 3) = '\0';
    struct timespec date = {strtol(browser_date, NULL, 10), 0};
    if (!clock_settime(CLOCK_REALTIME, &date)) {
      if (write(sock, rsp, strlen(rsp)) == strlen(rsp))
        return 0;
      else
        return 1;
    }
  }
  execl(resp_err, "resp_err", prm[1], NULL);
}
