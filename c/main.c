#ifndef waiter_h
	#include "funcs.h"
#endif

int main(int argsn, char *args[]) {
	int rate = atoi(args[2]), sample_size = atoi(args[3]);
	snd_pcm_t *pcm_p;
	FLAC__StreamDecoder *decoder = NULL;
	if((decoder = FLAC__stream_decoder_new()) == NULL) {
		write_0_to_album_file();
		execl(exec_waiter_path, "play.waiter", "cannot allocate decoder", NULL);
	}
	(void)FLAC__stream_decoder_set_md5_checking(decoder, true);
	(void)FLAC__stream_decoder_set_metadata_ignore_all(decoder);
	if (snd_pcm_open(&pcm_p, args[1], SND_PCM_STREAM_PLAYBACK, 0)) {
		write_0_to_album_file();
		execl(exec_waiter_path, "play.waiter", "cannot open pcm", NULL);
	}
	snd_pcm_hw_params_t *pcm_hw;
	if (snd_pcm_hw_params_malloc(&pcm_hw)){
		write_0_to_album_file();
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, "play.waiter", "cannot allocate memory for hw params", NULL);
	}
	snd_pcm_hw_params_any(pcm_p, pcm_hw);
	snd_pcm_hw_params_set_access(pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	int dir = -1;
	snd_pcm_hw_params_set_rate_near(pcm_p, pcm_hw, &rate, &dir);
	if (sample_size == 24){
		snd_pcm_hw_params_set_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S24_3LE);
	}
	if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p)) {
		write_0_to_album_file();
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, "play.waiter", "cannot start playing", NULL);
	}
	snd_pcm_hw_params_free(pcm_hw);
	file_lst *files=get_file_lst(args[4]);
	while (files->next) {
		char file_name[2048];
		sprintf(file_name, "%s/%s", args[4], files->name);
		FLAC__StreamDecoderInitStatus init_status;
		init_status = FLAC__stream_decoder_init_file(decoder, file_name, write_callback, metadata_callback, error_callback, pcm_p);
		if(init_status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
			if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)){
				if (!check_album()){
					write_0_to_album_file();
				}
				snd_pcm_close(pcm_p);
				FLAC__StreamDecoderState dec_state = FLAC__stream_decoder_get_state(decoder);
				FLAC__stream_decoder_finish(decoder);
				FLAC__stream_decoder_delete(decoder);
				execl(exec_waiter_path, "play.waiter", "error during playing", files->name, FLAC__StreamDecoderStateString[dec_state], NULL);
			}
			FLAC__stream_decoder_finish(decoder);
		} else {
			write_0_to_album_file();
			snd_pcm_close(pcm_p);
			FLAC__stream_decoder_delete(decoder);
			execl(exec_waiter_path, "play.waiter", "cannot init file", files->name, FLAC__StreamDecoderInitStatusString[init_status], NULL);
		}
		files=files->next;
	}
	snd_pcm_drain(pcm_p);
	write_0_to_album_file();
	snd_pcm_close(pcm_p);
	FLAC__stream_decoder_delete(decoder);
	execl(exec_waiter_path, "play.waiter", "the end", NULL);
}
