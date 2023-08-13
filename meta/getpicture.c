#include "shares.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <FLAC/metadata.h>

static FLAC__uint32 *length;
static int length_size = sizeof(FLAC__uint32);
static char *data_addr;
static int data_size;

int main(void)
{
	printf("fuck0\n");
	int shd = shm_open(shm_file, O_RDWR, 0);
	if (shd < 0)
	{
		printf("fuck1\n");
		return 1;
	}
	int page_size = getpagesize();
	void *shd_addr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED)
	{
		printf("fuck2\n");
		return 1;
	}
	length = shd_addr;
	data_addr = (char *)shd_addr + length_size;
	data_size = page_size - length_size;
	FLAC__StreamMetadata *picture = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
	printf("%s\n", data_addr);
	FLAC__bool ffkk = FLAC__metadata_get_picture(data_addr, &picture, FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER, NULL, NULL, (uint32_t)(-1), (uint32_t)(-1), (uint32_t)(-1), (uint32_t)(-1));
	printf("%d\n", ffkk);
	*length = picture->data.picture.data_length;
	FILE *fl = fopen(picture_path, "w");
	if (fl)
	{
		fwrite(picture->data.picture.data, 1, *length, fl);
		fclose(fl);
	}
	else
	{
		printf("fuck3\n");
		return 1;
	}
	return 0;
}
