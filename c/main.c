#ifndef waiter_h
	#include "funcs.h"
#endif

static int conversion;
static AVCodec *decode_codec;
static AVCodecContext *decode_context;
static AVCodec *encode_codec;
static AVCodecContext *encode_context;
static AVFrame *ff_frame;
static SwrContext *swr;
static AVPacket *pkt;
static int nb_out_samples;
static uint8_t *ff_output;
static snd_mixer_elem_t *melem;

static void write_vol_to_file(char * vol){
        int vol_file_dstr;
        if ((vol_file_dstr = open(volume_file_path, O_NONBLOCK|O_WRONLY)) != -1) {
                write(vol_file_dstr, vol, 1);
                close(vol_file_dstr);
        }
}

static int get_volume(char *ret){
        int vol_file_dstr;
        if ((vol_file_dstr = open(volume_file_path, O_NONBLOCK|O_RDONLY)) == -1){
                return 1;
        }
        read(vol_file_dstr, ret, 1);
        close(vol_file_dstr);
        return 0;
}

static void set_volume(void){
        char newvol;
        long vol, minvol, maxvol;
        if (get_volume(&newvol)){
                newvol = 5;
                int vol_file_dstr;
                if ((vol_file_dstr = open(volume_file_path, O_CREAT|O_NONBLOCK|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != -1) {
                        write(vol_file_dstr, &newvol, 1);
                        close(vol_file_dstr);
                }
        }
	snd_mixer_selem_get_playback_volume_range(melem, &minvol, &maxvol);
	if (newvol < minvol){
		newvol = minvol;
		write_vol_to_file(&newvol);
	}
	if (newvol > maxvol){
		newvol = maxvol;
		write_vol_to_file(&newvol);
	}
	snd_mixer_selem_get_playback_volume(melem, -1, &vol);
	if (vol != newvol){
		snd_mixer_selem_set_playback_volume(melem, -1, newvol);
	}
}


static void cp_little_endian(unsigned char *buf, FLAC__uint32 data, int samplesize){
        for (int i=0;i<samplesize;i++){
                *buf = data >> (8*i);
                buf++;
        }
}

static void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data){
}

static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data){
}



static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data){
	if (check_album()){
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	snd_pcm_t *pcm_p = (snd_pcm_t*)client_data;
	int samplesize = 3;
	int bufsize = samplesize*2*frame->header.blocksize;
	unsigned char * playbuf = malloc(bufsize);
	for(size_t i = 0; i < frame->header.blocksize; i++) {
		cp_little_endian(playbuf+(i*samplesize*2), buffer[0][i], samplesize);
		cp_little_endian(playbuf+(i*samplesize*2)+samplesize, buffer[1][i], samplesize);
	}
	if (conversion){
		pkt = av_packet_alloc();
		pkt->data = playbuf;
		pkt->size = bufsize;
		ff_frame = av_frame_alloc();
		avcodec_send_packet(decode_context, pkt);
		avcodec_receive_frame(decode_context, ff_frame);
		double ratio = (double)96000/atoi(getenv(rate_env));
		nb_out_samples = ff_frame->nb_samples * ratio + 32;
		av_samples_alloc(&ff_output, NULL, 2, nb_out_samples, AV_SAMPLE_FMT_S32, 0);
		nb_out_samples = swr_convert(swr, &ff_output, nb_out_samples, ff_frame->data, ff_frame->nb_samples);
		ff_frame->format = encode_context->sample_fmt;
		ff_frame->sample_rate = 96000;
		ff_frame->nb_samples = nb_out_samples;
		ff_frame->data[0] = ff_output;
		avcodec_send_frame(encode_context, ff_frame);
		avcodec_receive_packet(encode_context, pkt);
	}
	if (snd_pcm_mmap_writei(pcm_p, conversion ? pkt->data : playbuf, conversion ? (snd_pcm_uframes_t) nb_out_samples : (snd_pcm_uframes_t) frame->header.blocksize) < 0){
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	set_volume();
	if (conversion){
		av_freep(&ff_output);
		av_packet_free(&pkt);
		av_frame_free(&ff_frame);
	}
	free(playbuf);
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void main(void) {
	conversion = atoi(getenv(rate_env)) != 96000 || atoi(getenv(sample_size_env)) != 24;
	if (conversion){
		decode_codec = avcodec_find_decoder_by_name("pcm_s24le");
		decode_context = avcodec_alloc_context3(decode_codec);
		decode_context->channels = 2;
		decode_context->sample_rate = atoi(getenv(rate_env));
		avcodec_open2(decode_context, decode_codec, NULL);
		encode_codec = avcodec_find_encoder_by_name("pcm_s24le");
		encode_context = avcodec_alloc_context3(encode_codec);
		encode_context->sample_fmt = AV_SAMPLE_FMT_S32;
		encode_context->channels = 2;
		encode_context->sample_rate = 96000;
		avcodec_open2(encode_context, encode_codec, NULL);
		swr = swr_alloc_set_opts(swr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S32, 96000, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S32, atoi(getenv(rate_env)), 0, NULL);
		swr_init(swr);
	}
	char *card_pcm_name = malloc(10);
	strcpy(card_pcm_name, getenv(card_name_env));
	strcpy(card_pcm_name + strlen(card_pcm_name),",0");
	FLAC__StreamDecoder *decoder = NULL;
	decoder = FLAC__stream_decoder_new();
	FLAC__stream_decoder_set_md5_checking(decoder, true);
	FLAC__stream_decoder_set_metadata_ignore_all(decoder);
	snd_pcm_t *pcm_p;
	if (snd_pcm_open(&pcm_p, card_pcm_name, SND_PCM_STREAM_PLAYBACK, 0)) {
		stop_play();
		execl(exec_waiter_path, player_name, "cannot open pcm", NULL);
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
		execl(exec_waiter_path, player_name, "cannot start playing", NULL);
	}
	snd_mixer_t *mxr;
	if (snd_mixer_open(&mxr, 0)){
		stop_play();
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, player_name, "cannot open mixer", NULL);
	}
	if (snd_mixer_attach(mxr, getenv(card_name_env))){
		stop_play();
		snd_mixer_close(mxr);
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, player_name, "cannot attach mixer", NULL);
	}
	if (snd_mixer_selem_register(mxr, NULL, NULL)){
		stop_play();
		snd_mixer_close(mxr);
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, player_name, "cannot register simple elem", NULL);
	}
	if (snd_mixer_load(mxr)){
		stop_play();
		snd_mixer_close(mxr);
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, player_name, "cannot load mixer", NULL);
	}
	melem = snd_mixer_first_elem(mxr);
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
				snd_mixer_close(mxr);
				snd_pcm_close(pcm_p);
				FLAC__StreamDecoderState dec_state = FLAC__stream_decoder_get_state(decoder);
				FLAC__stream_decoder_finish(decoder);
				FLAC__stream_decoder_delete(decoder);
				execl(exec_waiter_path, player_name, "error during playing", files->name, FLAC__StreamDecoderStateString[dec_state], NULL);
			}
			FLAC__stream_decoder_finish(decoder);
		} else {
			stop_play();
			snd_mixer_close(mxr);
			snd_pcm_close(pcm_p);
			FLAC__stream_decoder_delete(decoder);
			execl(exec_waiter_path, player_name, "cannot init file", files->name, FLAC__StreamDecoderInitStatusString[init_status], NULL);
		}
		files=files->next;
	}
	snd_pcm_drain(pcm_p);
	stop_play();
	snd_mixer_close(mxr);
	snd_pcm_close(pcm_p);
	FLAC__stream_decoder_delete(decoder);
	execl(exec_waiter_path, player_name, "the end", NULL);
}
