#include "shares.h"

volatile FLAC__uint32 *length;
int length_size = sizeof(FLAC__uint32);
FLAC__uint32 *length_internal;
int length_internal_size = sizeof(FLAC__uint32);
char *data_addr;
char *data_addr_internal;
int data_size;
