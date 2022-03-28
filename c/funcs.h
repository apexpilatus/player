#define waiter_h

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>
#include <alsa/control.h>

#define time_out 1
#define album_file_path "/home/store/player/album"
#define exec_play_path "/home/store/player/a.out"
#define exec_waiter_path "/home/store/player/play.waiter"

extern void write_0_to_play_file();
extern int check_play_file();
extern void get_album(char *ret);
extern int check_album(char current[]);
extern int get_params(char *album_val, unsigned int *rate, unsigned short *frame_size);
