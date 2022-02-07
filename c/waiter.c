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
			char rate_as_str[100];
			sprintf(rate_as_str, "%d", rate);
			char **card_name;
			for (int i=0; i<6; i++) {
				if (!snd_card_get_name(i, card_name) && !memcmp(*card_name, frame_size == 4 ? "irDAC II" : "USB Audi", 8)) {
					char hw_name[100];
					sprintf(*hw_name, "hw:%d,0", i);
					execl(exec_play_path, "play.waiter", *hw_name, rate_as_str, *card_name, album_val, NULL);
				}				
			}
			write_0_to_play_file();
			execl(exec_waiter_path, "play.waiter", "no card to play", NULL);
		}
	}
}
