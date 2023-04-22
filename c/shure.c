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

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	snd_pcm_t *pcm_p = (snd_pcm_t *)client_data;
	int samplesize = sample_size / 8;
	int bufsize = samplesize * 2 * frame->header.blocksize;
	unsigned char *playbuf = malloc(bufsize);
	for (size_t i = 0; i < frame->header.blocksize; i++)
	{
		cp_little_endian(playbuf + (i * samplesize * 2), buffer[0][i], samplesize);
		cp_little_endian(playbuf + (i * samplesize * 2) + samplesize, buffer[1][i], samplesize);
	}
	if (snd_pcm_mmap_writei(pcm_p, playbuf, (snd_pcm_uframes_t)frame->header.blocksize) < 0)
	{
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	free(playbuf);
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

int main(int argsn, char *args[])
{
	if (chdir(args[1]))
	{
		return 1;
	}
	file_lst *files = get_file_lst(args[1]);
	if (!files->next && !files->name)
	{
		return 1;
	}
	if (get_params(files, &rate, &sample_size))
	{
		return 1;
	}
	snd_pcm_t *pcm_p;
	if (snd_pcm_open(&pcm_p, args[2], SND_PCM_STREAM_PLAYBACK, 0))
	{
		return 1;
	}
	snd_pcm_hw_params_t *pcm_hw;
	snd_pcm_hw_params_malloc(&pcm_hw);
	snd_pcm_hw_params_any(pcm_p, pcm_hw);
	snd_pcm_hw_params_set_access(pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	int dir = 0;
	snd_pcm_hw_params_set_rate(pcm_p, pcm_hw, rate, dir);
	snd_pcm_hw_params_set_format(pcm_p, pcm_hw, sample_size == 16 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_S24_3LE);
	if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p))
	{
		return 1;
	}
	while (files->next)
	{
		if (!strcmp(files->name, args[3]))
		{
			break;
		}
		files = files->next;
	}
	return play_album(files, write_callback, pcm_p);
}
