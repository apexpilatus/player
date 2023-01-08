#include "funcs.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

void get_file_content(char *file, char *ret) {
	int album_file_dstr;
	if ((album_file_dstr = open(file, O_NONBLOCK|O_RDONLY)) != -1) {
		ssize_t size = read(album_file_dstr, ret, album_str_len);
		ret[size] = 0;
		close(album_file_dstr);
	}
}
