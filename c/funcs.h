#define waiter_h

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>

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

#define curr_album_env "curalb"
#define card_name_env "cardname"
#define rate_env "flacrate"
#define sample_size_env "flacsample"

#define album_file_path "/home/exe/player/tmp/album"
#define track_file_path "/home/exe/player/tmp/track"
#define volume_file_path "/home/exe/player/tmp/volume"

#define exec_player_path "/home/exe/player/player"
#define exec_waiter_path "/home/exe/player/waiter"
#define exec_mixer_path "/home/exe/player/mixer"

#define card_name "Wilkins"
#define player_name "player"
#define waiter_name "waiter"
#define mixer_name "mixer"


typedef struct lst{
  char *name;
  struct lst *next;
} file_lst;

extern void get_file_content(char *file, char *ret);
extern char play_next(void);
extern file_lst* get_file_lst(char *dirname);
