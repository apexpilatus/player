#ifndef waiter_h
	#include "funcs.h"
#endif

void write_0_to_play_file() {
	int play_file_dstr;
	while ((play_file_dstr = open(album_file_path, O_NONBLOCK|O_WRONLY)) == -1) {
		sleep(time_out);
	}
	char play_val = 0;
	write(play_file_dstr, &play_val, 1);
	close(play_file_dstr);
}

char check_play_file(){
        int play_file_dstr;
        while ((play_file_dstr = open(album_file_path, O_NONBLOCK|O_RDONLY)) == -1) {
                sleep(time_out);
        }
        char play_val;
        read(play_file_dstr, &play_val, 1);
        close(play_file_dstr);
        return play_val;
}

void get_album(char *ret) {
	int album_file_dstr;
	while ((album_file_dstr = open(album_file_path, O_NONBLOCK|O_RDONLY)) == -1) {
		sleep(time_out);
	}
	ssize_t size = read(album_file_dstr, ret, 1024);
	ret[size] = 0;
	close(album_file_dstr);
}

int check_album(char current[]) {
	char next[1024];
	get_album(next);
	return strcmp(current, next);
}
extern file_lst* get_file_lst(char *dirname){
	file_lst *main_ptr = malloc(sizeof(file_lst));
	file_lst *cur_ptr = main_ptr;
	cur_ptr->name=NULL;
	cur_ptr->next=NULL;

	DIR *dp;
	struct dirent *ep;
	
	dp = opendir(dirname);
	if (dp != NULL) {
		while ((ep = readdir(dp))) {
			if (ep->d_type == DT_REG) {
				cur_ptr->name=malloc(strlen(ep->d_name)+1);
				memcpy(cur_ptr->name, ep->d_name, strlen(ep->d_name)+1);
				cur_ptr->next=malloc(sizeof(file_lst));
				cur_ptr=cur_ptr->next;
				cur_ptr->next=NULL;
			}
		}
		(void) closedir(dp);
	}
	cur_ptr=main_ptr;
	if (!cur_ptr->next){
		return cur_ptr;
	}
	file_lst *sort_ptr = cur_ptr;
	while (cur_ptr->next->next) {
		while(sort_ptr->next->next) {
			sort_ptr=sort_ptr->next;
			if (strcmp(cur_ptr->name, sort_ptr->name)>0){
				char *tmp=cur_ptr->name;
				cur_ptr->name=sort_ptr->name;
				sort_ptr->name=tmp;
			}
		}
		cur_ptr=cur_ptr->next;
		sort_ptr = cur_ptr;
	}
	return main_ptr;
}

int get_params(char *album_val, file_lst *files, unsigned int *rate, unsigned short *frame_size) {
	char file_name[2048];
	file_lst *first_file=files;
	unsigned int rate_1st;
	unsigned short frame_size_1st;
	while (files->next) {
		sprintf(file_name, "%s/%s", album_val, files->name);
		FLAC__StreamMetadata streaminfo;
		if (FLAC__metadata_get_streaminfo(file_name, &streaminfo)) {
			*rate = streaminfo.data.stream_info.sample_rate;
			*frame_size = streaminfo.data.stream_info.bits_per_sample/4;
			if (first_file == files) {
				rate_1st = *rate;
				frame_size_1st = *frame_size;
			} else {
				if (rate_1st != *rate || frame_size_1st != *frame_size) {
					return 1;
				}
			}
		} else {
			return 1;
		}
		files=files->next;
	}
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data){
	return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
}

void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data){
}

void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data){
}
