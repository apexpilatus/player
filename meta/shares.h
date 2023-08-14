#include <FLAC/metadata.h>

#define shm_file "/meta_shm_file"
#define picture_path "/tmp/picture"

extern volatile FLAC__uint32 *length;
extern int length_size;
extern char *data_addr;
extern int data_size;

#define shm_size() (getpagesize() * 300)

#define set_shm_vars() \
length = shd_addr; \
data_addr = (char *)shd_addr + length_size; \
data_size = shm_size() - length_size;
