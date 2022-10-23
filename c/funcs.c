#ifndef waiter_h
	#include "funcs.h"
#endif

void stop_play(void) {
	int play_file_dstr;
	if ((play_file_dstr = open(album_file_path, O_NONBLOCK|O_WRONLY)) != -1) {
		char play_val = 0;
		write(play_file_dstr, &play_val, 1);
		close(play_file_dstr);
	}
}

char check_play(void){
	int play_file_dstr;
	if ((play_file_dstr = open(album_file_path, O_NONBLOCK|O_RDONLY)) != -1) {
		char play_val;
		read(play_file_dstr, &play_val, 1);
		close(play_file_dstr);
		return play_val;
	}
	return 0;
}

void get_album(char *ret) {
	int album_file_dstr;
	if ((album_file_dstr = open(album_file_path, O_NONBLOCK|O_RDONLY)) != -1) {
		ssize_t size = read(album_file_dstr, ret, album_str_len);
		ret[size] = 0;
		close(album_file_dstr);
	}
}

int check_album(void) {
	char next[album_str_len];
	get_album(next);
	return strcmp(next, getenv(curr_album_env));
}

static int get_volume(char *ret){
	int vol_file_dstr;
	if ((vol_file_dstr = open(volume_file_path, O_NONBLOCK|O_RDONLY)) == -1){
		return 1;
	}
	read(vol_file_dstr, ret, 1);
	close(vol_file_dstr);
	return 0;
}

static void write_vol_to_file(char * vol){
	int vol_file_dstr;
	if ((vol_file_dstr = open(volume_file_path, O_CREAT|O_NONBLOCK|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != -1) {
		write(vol_file_dstr, vol, 1);
		close(vol_file_dstr);
	}
}

static void set_volume(void){
	char newvol;
	long vol, minvol, maxvol;
	if (get_volume(&newvol)){
		newvol = 3;
		write_vol_to_file(&newvol);
	}
	snd_mixer_t *mxr;
	if (!snd_mixer_open(&mxr, 0)){
		if (snd_mixer_attach(mxr, getenv(card_name_env))){
			snd_mixer_close(mxr);
		} else {
			if (snd_mixer_selem_register(mxr, NULL, NULL)){
				printf("cannot register simple mixer\n");
				snd_mixer_close(mxr);
			} else {
				if (snd_mixer_load(mxr)){
					printf("cannot load elements\n");
					snd_mixer_close(mxr);
				} else {
					snd_mixer_elem_t *melem = snd_mixer_first_elem(mxr);
					snd_mixer_selem_get_playback_volume_range(melem, &minvol, &maxvol);
					if (newvol < minvol){
						newvol = minvol;
						write_vol_to_file(&newvol);
					}
					if (newvol > maxvol){
						newvol = maxvol;
						write_vol_to_file(&newvol);
					}
					snd_mixer_selem_get_playback_volume(melem, -1, &vol);
					if (vol != newvol){
						snd_mixer_selem_set_playback_volume(melem, -1, newvol);
					}
					snd_mixer_close(mxr);
				}
			}
		}
	}
}

file_lst* get_file_lst(char *dirname){
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

int get_params(char *album_val, file_lst *files, unsigned int *rate, unsigned short *sample_size) {
	char file_name[2048];
	file_lst *first_file=files;
	unsigned int rate_1st;
	unsigned short sample_size_1st;
	while (files->next) {
		sprintf(file_name, "%s/%s", album_val, files->name);
		FLAC__StreamMetadata streaminfo;
		if (FLAC__metadata_get_streaminfo(file_name, &streaminfo)) {
			*rate = streaminfo.data.stream_info.sample_rate;
			*sample_size = streaminfo.data.stream_info.bits_per_sample;
			if (first_file == files) {
				rate_1st = *rate;
				sample_size_1st = *sample_size;
			} else {
				if (rate_1st != *rate || sample_size_1st != *sample_size) {
					return 1;
				}
			}
		} else {
			return 1;
		}
		files=files->next;
	}
}

void cp_little_endian(char *buf, FLAC__uint32 data, int samplesize)
{
	for (int i=0;i<samplesize;i++){
		*buf = data >> (8*i);
		buf++;
	}
}

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data){
	if (check_album()){
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	set_volume();
	snd_pcm_t *pcm_p = (snd_pcm_t*)client_data;
	snd_pcm_hw_params_t *pcm_hw;
	if (snd_pcm_hw_params_malloc(&pcm_hw) || snd_pcm_hw_params_current(pcm_p, pcm_hw)){
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	int samplesize=snd_pcm_hw_params_get_sbits(pcm_hw)/8;
	snd_pcm_hw_params_free(pcm_hw);
	char * playbuf = malloc(samplesize*2*frame->header.blocksize);
	for(size_t i = 0; i < frame->header.blocksize; i++) {
		cp_little_endian(playbuf+(i*samplesize*2), buffer[0][i], samplesize);
		cp_little_endian(playbuf+(i*samplesize*2)+samplesize, buffer[1][i], samplesize);
	}
	if (snd_pcm_mmap_writei(pcm_p, playbuf, (snd_pcm_uframes_t) frame->header.blocksize) < 0){
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	free(playbuf);
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data){
}

void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data){
}
