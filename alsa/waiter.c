#include "shares.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static pid_t mixer_pid;
static char *data_addr;
static int data_size;
static volatile long *target_vol_ptr;
static volatile long *max_vol_ptr;
static int target_vol_size = sizeof(long);
static int max_vol_size = sizeof(long);
static int card_num;

static inline int open_mixer()
{
	*max_vol_ptr = 0;
	if ((card_num = snd_card_get_index("Pro")) >= 0 || (card_num = snd_card_get_index("S50")) >= 0)
	{
		sprintf(data_addr, "hw:%d", card_num);
		mixer_pid = fork();
		if (!mixer_pid)
		{
			execl(exec_mixer_path, mixer_name, NULL);
		}
		while (!*max_vol_ptr)
			;
		if (*max_vol_ptr > 0)
		{
			return 0;
		}
	}
	return 1;
}

static inline int close_mixer()
{
	*max_vol_ptr = 0;
	waitpid(mixer_pid, NULL, 0);
}

static void player0_play(int sock)
{
	if (open_mixer())
	{
		*target_vol_ptr = 0;
		*max_vol_ptr = 0;
	}
	else
	{
		kill(mixer_pid, SIGUSR1);
	}
	close_mixer();
	int album_size, track_size;
	album_size = read(sock, data_addr, data_size);
	write(sock, "ok\n", 3);
	track_size = read(sock, data_addr + album_size + 1, data_size - album_size - 1);
	write(sock, "ok\n", 3);
	if (album_size > 0 && track_size > 0)
	{
		data_addr[album_size++] = '\0';
		data_addr[album_size + track_size++] = '\0';
		if (player_pid > 0)
		{
			kill(player_pid, SIGTERM);
			waitpid(player_pid, NULL, 0);
			player_pid = 0;
		}
		if (card_num >= 0)
		{
			sprintf(data_addr + album_size + track_size, "hw:%d,0", card_num);
			player_pid = fork();
			if (!player_pid)
			{
				execl(exec_player_path, player_name, NULL);
			}
		}
	}
}

static void player1_set_volume(int sock)
{
	if (open_mixer())
	{
		*target_vol_ptr = 0;
		*max_vol_ptr = 0;
	}
	sprintf(data_addr, "%ld;%ld%c", *target_vol_ptr, *max_vol_ptr, '\n');
	write(sock, data_addr, strlen(data_addr));
	ssize_t nbytes;
	while ((nbytes = read(sock, data_addr, data_size)) > 0)
	{
		if (nbytes == target_vol_size)
		{
			for (int i = 0; i < target_vol_size; i++)
			{
				((char *)target_vol_ptr)[target_vol_size - 1 - i] = data_addr[i];
			}
			kill(mixer_pid, SIGUSR1);
		}
	}
	close_mixer();
}

static void player2_stop(int sock)
{
	if (player_pid > 0)
	{
		kill(player_pid, SIGTERM);
		waitpid(player_pid, NULL, 0);
		player_pid = 0;
	}
	write(sock, "ok\n", 3);
}

static void player3_exit(int sock)
{
	player2_stop(sock);
	exit(0);
}

static void (*action[])(int sock) = {
	player0_play,
	player1_set_volume,
	player2_stop,
	player3_exit};

int main(void)
{
	int shd = shm_open(shm_file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (shd < 0)
	{
		return 1;
	}
	if (ftruncate(shd, shm_size()))
	{
		return 1;
	}
	void *shd_addr = mmap(NULL, shm_size(), PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED)
	{
		return 1;
	}
	target_vol_ptr = shd_addr;
	max_vol_ptr = target_vol_ptr + 1;
	data_addr = (char *)shd_addr + target_vol_size + max_vol_size;
	data_size = shm_size() - target_vol_size - max_vol_size;
	*target_vol_ptr = 0;
	int sock_listen, sock;
	sock_listen = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(listen_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t addr_size = sizeof(addr);
	if (bind(sock_listen, (struct sockaddr *)&addr, addr_size) < 0)
	{
		return 1;
	}
	if (listen(sock_listen, 1) < 0)
	{
		return 1;
	}
	while (1)
	{
		if ((sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size)) < 0)
		{
			continue;
		}
		char action_num;
		if (read(sock, &action_num, 1) > 0)
		{
			action[strtol(&action_num, NULL, 10)](sock);
		}
		close(sock);
	}
}
