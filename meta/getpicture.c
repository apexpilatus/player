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
	void *shd_addr = mmap(NULL, shm_size(), PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED)
	{
		return 1;
	}
	set_shm_vars();
	FLAC__StreamMetadata *picture = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
	if (!FLAC__metadata_get_picture(data_addr, &picture, -1, NULL, NULL, (uint32_t)(-1), (uint32_t)(-1), (uint32_t)(-1), (uint32_t)(-1)))
	{
		return 1;
	}
	*length = picture->data.picture.data_length;
	memcpy(data_addr, picture->data.picture.data, picture->data.picture.data_length);
	return 0;
}
