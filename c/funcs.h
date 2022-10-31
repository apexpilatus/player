#define waiter_h

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>
#include <alsa/control.h>
#include <alsa/mixer.h>

#include <FLAC/metadata.h>
#include <FLAC/stream_decoder.h>

#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>

#define time_out 1
#define album_str_len 1024

#define curr_album_env "CURRALB"
#define card_name_env "CARDNAME"
#define rate_env "FLACRATE"
#define sample_size_env "FLACSAMPLE"

#define album_file_path "/home/exe/player/tmp/album"
#define volume_file_path "/home/exe/player/tmp/volume"
#define exec_play_path "/home/exe/player/player"
#define exec_waiter_path "/home/exe/player/waiter"

#define card_name "Wilkins"
#define player_name "player"


typedef struct lst{
  char *name;
  struct lst *next;
} file_lst;

extern file_lst* get_file_lst(char *dirname);
extern void stop_play(void);
extern void get_album(char *ret);
