#ifndef waiter_h
	#include "funcs.h"
#endif

void main(void) {
	char *card_pcm_name = malloc(10);
	strcpy(card_pcm_name, getenv(card_name_env));
	strcpy(card_pcm_name + strlen(card_pcm_name),",0");
	snd_pcm_t *pcm_p;
	FLAC__StreamDecoder *decoder = NULL;
	decoder = FLAC__stream_decoder_new();
	FLAC__stream_decoder_set_md5_checking(decoder, true);
	FLAC__stream_decoder_set_metadata_ignore_all(decoder);
	if (snd_pcm_open(&pcm_p, card_pcm_name, SND_PCM_STREAM_PLAYBACK, 0)) {
		stop_play();
		execl(exec_waiter_path, "play.waiter", "cannot open pcm", NULL);
	}
	snd_pcm_hw_params_t *pcm_hw;
	snd_pcm_hw_params_malloc(&pcm_hw);
	snd_pcm_hw_params_any(pcm_p, pcm_hw);
	snd_pcm_hw_params_set_access(pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	int dir = 0;
	snd_pcm_hw_params_set_rate(pcm_p, pcm_hw, 96000, dir);
	snd_pcm_hw_params_set_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S24_3LE);
	if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p)) {
		stop_play();
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, "play.waiter", "cannot start playing", NULL);
	}
	snd_pcm_hw_params_free(pcm_hw);
	file_lst *files=get_file_lst(getenv(curr_album_env));
	while (files->next) {
		char file_name[album_str_len + 50];
		sprintf(file_name, "%s/%s", getenv(curr_album_env), files->name);
		FLAC__StreamDecoderInitStatus init_status;
		init_status = FLAC__stream_decoder_init_file(decoder, file_name, write_callback, metadata_callback, error_callback, pcm_p);
		if(init_status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
			if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)){
				if (!check_album()){
					stop_play();
				}
				snd_pcm_close(pcm_p);
				FLAC__StreamDecoderState dec_state = FLAC__stream_decoder_get_state(decoder);
				FLAC__stream_decoder_finish(decoder);
				FLAC__stream_decoder_delete(decoder);
				execl(exec_waiter_path, "play.waiter", "error during playing", files->name, FLAC__StreamDecoderStateString[dec_state], NULL);
			}
			FLAC__stream_decoder_finish(decoder);
		} else {
			stop_play();
			snd_pcm_close(pcm_p);
			FLAC__stream_decoder_delete(decoder);
			execl(exec_waiter_path, "play.waiter", "cannot init file", files->name, FLAC__StreamDecoderInitStatusString[init_status], NULL);
		}
		files=files->next;
	}
	snd_pcm_drain(pcm_p);
	stop_play();
	snd_pcm_close(pcm_p);
	FLAC__stream_decoder_delete(decoder);
	execl(exec_waiter_path, "play.waiter", "the end", NULL);
}
