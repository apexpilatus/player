#ifndef waiter_h
	#include "funcs.h"
#endif

int main(int argsn, char *args[]){
	while (1) {
		if (check_play_file() != 1) {
			sleep(time_out);
		} else {
			char album_val[1024];
			get_album(album_val);
			
			snd_ctl_t *ctl_p;
			if (!snd_ctl_open(&ctl_p, "hw:2", SND_CTL_ASYNC)) {
				execl(exec_play_path, "play.waiter", snd_ctl_name(ctl_p), album_val, NULL);
			}
			
			execl(exec_play_path, "play.waiter", "fook", album_val, NULL);
		}
	}
}
