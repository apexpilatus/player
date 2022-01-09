#include "funcs.h"

int main(int argsn, char *args[]){
	while (1) {
		if (check_play_file() != 1) {
			sleep(time_out);
		} else {
			char album_val[1024];
			get_album(album_val);
			execl("a.out", album_val, NULL);
		}
	}
}
