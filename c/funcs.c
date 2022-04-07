#ifndef waiter_h
	#include "funcs.h"
#endif

void write_0_to_play_file() {
	int play_file_dstr;
	while ((play_file_dstr = open(album_file_path, O_NONBLOCK|O_WRONLY)) == -1) {
		sleep(time_out);
	}
	char play_val = 0;
	write(play_file_dstr, &play_val, 1);
	close(play_file_dstr);
}

char check_play_file(){
        int play_file_dstr;
        while ((play_file_dstr = open(album_file_path, O_NONBLOCK|O_RDONLY)) == -1) {
                sleep(time_out);
        }
        char play_val;
        read(play_file_dstr, &play_val, 1);
        close(play_file_dstr);
        return play_val;
}

void get_album(char *ret) {
	int album_file_dstr;
	while ((album_file_dstr = open(album_file_path, O_NONBLOCK|O_RDONLY)) == -1) {
		sleep(time_out);
	}
	ssize_t size = read(album_file_dstr, ret, 1024);
	ret[size] = 0;
	close(album_file_dstr);
}

int check_album(char current[]) {
	char next[1024];
	get_album(next);
	return strcmp(current, next);
}
extern file_lst* get_file_lst(char *dir){
	file_lst *main_ptr = malloc(sizeof(file_lst));
	file_lst *cur_ptr = main_ptr;
	cur_ptr->next=NULL;

	DIR *dp;
	struct dirent *ep;
}

int get_params(char *album_val, unsigned int *rate, unsigned short *frame_size) {
	char file_name[2048];
	unsigned int rate_1st;
	unsigned short frame_size_1st;
	for (int i = 1; i < 100; i++) {
		sprintf(file_name, "%s/%d.wav", album_val, i);
		int music_file_dstr = open(file_name, O_NONBLOCK|O_RDONLY);
		if (music_file_dstr != -1) {
			lseek(music_file_dstr, 24, SEEK_SET);
			read(music_file_dstr, rate, 4);
			lseek(music_file_dstr, 32, SEEK_SET);
			read(music_file_dstr, frame_size, 2);
			close(music_file_dstr);
			if (i == 1) {
				rate_1st = *rate;
				frame_size_1st = *frame_size;
			} else {
				if (rate_1st != *rate || frame_size_1st != *frame_size) {
					return 1;
				}
			}
		} else {
			if (i == 1) {
				return 1;
			} else {
				return 0;
			}
		}
	}
}
