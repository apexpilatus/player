#ifndef waiter_h
	#include "funcs.h"
#endif

int main(int argsn, char *args[]){
	while (1) {
		if (check_play_file() == 0) {
			sleep(time_out);
		} else {
			fuck++;
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
			if (card_num >= 0){
				if (frame_size != 4) {
					char card_name[10];
					snprintf(card_name, 5, "hw:%d", card_num);
					snd_ctl_t *ctl_p;
					if (!snd_ctl_open(&ctl_p, card_name, SND_CTL_NONBLOCK)){
						snd_ctl_elem_list_t *elist;
						snd_ctl_elem_list_malloc(&elist);
						snd_ctl_elem_list(ctl_p, elist);
						int ecount;
						ecount = snd_ctl_elem_list_get_count(elist);
						snd_ctl_elem_list_alloc_space(elist, ecount);
						snd_ctl_elem_list(ctl_p, elist);
						for (int i=0; i<ecount; i++){
							if (!strcmp(snd_ctl_elem_list_get_name(elist, i)+13, "Switch")) {
								snd_ctl_elem_value_t *eval;
								snd_ctl_elem_value_malloc(&eval);
								snd_ctl_elem_id_t *e_id;
								snd_ctl_elem_id_malloc(&e_id);
								snd_ctl_elem_list_get_id(elist, i, e_id);
								snd_ctl_elem_value_set_id(eval, e_id);
								snd_ctl_elem_value_set_numid(eval, snd_ctl_elem_list_get_numid(elist, i));
								snd_ctl_elem_read(ctl_p, eval);
								snd_ctl_elem_value_set_boolean(eval, 0, 1);
								snd_ctl_elem_write(ctl_p, eval);
							}
							if (!strcmp(snd_ctl_elem_list_get_name(elist, i)+13, "Volume")){
								snd_ctl_elem_value_t *eval;
								snd_ctl_elem_value_malloc(&eval);
								snd_ctl_elem_id_t *e_id;
								snd_ctl_elem_id_malloc(&e_id);
								snd_ctl_elem_list_get_id(elist, i, e_id);
								snd_ctl_elem_value_set_id(eval, e_id);
								snd_ctl_elem_value_set_numid(eval, snd_ctl_elem_list_get_numid(elist, i));
								snd_ctl_elem_read(ctl_p, eval);
								snd_ctl_elem_value_set_integer(eval, 0, 110);
								snd_ctl_elem_value_set_integer(eval, 1, 110);
								snd_ctl_elem_write(ctl_p, eval);
							}
						}
						snd_ctl_close(ctl_p);
					}
				}
				char card_pcm_name[10];
				snprintf(card_pcm_name, 7, "hw:%d,0", card_num);
				execl(exec_play_path, "play.waiter", card_pcm_name, rate_as_str, frame_size_as_str, album_val, NULL);
			}
			write_0_to_play_file();
			char fook[1024];
			sprintf(fook, "%d", fuck);
			execl(exec_waiter_path, "play.waiter", "no card to play", fook, NULL);
		}
	}
}
