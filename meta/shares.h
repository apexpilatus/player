#include <FLAC/metadata.h>

#define shm_file "/meta_shm_file"
#define picture_path "/tmp/picture"

extern volatile FLAC__uint32 *length;
extern int length_size;
extern FLAC__uint32 *length_internal;
extern int length_internal_size;
extern char *data_addr;
extern char *data_addr_internal;
extern int data_size;

#define shm_size() (getpagesize() * 300)

#define set_shm_vars() \
length = shd_addr; \
data_addr = (char *)shd_addr + length_size; \
length_internal = shd_addr + (shm_size() / 2); \
data_addr_internal = (char *)length_internal + length_internal_size; \
data_size = shm_size() - length_size;
