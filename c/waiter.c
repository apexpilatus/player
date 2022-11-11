#ifndef waiter_h
	#include "funcs.h"
#endif

static void corrupt_file(void) {
	int play_file_dstr;
	if ((play_file_dstr = open(album_file_path, O_NONBLOCK|O_WRONLY)) != -1) {
		char play_val = 0;
		write(play_file_dstr, &play_val, 1);
		close(play_file_dstr);
	}
}

static int get_params(char *album_val, file_lst *files, unsigned int *rate, unsigned short *sample_size){
	char file_name[2048];
	file_lst *first_file=files;
	unsigned int rate_1st;
	unsigned short sample_size_1st;
	while (files->next) {
		sprintf(file_name, "%s/%s", album_val, files->name);
		FLAC__StreamMetadata streaminfo;
		if (FLAC__metadata_get_streaminfo(file_name, &streaminfo)) {
			*rate = streaminfo.data.stream_info.sample_rate;
			*sample_size = streaminfo.data.stream_info.bits_per_sample;
			if (first_file == files) {
				rate_1st = *rate;
				sample_size_1st = *sample_size;
			} else {
				if (rate_1st != *rate || sample_size_1st != *sample_size) {
					return 1;
				}
			}
		} else {
			return 1;
		}
		files=files->next;
	}
	return 0;
}

void main(int argsn, char *args[]){
	while (1) {
		if (!play_next()) {
			sleep(time_out);
		} else {
			char album_val[album_str_len];
			get_album(album_val);
			unsigned int rate;
			unsigned short sample_size;
			file_lst *files=get_file_lst(album_val);
			if (!files->next && !files->name){
				corrupt_file();
				execl(exec_waiter_path, player_name, "directory is empty", (char *) NULL);
			}
			if (get_params(album_val, files, &rate, &sample_size)){
				corrupt_file();
				execl(exec_waiter_path, player_name, "files have different format or cannot read", files->name, (char *) NULL);
			}
			char rate_as_str[7], sample_size_as_str[3];
			sprintf(rate_as_str, "%d", rate);
			sprintf(sample_size_as_str, "%d", sample_size);
			int card_num = snd_card_get_index(card_name);
			if (card_num >= 0){
				char card_pcm_name[7];
				sprintf(card_pcm_name, "hw:%d", card_num);
				char *env[] = {malloc(album_str_len + 50), malloc(50), malloc(50), malloc(50), (char *) NULL};
				strcpy(env[0], curr_album_env);
				strcpy(env[0]+strlen(env[0]), "=");
				strcpy(env[0]+strlen(env[0]), album_val);
				strcpy(env[1], card_name_env);
				strcpy(env[1]+strlen(env[1]), "=");
				strcpy(env[1]+strlen(env[1]), card_pcm_name);
				strcpy(env[2], rate_env);
				strcpy(env[2]+strlen(env[2]), "=");
				strcpy(env[2]+strlen(env[2]), rate_as_str);
				strcpy(env[3], sample_size_env);
				strcpy(env[3]+strlen(env[3]), "=");
				strcpy(env[3]+strlen(env[3]), sample_size_as_str);
				corrupt_file();
				execle(exec_play_path, player_name, (char *) NULL, env);
			}
			corrupt_file();
			execl(exec_waiter_path, player_name, "no card to play", (char *) NULL);
		}
	}
}
