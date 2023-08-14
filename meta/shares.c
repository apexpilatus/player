#include "shares.h"

volatile FLAC__uint32 *picture_length;
int picture_length_size = sizeof(FLAC__uint32);
volatile FLAC__uint32 *num_comments;
int num_comments_size = sizeof(FLAC__uint32);
char *data_addr;
int data_size;
