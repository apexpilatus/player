#include <stdio.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>

#include <FLAC/stream_decoder.h>

#include "playlib.h"

static void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data){
}

static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data){
}

int play_album(file_lst *files, FLAC__StreamDecoderWriteStatus (*write_callback)(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data), snd_pcm_t *pcm_p) {
	FLAC__StreamDecoder *decoder = NULL;
	decoder = FLAC__stream_decoder_new();
	FLAC__stream_decoder_set_md5_checking(decoder, false);
	FLAC__stream_decoder_set_metadata_ignore_all(decoder);
	while (files->next) {
		FLAC__StreamDecoderInitStatus init_status;
		init_status = FLAC__stream_decoder_init_file(decoder, files->name, write_callback, metadata_callback, error_callback, pcm_p);
		if(init_status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
			if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)){
				return 1;
			}
			FLAC__stream_decoder_finish(decoder);
		} else {
			return 1;
		}
		files=files->next;
	}
	snd_pcm_drain(pcm_p);
	return 0;
}
