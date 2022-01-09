#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

extern const int time_out;
extern const char play_file_path[];
extern const char album_file_path[];
extern const char exec_play_path[];
extern const char exec_waiter_path[];

extern void write_0_to_play_file();
extern int check_play_file();
extern void get_album(char *ret);
extern int check_album(char current[]);
extern void get_params(char *album_val, unsigned int *rate, unsigned short *frame_size);
