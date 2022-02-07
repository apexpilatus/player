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
			unsigned int rate;
			unsigned short frame_size;
			get_params(album_val, &rate, &frame_size);
			char **card_name;
			for (int i=0; i<6; i++) {
				if (!snd_card_get_name(i, card_name) && !memcmp(*card_name, frame_size == 4 ? "irDAC II" : "USB Audi", 8)) {
					sprintf(*card_name, "hw:%d,0", i);
					execl(exec_play_path, "play.waiter", *card_name, album_val, NULL);
				}				
			}
		}
	}
}
