#include <stdlib.h>
#include <unistd.h>

int main(void) {
  while (1) {
    system(
        "streamer=$(ip route|grep default|awk '{print$3}');"
        "store=$(ip addr show dev wlan0|grep -v inet6|grep inet|"
        "awk 'gsub(\"/\",\" \"){print$2}');"
        "if [ -n \"$streamer\" ];then echo $store|nc -w 5 $streamer 9696;fi");
    sleep(5);
  }
}
