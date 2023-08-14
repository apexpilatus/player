#include "shares.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(void)
{
	int shd = shm_open(shm_file, O_RDWR, 0);
	if (shd < 0)
	{
		return 1;
	}
	int page_size = getpagesize();
	void *shd_addr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED)
	{
		return 1;
	}
    set_shm_addr();
	FLAC__StreamMetadata *tags = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
	if (!FLAC__metadata_get_tags(data_addr, &tags))
	{
		return 1;
	}
	*num_comments = tags->data.vorbis_comment.num_comments;
	char *str = data_addr;
	for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++)
	{
		str = str + strlen(str) + 1;
		strcpy(str, tags->data.vorbis_comment.comments[i].entry);
	}
	str = str + strlen(str) + 1;
	FLAC__StreamMetadata *rate = FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
	if (!FLAC__metadata_get_streaminfo(data_addr, rate))
	{
		return 1;
	}
	sprintf(str, "RATE=%u/%g", rate->data.stream_info.bits_per_sample, rate->data.stream_info.sample_rate / 1000.0);
	return 0;
}
