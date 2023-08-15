#include <FLAC/metadata.h>

#define shm_file "/meta_shm_file"
#define picture_path "/tmp/picture"

extern volatile FLAC__uint32 *length;
extern int length_size;
extern volatile FLAC__uint32 *length_half;
extern int length_half_size;
extern char *data_addr;
extern char *data_addr_half;
extern int data_size;

#define shm_size() (getpagesize() * 300)

#define set_shm_vars()                                                   \
    length = shd_addr;                                                   \
    data_addr = (char *)shd_addr + length_size;                          \
    length_half = shd_addr + (shm_size() / 2);                       \
    data_addr_half = (char *)length_half + length_half_size; \
    data_size = shm_size() - length_size;
