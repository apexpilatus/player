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


static pid_t player_pid;

void action0_play(int sock) {
	write(sock, "ok\n", 3);
	char album[1024], track[10];
	int album_size, track_size;
	album_size = read(sock, album, 1024);
	write(sock, "ok\n", 3);
	track_size = read(sock, track, 10);
	write(sock, "ok\n", 3);
	if (album_size > 0 && track_size > 0) {
		album[album_size] = 0;
		track[track_size] = 0;
		if (player_pid > 0){
			kill(player_pid, SIGTERM);
			int status;
			wait(&status);
			player_pid = 0;
		}
		int card_num = snd_card_get_index(card_name);
		if (card_num >= 0){
			char card_pcm_name[7];
			sprintf(card_pcm_name, "hw:%d", card_num);
			player_pid = fork();
			if (!player_pid){
				execl(exec_player_path, player_name, album, card_pcm_name, track, NULL);
			}
		}
	}
}

void (*action[])(int sock) = {action0_play};

int main(void){
	int sock_listen, sock;
	sock_listen = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
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
