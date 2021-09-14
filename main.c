#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>
#include <alsa/pcm.h>
#include <alsa/error.h>

const int time_out = 3;
const char play_file_path[] = "/home/player/play";
const char music_root[] = "/home/music";
const char album_file_path[] = "/home/player/album";

void write_0_to_play_file() {
	FILE *play_file;
	while (!(play_file = fopen(play_file_path, "wb"))) {
		sleep(time_out);
	}
	char play_val = 0;
	fwrite(&play_val, 1, 1, play_file);
	fclose(play_file);
}

void get_album(char *ret) {
	FILE *album_file;
	while (!(album_file = fopen(album_file_path, "r"))) {
		sleep(time_out);
	}
	fgets(ret, 1024, album_file);
	fclose(album_file);
}

int check_album(char current[]) {
	char next[1024];
	get_album(next);
	return strcmp(current, next);
}

int main() {
	while (1) {
		FILE *play_file;
		while (!(play_file = fopen(play_file_path, "rb"))) {
			sleep(time_out);
		}
		char play_val;
		fread(&play_val, 1, 1, play_file);
		fclose(play_file);
		if (play_val != 1) {
			sleep(time_out);
		} else {
			snd_pcm_t *pcm_p;
			unsigned long buf_size;
			if (snd_pcm_open(&pcm_p, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) {
				write_0_to_play_file();
				continue;
			}
			snd_pcm_hw_params_t *pcm_hw = malloc(snd_pcm_hw_params_sizeof());
			snd_pcm_hw_params_any(pcm_p, pcm_hw);

			snd_pcm_hw_params_set_access(pcm_p, pcm_hw,
					SND_PCM_ACCESS_MMAP_INTERLEAVED);
			unsigned int rate = 44100;
			int dir = -1;
			snd_pcm_hw_params_set_rate_near(pcm_p, pcm_hw, &rate, &dir);
			if (snd_pcm_hw_params(pcm_p, pcm_hw)) {
				free(pcm_hw);
				snd_pcm_close(pcm_p);
				write_0_to_play_file();
				continue;
			}

			snd_pcm_hw_params_get_buffer_size(pcm_hw,
					(snd_pcm_uframes_t*) &buf_size);
			free(pcm_hw);

			unsigned int buf[buf_size];

			unsigned long read_size = 0;

			char album_val[1024];
			get_album(album_val);
			for (int i = 1; i < 100; i++) {
				char file_name[2048];
				sprintf(file_name, "%s/%s/%d.wav", music_root, album_val, i);
				FILE *music_file = fopen(file_name, "rb");
				if (music_file) {
					char ch[50];
					fread(ch, 4, 11, music_file);
					long play_err;
					if (read_size < buf_size && read_size > 0) {
						read_size += fread(buf + read_size, 4,
								buf_size - read_size, music_file);
						if (read_size < buf_size) {
							fclose(music_file);
							continue;
						}
						if ((play_err = snd_pcm_mmap_writei(pcm_p, buf,
								(snd_pcm_uframes_t) read_size)) < 0) {
							fclose(music_file);
							write_0_to_play_file();
							break;
						}
					}
					while ((read_size = fread(buf, 4, buf_size, music_file))) {
						if (check_album(album_val) != 0) {
							break;
						}
						if (read_size < buf_size) {
							break;
						}
						if ((play_err = snd_pcm_mmap_writei(pcm_p, buf,
								(snd_pcm_uframes_t) read_size)) < 0) {
							break;
						}
					}
					fclose(music_file);
					if (play_err < 0) {
						write_0_to_play_file();
						break;
					}
					if (check_album(album_val) != 0) {
						break;
					}
				} else {
					if (read_size < buf_size && read_size > 0) {
						if (snd_pcm_mmap_writei(pcm_p, buf,
								(snd_pcm_uframes_t) read_size) >= 0) {
							snd_pcm_drain(pcm_p);
						}
					}
					write_0_to_play_file();
					break;
				}
			}
			snd_pcm_close(pcm_p);
		}
	}
}
