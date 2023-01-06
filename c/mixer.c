static snd_mixer_elem_t *melem;

static int get_volume(char *ret){
        int vol_file_dstr;
        if ((vol_file_dstr = open(volume_file_path, O_NONBLOCK|O_RDONLY)) == -1){
                return 1;
        }
        read(vol_file_dstr, ret, 1);
        close(vol_file_dstr);
        return 0;
}

static void set_volume(void){
        char newvol;
        long vol, minvol, maxvol;
        if (get_volume(&newvol)){
                newvol = 5;
                int vol_file_dstr;
                if ((vol_file_dstr = open(volume_file_path, O_CREAT|O_NONBLOCK|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != -1) {
                        write(vol_file_dstr, &newvol, 1);
                        close(vol_file_dstr);
                }
        }
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
}

int main(void){
	snd_mixer_t *mxr;
	if (snd_mixer_open(&mxr, 0)){
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, waiter_name, "cannot open mixer", NULL);
	}
	if (snd_mixer_attach(mxr, getenv(card_name_env))){
		snd_mixer_close(mxr);
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, waiter_name, "cannot attach mixer", NULL);
	}
	if (snd_mixer_selem_register(mxr, NULL, NULL)){
		snd_mixer_close(mxr);
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, waiter_name, "cannot register simple elem", NULL);
	}
	if (snd_mixer_load(mxr)){
		snd_mixer_close(mxr);
		snd_pcm_close(pcm_p);
		FLAC__stream_decoder_delete(decoder);
		execl(exec_waiter_path, waiter_name, "cannot load mixer", NULL);
	}
	melem = snd_mixer_first_elem(mxr);
	while(1){
		set_volume();
		sleep(1);
	}
}
