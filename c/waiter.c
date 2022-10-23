#ifndef waiter_h
	#include "funcs.h"
#endif

int main(int argsn, char *args[]){
	while (1) {
		if (check_play() == 0) {
			sleep(time_out);
		} else {
			char album_val[album_str_len];
			get_album(album_val);
			unsigned int rate;
			unsigned short sample_size;
			file_lst *files=get_file_lst(album_val);
			if (!files->next && !files->name){
				stop_play();
				execl(exec_waiter_path, "play.waiter", "directory is empty", (char *) NULL);
			}
			if (get_params(album_val, files, &rate, &sample_size)){
				stop_play();
				execl(exec_waiter_path, "play.waiter", "files have different format or cannot read", files->name, (char *) NULL);
			}
			char rate_as_str[6], sample_size_as_str[3];
			sprintf(rate_as_str, "%d", rate);
			sprintf(sample_size_as_str, "%d", sample_size);
			int card_num = snd_card_get_index(card_name);
			if (card_num >= 0){
				char card_pcm_name[7];
				sprintf(card_pcm_name, "hw:%d", card_num);
				char *env[] = {malloc(1050), malloc(50), (char *) NULL};
				strcpy(env[0], curr_album_env);
				strcpy(env[0]+strlen(env[0]), "=");
				strcpy(env[0]+strlen(env[0]), album_val);
				strcpy(env[1], card_name_env);
				strcpy(env[1]+strlen(env[1]), "=");
				strcpy(env[1]+strlen(env[1]), card_pcm_name);
				execle(exec_play_path, "play.waiter", rate_as_str, sample_size_as_str, (char *) NULL, env);
			}
			stop_play();
			execl(exec_waiter_path, "play.waiter", "no card to play", (char *) NULL);
		}
	}
}
