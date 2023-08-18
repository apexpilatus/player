#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>

#include <FLAC/stream_decoder.h>

#include "playlib.h"

static unsigned int rate;
static unsigned short sample_size;
extern unsigned char *buf0;
extern unsigned char *buf1;
void *playbuf[2];

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	snd_pcm_t *pcm_p = (snd_pcm_t *)client_data;
	int sample_size_bytes = sample_size / 8;
	for (size_t i = 0; i < frame->header.blocksize; i++)
	{
		cp_little_endian(buf0 + (i * sample_size_bytes), buffer[0][i], sample_size_bytes);
		cp_little_endian(buf1 + (i * sample_size_bytes), buffer[1][i], sample_size_bytes);
	}
	if (snd_pcm_mmap_writen(pcm_p, playbuf, (snd_pcm_uframes_t)frame->header.blocksize) < 0)
	{
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

int main(void)
{
	if (get_shared_vars())
	{
		return 1;
	}
	playbuf[0] = buf0;
	playbuf[1] = buf1;
	extern char *album;
	extern char *track;
	extern char *card_name;
	if (chdir(album))
	{
		return 1;
	}
	file_lst *files = get_file_lst(".");
	if (!files->next && !files->name)
	{
		return 1;
	}
	if (get_params(files, &rate, &sample_size))
	{
		return 1;
	}
	snd_pcm_t *pcm_p;
	if (snd_pcm_open(&pcm_p, card_name, SND_PCM_STREAM_PLAYBACK, 0))
	{
		return 1;
	}
	snd_pcm_hw_params_t *pcm_hw;
	snd_pcm_hw_params_malloc(&pcm_hw);
	snd_pcm_hw_params_any(pcm_p, pcm_hw);
	snd_pcm_hw_params_set_access(pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
	int dir = 0;
	snd_pcm_hw_params_set_rate(pcm_p, pcm_hw, rate, dir);
	snd_pcm_hw_params_set_format(pcm_p, pcm_hw, sample_size == 16 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_S24_3LE);
	if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p))
	{
		return 1;
	}
	while (files->next)
	{
		if (!strcmp(files->name, track))
		{
			break;
		}
		files = files->next;
	}
	return play_album(files, write_callback, pcm_p);
}
