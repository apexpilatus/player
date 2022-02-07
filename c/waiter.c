#ifndef waiter_h
	#include "funcs.h"
#endif

int main(int argsn, char *args[]){
	while (1) {
		if (check_play_file() != 1) {
			sleep(time_out);
		} else {
			char album_val[1024];
			get_album(album_val);
			
			snd_ctl_t *pcm_c;
			if (snd_ctl_open (&pcm_c, "hw:2,0", SND_CTL_NONBLOCK)) {
				//execl(exec_waiter_path, "play.waiter", "cannot open pcm", NULL);
				execl(exec_play_path, "play.waiter", album_val, /*snd_ctl_name(pcm_c)*/ "fuck", NULL);
			}
			
			execl(exec_play_path, "play.waiter", album_val, "fook", NULL);
		}
	}
}
