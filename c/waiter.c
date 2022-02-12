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
			if (get_params(album_val, &rate, &frame_size)){
				write_0_to_play_file();
				execl(exec_waiter_path, "play.waiter", "files have different format or cannot read first file", NULL);
			}
			char rate_as_str[10], frame_size_as_str[10];
			snprintf(rate_as_str, 6, "%d", rate);
			snprintf(frame_size_as_str, 2, "%d", frame_size);
			int card_num = snd_card_get_index(frame_size == 4 ? "II" : "U96khz");
			if (card_num > 0){
				char card_name[1024];
				
				sprintf(name, "hw:%d", card_ind);
				snd_ctl_t *ctl_p;
				if (!snd_ctl_open(&ctl_p, name, SND_CTL_NONBLOCK)){
					snd_ctl_nonblock(ctl_p, 0);
					snd_ctl_elem_list_t *elist;
					snd_ctl_elem_list_malloc(&elist);
					snd_ctl_elem_list(ctl_p, elist);
					int ecount;
					ecount = snd_ctl_elem_list_get_count(elist);
					snd_ctl_elem_list_alloc_space(elist, ecount);
					snd_ctl_elem_list(ctl_p, elist);
					snd_ctl_elem_list_free_space(elist);
					snd_ctl_elem_list_free(elist);
					
					printf("fuck\n");
					
					snd_ctl_close(ctl_p);
				}
				
				sprintf(card_name, "hw:%d,0", card_num);
				execl(exec_play_path, "play.waiter", card_name, rate_as_str, frame_size_as_str, album_val, NULL);
			}
			write_0_to_play_file();
			execl(exec_waiter_path, "play.waiter", "no card to play", NULL);
		}
	}
}
