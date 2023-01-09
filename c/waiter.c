#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>
#include <alsa/control.h>
#include <alsa/mixer.h>

#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>

#define card_name "Wilkins"
#define exec_player_path "/home/exe/player/player"
#define player_name "player"
#define album_file_path "/home/exe/player/tmp/album"
#define track_file_path "/home/exe/player/tmp/track"
#define album_str_len 1024

static pid_t player_pid;

static void corrupt_file(void) {
	int play_file_dstr;
	if ((play_file_dstr = open(album_file_path, O_NONBLOCK|O_WRONLY)) != -1) {
		char play_val = 0;
		write(play_file_dstr, &play_val, 1);
		close(play_file_dstr);
	}
}

void get_file_content(char *file, char *ret) {
	int album_file_dstr;
	if ((album_file_dstr = open(file, O_NONBLOCK|O_RDONLY)) != -1) {
		ssize_t size = read(album_file_dstr, ret, album_str_len);
		ret[size] = 0;
		close(album_file_dstr);
	}
}

static char play_next(void){
	char alb[album_str_len];
	alb[0] = 0;
	get_file_content(album_file_path, alb);
	return alb[0];
}

int main(void){
	while (1) {
		if (!play_next()) {
			sleep(1);
		} else {
			if (player_pid > 0){
				kill(player_pid, SIGTERM);
				int status;
				wait(&status);
				player_pid = 0;
			}
			char album_val[album_str_len];
			get_file_content(album_file_path, album_val);
			corrupt_file();
			char file_to_play[10];
			get_file_content(track_file_path, file_to_play);
			int card_num = snd_card_get_index(card_name);
			if (card_num >= 0){
				char card_pcm_name[7];
				sprintf(card_pcm_name, "hw:%d", card_num);
				player_pid = fork();
				if (!player_pid){
					execl(exec_player_path, player_name, album_val, card_pcm_name, file_to_play, NULL);
				}
			}
		}
	}
}
