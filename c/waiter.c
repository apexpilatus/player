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
			
			snd_pcm_t *pcm_p;
			if (!snd_pcm_open(&pcm_p, "hw:2,0", SND_PCM_STREAM_PLAYBACK, 0)) {
				snd_pcm_info_t * pcm_info = malloc(snd_pcm_info_sizeof());
				snd_pcm_info(pcm_p, pcm_info);
				execl(exec_play_path, "play.waiter", snd_pcm_info_get_name(pcm_info), album_val, NULL);
			}
			
			execl(exec_play_path, "play.waiter", "fook", album_val, NULL);
		}
	}
}
