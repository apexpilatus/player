#include "shares.h"
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/mman.h>

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
#define exec_mixer_path "/home/exe/player/mixer"
#define mixer_name "mixer"
#define listen_port 8888
#define file_str_size 10


static pid_t player_pid;
void * shd_addr;

void action0_play(int sock) {
	write(sock, "ok\n", 3);
	char track[file_str_size];
	int album_size, track_size;
	album_size = read(sock, shd_addr + 1, getpagesize() - 1);
	write(sock, "ok\n", 3);
	track_size = read(sock, track, file_str_size);
	write(sock, "ok\n", 3);
	if (album_size > 0 && track_size > 0) {
		((char*)shd_addr)[album_size + 1] = 0;
		track[track_size] = 0;
		if (player_pid > 0){
			kill(player_pid, SIGTERM);
			waitpid(player_pid, NULL, 0);
			player_pid = 0;
		}
		int card_num = snd_card_get_index(card_name);
		if (card_num >= 0){
			char card_pcm_name[7];
			sprintf(card_pcm_name, "hw:%d", card_num);
			player_pid = fork();
			if (!player_pid){
				execl(exec_player_path, player_name, shd_addr + 1, card_pcm_name, track, NULL);
			}
		}
	}
}

void action1_set_vol(int sock) {
	write(sock, "ok\n", 3);
	if (read(sock, shd_addr, 1) > 0 && player_pid > 0) {
		int card_num = snd_card_get_index(card_name);
		if (card_num >= 0){
			char card_pcm_name[7];
			sprintf(card_pcm_name, "hw:%d", card_num);
			pid_t mixer_pid = fork();
			if (!mixer_pid){
				execl(exec_mixer_path, mixer_name, card_pcm_name, NULL);
			}
			if (mixer_pid > 0){
				waitpid(mixer_pid, NULL, 0);
			}
		}
	}
	write(sock, "ok\n", 3);
}

void action2_get_vol(int sock) {
	write(sock, shd_addr, 1);
}

void (*action[])(int sock) = {
	action0_play,
	action1_set_vol,
	action2_get_vol
};

int main(void){
	int shd = shm_open(shm_file, O_CREAT|O_RDWR|O_APPEND, S_IRUSR|S_IWUSR);
	if (shd < 0) {
		return 1;
	}
	int page_size = getpagesize();
	ftruncate(shd, page_size);
	shd_addr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED){
		return 1;
	}
	*(char*)shd_addr = 5;
	int sock_listen, sock;
	sock_listen = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(listen_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t addr_size = sizeof(addr);
	if (bind(sock_listen, (struct sockaddr *) &addr, addr_size) < 0)
	{
		return 1;
	}
	if (listen(sock_listen, 1) < 0)
	{
		return 1;
	}
	while(1)
	{
		if ((sock = accept(sock_listen, (struct sockaddr *) &addr, &addr_size)) < 0)
		{
			continue;
		}
		char action_num;
		if (read(sock, &action_num, 1) > 0) {
			action[action_num](sock);
		}
		close(sock);
	}
}
