#ifndef waiter_h
	#include "funcs.h"
#endif

int main(int argsn, char *args[]) {
	unsigned long buf_size_in_frames;
	int rate = atoi(args[2]), frame_size = atoi(args[3]);
	snd_pcm_t *pcm_p;
	FLAC__StreamDecoder *decoder = NULL;
	if((decoder = FLAC__stream_decoder_new()) == NULL) {
		write_0_to_play_file();
		execl(exec_waiter_path, "play.waiter", "cannot allocate decoder", NULL);
	}
	(void)FLAC__stream_decoder_set_md5_checking(decoder, false);
	(void)FLAC__stream_decoder_set_metadata_ignore_all(decoder);
	if (snd_pcm_open(&pcm_p, args[1], SND_PCM_STREAM_PLAYBACK, 0)) {
		write_0_to_play_file();
		execl(exec_waiter_path, "play.waiter", "cannot open pcm", NULL);
	}
	snd_pcm_hw_params_t *pcm_hw;
	if (snd_pcm_hw_params_malloc(&pcm_hw)){
		write_0_to_play_file();
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, "play.waiter", "cannot allocate memory for hw params", NULL);
	}
	snd_pcm_hw_params_any(pcm_p, pcm_hw);
	snd_pcm_hw_params_set_access(pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	int dir = -1;
	snd_pcm_hw_params_set_rate_near(pcm_p, pcm_hw, &rate, &dir);
	snd_pcm_hw_params_set_format(pcm_p, pcm_hw, frame_size == 4 ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_S24_3LE);
	if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p)) {
		write_0_to_play_file();
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, "play.waiter", "cannot start playing", NULL);
	}
	snd_pcm_hw_params_get_buffer_size(pcm_hw, (snd_pcm_uframes_t*) &buf_size_in_frames);
	snd_pcm_hw_params_free(pcm_hw);
	unsigned long read_size = 0, buf_size_in_bytes = buf_size_in_frames * frame_size;
	char buf[buf_size_in_bytes];
	file_lst *files=get_file_lst(args[4]);
	while (files->next) {
		char file_name[2048];
		sprintf(file_name, "%s/%s", args[4], files->name);
		
		/*int music_file_dstr = open(file_name, O_NONBLOCK|O_RDONLY);
		if (music_file_dstr != -1) {
			lseek(music_file_dstr, 16, SEEK_SET);
			int format_chank_size;
			read(music_file_dstr, &format_chank_size, 4);
			lseek(music_file_dstr, format_chank_size + 8, SEEK_CUR);
			long play_err;
			while ((read_size = read(music_file_dstr, buf, buf_size_in_bytes))) {
				if (check_album(args[4]) != 0) {
					break;
				}
				if ((play_err = snd_pcm_mmap_writei(pcm_p, buf, (snd_pcm_uframes_t) read_size/frame_size)) < 0) {
					break;
				}
			}
			close(music_file_dstr);
			if (play_err < 0) {
				write_0_to_play_file();
				snd_pcm_close(pcm_p);
				FLAC__stream_decoder_delete(decoder);
				execl(exec_waiter_path, "play.waiter", "play error", NULL);
			}
			if (check_album(args[4]) != 0) {
				snd_pcm_close(pcm_p);
				FLAC__stream_decoder_delete(decoder);
				execl(exec_waiter_path, "play.waiter", "new album", NULL);
			}*/
		FLAC__StreamDecoderInitStatus init_status;
		init_status = FLAC__stream_decoder_init_file(decoder, file_name, write_callback, metadata_callback, error_callback, pcm_p);
		if(init_status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
			if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)){
				write_0_to_play_file();
				snd_pcm_close(pcm_p);
				FLAC__stream_decoder_finish(decoder);
				FLAC__StreamDecoderState dec_state = FLAC__stream_decoder_get_state(decoder);
				FLAC__stream_decoder_delete(decoder);
				execl(exec_waiter_path, "play.waiter", "play file", files->name, FLAC__StreamDecoderStateString[dec_state], NULL);
			}
			FLAC__stream_decoder_finish(decoder);
		} else {
			write_0_to_play_file();
			snd_pcm_close(pcm_p);
			FLAC__stream_decoder_delete(decoder);
			execl(exec_waiter_path, "play.waiter", "cannot init file", files->name, FLAC__StreamDecoderInitStatusString[init_status], NULL);
		}
		files=files->next;
	}
	snd_pcm_drain(pcm_p);
	write_0_to_play_file();
	snd_pcm_close(pcm_p);
	FLAC__stream_decoder_delete(decoder);
	execl(exec_waiter_path, "play.waiter", "the end", NULL);
}
