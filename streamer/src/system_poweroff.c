#include <stdlib.h>
#include <unistd.h>

int main(int prm_n, char *prm[]) {
  int sock = strtol(prm[1], NULL, 10);
  close(sock);
  if (!system("poweroff"))
    return 0;
  else
    return 1;
}
