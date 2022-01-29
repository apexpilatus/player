#ifndef waiter_h
	#include "funcs.h"
#endif

int main(int argsn, char *args[]){
	while (1) {
		if (check_play_file() != 1) {
			void *fuck=malloc(1024);
			sleep(time_out);
		} else {
			char album_val[1024];
			get_album(album_val);
			execl(exec_play_path, "play.waiter", album_val, NULL);
		}
	}
}
