#include <stdio.h>
#include <unistd.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>

#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>

#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>

#include "playlib.h"

static int conversion;
static unsigned int rate;
static unsigned int fixed_rate = 96000;
static unsigned short sample_size;
static AVCodec *decode_codec;
static AVCodecContext *decode_context;
static AVCodec *encode_codec;
static AVCodecContext *encode_context;
static AVFrame *ff_frame;
static SwrContext *swr;
static AVPacket *pkt;
static int nb_out_samples;
static uint8_t *ff_output;

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data){
	snd_pcm_t *pcm_p = (snd_pcm_t*)client_data;
	int samplesize = sample_size/8;
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
		double ratio = (double)fixed_rate/rate;
		nb_out_samples = ff_frame->nb_samples * ratio + 32;
		av_samples_alloc(&ff_output, NULL, 2, nb_out_samples, AV_SAMPLE_FMT_S32, 0);
		nb_out_samples = swr_convert(swr, &ff_output, nb_out_samples, (const uint8_t **)ff_frame->data, ff_frame->nb_samples);
		ff_frame->format = encode_context->sample_fmt;
		ff_frame->sample_rate = fixed_rate;
		ff_frame->nb_samples = nb_out_samples;
		ff_frame->data[0] = ff_output;
		avcodec_send_frame(encode_context, ff_frame);
		avcodec_receive_packet(encode_context, pkt);
	}
	if (snd_pcm_mmap_writei(pcm_p, conversion ? pkt->data : playbuf, conversion ? (snd_pcm_uframes_t) nb_out_samples : (snd_pcm_uframes_t) frame->header.blocksize) < 0){
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if (conversion){
		av_freep(&ff_output);
		av_packet_free(&pkt);
		av_frame_free(&ff_frame);
	}
	free(playbuf);
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

int main(int argsn, char *args[]) {
	if (chdir(args[1])) {
		return 1;
	}
	file_lst *files=get_file_lst(args[1]);
	if (!files->next && !files->name){
		return 1;
	}
	if (get_params(files, &rate, &sample_size)){
		return 1;
	}
	conversion = rate != fixed_rate || sample_size != 24;
	if (conversion){
		decode_codec = avcodec_find_decoder_by_name(sample_size == 24 ? "pcm_s24le" : "pcm_s16le");
		decode_context = avcodec_alloc_context3(decode_codec);
		decode_context->channels = 2;
		decode_context->sample_rate = rate;
		avcodec_open2(decode_context, decode_codec, NULL);
		encode_codec = avcodec_find_encoder_by_name("pcm_s24le");
		encode_context = avcodec_alloc_context3(encode_codec);
		encode_context->sample_fmt = AV_SAMPLE_FMT_S32;
		encode_context->channels = 2;
		encode_context->sample_rate = fixed_rate;
		avcodec_open2(encode_context, encode_codec, NULL);
		swr = swr_alloc_set_opts(swr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S32, fixed_rate, AV_CH_LAYOUT_STEREO, sample_size == 24 ? AV_SAMPLE_FMT_S32 : AV_SAMPLE_FMT_S16, rate, 0, NULL);
		swr_init(swr);
	}
	snd_pcm_t *pcm_p;
	if (snd_pcm_open(&pcm_p, args[2], SND_PCM_STREAM_PLAYBACK, 0)) {
		return 1;
	}
	snd_pcm_hw_params_t *pcm_hw;
	snd_pcm_hw_params_malloc(&pcm_hw);
	snd_pcm_hw_params_any(pcm_p, pcm_hw);
	snd_pcm_hw_params_set_access(pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	int dir = 0;
	snd_pcm_hw_params_set_rate(pcm_p, pcm_hw, fixed_rate, dir);
	snd_pcm_hw_params_set_format(pcm_p, pcm_hw, SND_PCM_FORMAT_S24_3LE);
	if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p)) {
		return 1;
	}
	while (files->next) {
		if(!strcmp(files->name, args[3])){
			break;
		}
		files=files->next;
	}
	return play_album(files, write_callback, pcm_p);
}
