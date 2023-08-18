#include "shares.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>

#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>

#include "playlib.h"

char *album;
char *track;
char *card_name;
void *buf0;
void *buf1;

static int vol_size = sizeof(long) * 2;

void cp_little_endian(unsigned char *buf, FLAC__uint32 data, int samplesize)
{
	for (int i = 0; i < samplesize; i++)
	{
		*buf = data >> (8 * i);
		buf++;
	}
}

int get_shared_vars(void)
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
	buf0 = card_name + strlen(card_name) + 1;
	buf1 = shd_addr + (shm_size() / 2);
	return 0;
}

file_lst *get_file_lst(char *dirname)
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

int get_params(file_lst *files, unsigned int *rate, unsigned short *sample_size)
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

static void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
}

static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
}

int play_album(file_lst *files, FLAC__StreamDecoderWriteCallback write_callback, snd_pcm_t *pcm_p)
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
	return 0;
}
