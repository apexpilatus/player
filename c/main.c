#include "funcs.h"
#include <stdlib.h>
#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control.h>
#include <alsa/pcm.h>
#include <alsa/error.h>

int main() {
	while (1) {
		if (check_play_file() != 1) {
			sleep(time_out);
		} else {
			char album_val[1024];
			get_album(album_val);
			unsigned int rate;
			unsigned short frame_size;
			get_params(album_val, &rate, &frame_size);
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
			int dir = -1;
			snd_pcm_hw_params_set_rate_near(pcm_p, pcm_hw, &rate, &dir);
			snd_pcm_hw_params_set_format(pcm_p, pcm_hw, frame_size == 4 ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_S24_3LE);
			if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p)) {
				snd_pcm_hw_params_free(pcm_hw);
				snd_pcm_close(pcm_p);
				write_0_to_play_file();
				continue;
			}
			snd_pcm_hw_params_get_buffer_size(pcm_hw, (snd_pcm_uframes_t*) &buf_size_in_frames);
			snd_pcm_hw_params_free(pcm_hw);
			unsigned long read_size = 0, buf_size_in_bytes = buf_size_in_frames * frame_size;
			char buf[buf_size_in_bytes];
			for (int i = 1; i < 100; i++) {
				char file_name[2048];
				sprintf(file_name, "%s/%d.wav", album_val, i);
				int music_file_dstr = open(file_name, O_NONBLOCK|O_RDONLY);
				if (music_file_dstr != -1) {
					lseek(music_file_dstr, 16, SEEK_SET);
                                        int format_chank_size;
                                        read(music_file_dstr, &format_chank_size, 4);
                                        lseek(music_file_dstr, format_chank_size + 8, SEEK_CUR);
                                        long play_err;
					if (read_size < buf_size_in_bytes && read_size > 0) {
						read_size += read(music_file_dstr, buf + read_size, buf_size_in_bytes - read_size);
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
						read_size/=frame_size;
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
