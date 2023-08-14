#include "shares.h"

volatile FLAC__uint32 *length;
int length_size = sizeof(FLAC__uint32);
char *data_addr;
char *data_addr_internal;
int data_size;
