#include <FLAC/metadata.h>

#define shm_file "/meta_shm_file"
#define picture_path "/tmp/picture"

extern volatile FLAC__uint32 *picture_length;
extern int picture_length_size;
extern volatile FLAC__uint32 *num_comments;
extern int num_comments_size;
extern char *data_addr;
extern int data_size;

#define set_shm_addr() \
picture_length = shd_addr; \
num_comments = picture_length + 1; \
data_addr = (char *)shd_addr + picture_length_size + num_comments_size; \
data_size = page_size - picture_length_size - num_comments_size;
