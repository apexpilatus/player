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

#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>

typedef struct lst
{
	char *name;
	struct lst *next;
} file_lst;

static char *album;
static char *track;
static char *card_name;
static char *data_half;

static int vol_size = sizeof(long) * 2;

static unsigned int rate;
static unsigned short sample_size;
static char *buf0, *buf1;
static void *playbuf[2];
static unsigned char off;

static inline void cp_little_endian(char *buf, char *data, int samplesize)
{
	for (int i = 0; i < samplesize; i++)
	{
		memcpy(buf, data + i, 1);
		buf++;
	}
}

static int get_shared_vars(void)
{
	int shd = shm_open(shm_file, O_RDWR, 0);
	if (shd < 0)
	{
		return 1;
	}
	void *shd_addr = mmap(NULL, shm_size(), PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED)
	{
		return 1;
	}
	album = (char *)shd_addr + vol_size;
	track = album + strlen(album) + 1;
	card_name = track + strlen(track) + 1;
	data_half = (char *)shd_addr + (shm_size() / 2);
	return 0;
}

static int get_params(file_lst *files, unsigned int *rate, unsigned short *sample_size)
{
	file_lst *first_file = files;
	unsigned int rate_1st;
	unsigned short sample_size_1st;
	while (files->next)
	{
		FLAC__StreamMetadata streaminfo;
		if (FLAC__metadata_get_streaminfo(files->name, &streaminfo))
		{
			*rate = streaminfo.data.stream_info.sample_rate;
			*sample_size = streaminfo.data.stream_info.bits_per_sample;
			if (first_file == files)
			{
				rate_1st = *rate;
				sample_size_1st = *sample_size;
			}
			else
			{
				if (rate_1st != *rate || sample_size_1st != *sample_size)
				{
					return 1;
				}
			}
		}
		else
		{
			return 1;
		}
		files = files->next;
	}
	return 0;
}

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

static inline FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	snd_pcm_t *pcm_p = (snd_pcm_t *)client_data;
	int sample_size_bytes = sample_size / 8;
	for (size_t i = 0; i < frame->header.blocksize; i++)
	{
		cp_little_endian(buf0 + off + (i * (sample_size_bytes + off)), (char *)(buffer[0] + i), sample_size_bytes);
		cp_little_endian(buf1 + off + (i * (sample_size_bytes + off)), (char *)(buffer[1] + i), sample_size_bytes);
	}
	if (snd_pcm_mmap_writen(pcm_p, playbuf, (snd_pcm_uframes_t)frame->header.blocksize) < 0)
	{
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static inline void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
}

static inline void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
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

int main(void)
{
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(2, &cpu_set);
	sched_setaffinity(getpid(), sizeof(cpu_set), &cpu_set);
	if (get_shared_vars())
	{
		return 1;
	}
	buf0 = card_name + strlen(card_name) + 1;
	buf1 = data_half;
	playbuf[0] = buf0;
	playbuf[1] = buf1;
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
	off = sample_size == 16 ? 0 : 1;
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
	snd_pcm_hw_params_set_format(pcm_p, pcm_hw, sample_size == 16 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_S32);
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
