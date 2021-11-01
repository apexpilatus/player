#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>
#include <alsa/pcm.h>
#include <alsa/error.h>

const int time_out = 1;
const char play_file_path[] = "/home/sd/player/play";
const char music_root[] = "/home/disk/music";
const char album_file_path[] = "/home/sd/player/album";

void write_0_to_play_file() {
	int play_file_dstr;
	while ((play_file_dstr = open(play_file_path, O_NONBLOCK|O_WRONLY)) == -1) {
		sleep(time_out);
	}
	char play_val = 0;
	write(play_file_dstr, &play_val, 1);
	close(play_file_dstr);
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

int main() {
	while (1) {
		int play_file_dstr;
		while ((play_file_dstr = open(play_file_path, O_NONBLOCK|O_RDONLY)) == -1) {
			sleep(time_out);
		}
		char play_val;
		read(play_file_dstr, &play_val, 1);
		close(play_file_dstr);
		if (play_val != 1) {
			sleep(time_out);
		} else {
			snd_pcm_t *pcm_p;
			unsigned long buf_size_in_frames;
			if (snd_pcm_open(&pcm_p, "hw:2,0", SND_PCM_STREAM_PLAYBACK, 0)) {
				write_0_to_play_file();
				continue;
			}
			snd_pcm_hw_params_t *pcm_hw;
			if (snd_pcm_hw_params_malloc(&pcm_hw)){
				snd_pcm_close(pcm_p);
				write_0_to_play_file();
				continue;
			}
			snd_pcm_hw_params_any(pcm_p, pcm_hw);
			snd_pcm_hw_params_set_access(pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
			unsigned int rate = 44100;
			int dir = -1;
			snd_pcm_hw_params_set_rate_near(pcm_p, pcm_hw, &rate, &dir);
			if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p)) {
				snd_pcm_hw_params_free(pcm_hw);
				snd_pcm_close(pcm_p);
				write_0_to_play_file();
				continue;
			}
			snd_pcm_hw_params_get_buffer_size(pcm_hw, (snd_pcm_uframes_t*) &buf_size_in_frames);
			snd_pcm_hw_params_free(pcm_hw);
			unsigned int buf[buf_size_in_frames];
			unsigned long read_size = 0, buf_size_in_bytes = buf_size_in_frames * 4;
			char album_val[1024];
			get_album(album_val);
			for (int i = 1; i < 100; i++) {
				char file_name[2048];
				sprintf(file_name, "%s/%s/%d.wav", music_root, album_val, i);
				int music_file_dstr = open(file_name, O_NONBLOCK|O_RDONLY);
				if (music_file_dstr != -1) {
					char ch[50];
					read(music_file_dstr, ch, 44);
					long play_err;
					if (read_size < buf_size_in_bytes && read_size > 0) {
						read_size += read(music_file_dstr, (char *)buf + read_size, buf_size_in_bytes - read_size);
						if (read_size < buf_size_in_bytes) {
							close(music_file_dstr);
							continue;
						}
						if ((play_err = snd_pcm_mmap_writei(pcm_p, buf, (snd_pcm_uframes_t) buf_size_in_frames)) < 0) {
							close(music_file_dstr);
							write_0_to_play_file();
							break;
						}
					}
					while ((read_size = read(music_file_dstr, buf, buf_size_in_bytes))) {
						if (check_album(album_val) != 0) {
							break;
						}
						if (read_size < buf_size_in_bytes) {
							break;
						}
						if ((play_err = snd_pcm_mmap_writei(pcm_p, buf, (snd_pcm_uframes_t) buf_size_in_frames)) < 0) {
							break;
						}
					}
					close(music_file_dstr);
					if (play_err < 0) {
						write_0_to_play_file();
						break;
					}
					if (check_album(album_val) != 0) {
						break;
					}
				} else {
					if (read_size < buf_size_in_bytes && read_size > 0) {
						read_size/=4;
						if (snd_pcm_mmap_writei(pcm_p, buf, (snd_pcm_uframes_t) read_size) >= 0) {
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
