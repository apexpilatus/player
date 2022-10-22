#ifndef waiter_h
	#include "funcs.h"
#endif

int main(int argsn, char *args[]){
	while (1) {
		if (check_play_val() == 0) {
			sleep(time_out);
		} else {
			char album_val[1024];
			get_album(album_val);
			unsigned int rate;
			unsigned short sample_size;
			file_lst *files=get_file_lst(album_val);
			if (!files->next && !files->name){
				write_0_to_album_file();
				execl(exec_waiter_path, "play.waiter", "directory is empty", (char *) NULL);
			}
			if (get_params(album_val, files, &rate, &sample_size)){
				write_0_to_album_file();
				execl(exec_waiter_path, "play.waiter", "files have different format or cannot read", files->name, (char *) NULL);
			}
			char rate_as_str[6], sample_size_as_str[3];
			snprintf(rate_as_str, 6, "%d", rate);
			snprintf(sample_size_as_str, 3, "%d", sample_size);
			int card_num = snd_card_get_index(card_name);
			if (card_num >= 0){
				char card_pcm_name[7];
				snprintf(card_pcm_name, 7, "hw:%d,0", card_num);
				char *env[] = {malloc(1050), (char *) NULL};
				strcpy(env[0], "ALBM=");
				strcpy(env[0]+5, album_val);
				execle(exec_play_path, "play.waiter", card_pcm_name, rate_as_str, sample_size_as_str, album_val, (char *) NULL, env);
			}
			write_0_to_album_file();
			execl(exec_waiter_path, "play.waiter", "no card to play", (char *) NULL);
		}
	}
}
