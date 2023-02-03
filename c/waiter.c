#include "shares.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/mman.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>

#define player_name "player"
#define mixer_name "mixer"
#define listen_port 8888

static pid_t player_pid;
static void * shd_addr;

static inline void update_mixer() {
	int mixer_card_num = snd_card_get_index(card_name);
	if (mixer_card_num >= 0){
		sprintf((char*)shd_addr + 1, "hw:%d", mixer_card_num);
		pid_t mixer_pid = fork();
		if (!mixer_pid){
			execl(exec_mixer_path, mixer_name, (char*)shd_addr + 1, NULL);
		}
		if (mixer_pid > 0){
			waitpid(mixer_pid, NULL, 0);
		}
	}
}

static void action0_play(int sock) {
	write(sock, "ok\n", 3);
	update_mixer();
	int album_size, track_size;
	album_size = read(sock, (char*)shd_addr + 1, getpagesize() - 1);
	write(sock, "ok\n", 3);
	track_size = read(sock, (char*)shd_addr + album_size + 2, getpagesize() - album_size - 2);
	write(sock, "ok\n", 3);
	if (album_size > 0 && track_size > 0) {
		((char*)shd_addr)[album_size + 1] = 0;
		((char*)shd_addr)[album_size + track_size + 2] = 0;
		if (player_pid > 0){
			kill(player_pid, SIGTERM);
			waitpid(player_pid, NULL, 0);
			player_pid = 0;
		}
		int card_num = snd_card_get_index(card_name);
		if (card_num >= 0){
			sprintf((char*)shd_addr + album_size + track_size + 3, "hw:%d,0", card_num);
			player_pid = fork();
			if (!player_pid){
				execl(exec_player_path, player_name, (char*)shd_addr + 1, (char*)shd_addr + album_size + track_size + 3, (char*)shd_addr + album_size + 2, NULL);
			}
		}
	}
}

static void action1_set_vol(int sock) {
	write(sock, "ok\n", 3);
	if (read(sock, shd_addr, 1) > 0) {
		update_mixer();
	}
	write(sock, "ok\n", 3);
}

static void action2_get_vol(int sock) {
	write(sock, shd_addr, 1);
}

static void action3_stop(int sock) {
	if (player_pid > 0){
		kill(player_pid, SIGTERM);
		waitpid(player_pid, NULL, 0);
		player_pid = 0;
	}
	write(sock, "ok\n", 3);
}

static void (*action[])(int sock) = {
	action0_play,
	action1_set_vol,
	action2_get_vol,
	action3_stop
};

int main(void){
	int shd = shm_open(shm_file, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	if (shd < 0) {
		return 1;
	}
	int page_size = getpagesize();
	if (ftruncate(shd, page_size)){
		return 1;
	}
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
