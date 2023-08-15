#include "shares.h"

volatile FLAC__uint32 *length;
int length_size = sizeof(FLAC__uint32);
volatile FLAC__uint32 *length_half;
int length_half_size = sizeof(FLAC__uint32);
char *data_addr;
char *data_addr_half;
int data_size;
