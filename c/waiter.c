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
			char **card_name;
			if (!snd_card_get_name(2, card_name) && !memcmp(*card_name, "irDAC II", 8)) {
				execl(exec_play_path, "play.waiter", *card_name, album_val, NULL);
			}
			
			execl(exec_play_path, "play.waiter", "fook", album_val, NULL);
		}
	}
}
