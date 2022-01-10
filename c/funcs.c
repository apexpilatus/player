#ifndef waiter_h
	#include "funcs.h"
#endif

void write_0_to_play_file() {
	int play_file_dstr;
	while ((play_file_dstr = open(play_file_path, O_NONBLOCK|O_WRONLY)) == -1) {
		sleep(time_out);
	}
	int play_val = 0;
	write(play_file_dstr, &play_val, 1);
	close(play_file_dstr);
}

int check_play_file(){
        int play_file_dstr;
        while ((play_file_dstr = open(play_file_path, O_NONBLOCK|O_RDONLY)) == -1) {
                sleep(time_out);
        }
        int play_val;
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

void get_params(char *album_val, unsigned int *rate, unsigned short *frame_size) {
	char file_name[2048];
	sprintf(file_name, "%s/1.wav", album_val);
	int music_file_dstr = open(file_name, O_NONBLOCK|O_RDONLY);
	if (music_file_dstr != -1) {
		lseek(music_file_dstr, 24, SEEK_SET);
		read(music_file_dstr, rate, 4);
		lseek(music_file_dstr, 32, SEEK_SET);
		read(music_file_dstr, frame_size, 2);
		close(music_file_dstr);
	}
}
