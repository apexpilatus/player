#define _GNU_SOURCE

#include "shares.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sched.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>

#include <FLAC/stream_decoder.h>

typedef struct lst
{
	char *name;
	struct lst *next;
} file_lst;

static char *album, *track, *card_name, *buf, *buf_ptr;
static long bytes;
static unsigned int kHz, off;

static file_lst *get_file_lst(char *dirname)
{
	file_lst *main_ptr = malloc(sizeof(file_lst));
	file_lst *cur_ptr = main_ptr;
	cur_ptr->name = NULL;
	cur_ptr->next = NULL;
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dirname);
	if (dp != NULL)
	{
		while ((ep = readdir(dp)))
		{
			if (ep->d_type == DT_REG)
			{
				cur_ptr->name = malloc(strlen(ep->d_name) + 1);
				memcpy(cur_ptr->name, ep->d_name, strlen(ep->d_name) + 1);
				cur_ptr->next = malloc(sizeof(file_lst));
				cur_ptr = cur_ptr->next;
				cur_ptr->next = NULL;
			}
		}
		(void)closedir(dp);
	}
	cur_ptr = main_ptr;
	if (!cur_ptr->next)
	{
		return cur_ptr;
	}
	file_lst *sort_ptr = cur_ptr;
	while (cur_ptr->next->next)
	{
		while (sort_ptr->next->next)
		{
			sort_ptr = sort_ptr->next;
			if (strcmp(cur_ptr->name, sort_ptr->name) > 0)
			{
				char *tmp = cur_ptr->name;
				cur_ptr->name = sort_ptr->name;
				sort_ptr->name = tmp;
			}
		}
		cur_ptr = cur_ptr->next;
		sort_ptr = cur_ptr;
	}
	return main_ptr;
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	buf = buf_ptr;
	for (size_t i = 0; i < frame->header.blocksize; i++)
	{
		buf += off;
		memcpy(buf, buffer[0] + i, bytes);
		buf += bytes;
		buf += off;
		memcpy(buf, buffer[1] + i, bytes);
		buf += bytes;
	}
	if (snd_pcm_mmap_writei((snd_pcm_t *)client_data, buf_ptr, (snd_pcm_uframes_t)frame->header.blocksize) < 0)
	{
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
}

void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
}

static inline int play_album(file_lst *files, FLAC__StreamDecoderWriteCallback write_callback, snd_pcm_t *pcm_p)
{
	FLAC__StreamDecoder *decoder = NULL;
	decoder = FLAC__stream_decoder_new();
	FLAC__stream_decoder_set_md5_checking(decoder, false);
	FLAC__stream_decoder_set_metadata_ignore_all(decoder);
	while (files->next)
	{
		FLAC__StreamDecoderInitStatus init_status;
		init_status = FLAC__stream_decoder_init_file(decoder, files->name, write_callback, metadata_callback, error_callback, pcm_p);
		if (init_status == FLAC__STREAM_DECODER_INIT_STATUS_OK)
		{
			if (!FLAC__stream_decoder_process_until_end_of_stream(decoder))
			{
				return 1;
			}
			FLAC__stream_decoder_finish(decoder);
		}
		else
		{
			return 1;
		}
		files = files->next;
	}
	snd_pcm_drain(pcm_p);
	snd_pcm_close(pcm_p);
	return 0;
}

int main(int argn, char *args[])
{
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(2, &cpu_set);
	sched_setaffinity(getpid(), sizeof(cpu_set), &cpu_set);
	album = args[1];
	if (chdir(album))
	{
		return 1;
	}
	file_lst *files = get_file_lst(".");
	if (!files->next && !files->name)
	{
		return 1;
	}
	snd_pcm_t *pcm_p;
	bytes = strtol(args[2], NULL, 10) / 8;
	off = bytes == 2 ? 0 : 1;
	card_name = args[3];
	if (snd_pcm_open(&pcm_p, card_name, SND_PCM_STREAM_PLAYBACK, 0))
	{
		return 1;
	}
	snd_pcm_hw_params_t *pcm_hw;
	snd_pcm_hw_params_malloc(&pcm_hw);
	snd_pcm_hw_params_any(pcm_p, pcm_hw);
	snd_pcm_hw_params_set_access(pcm_p, pcm_hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	int dir = 0;
	kHz = strtod(args[4], NULL) * 1000;
	snd_pcm_hw_params_set_rate(pcm_p, pcm_hw, kHz, dir);
	snd_pcm_hw_params_set_format(pcm_p, pcm_hw, bytes == 2 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_S32);
	if (snd_pcm_hw_params(pcm_p, pcm_hw) || snd_pcm_prepare(pcm_p))
	{
		return 1;
	}
	track = args[5];
	while (files->next)
	{
		if (!strcmp(files->name, track))
		{
			break;
		}
		files = files->next;
	}
	buf_ptr = malloc(getpagesize() * 1000);
	return play_album(files, write_callback, pcm_p);
}
