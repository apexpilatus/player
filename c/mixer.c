#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>
#include <alsa/control.h>
#include <alsa/mixer.h>

static snd_mixer_elem_t *melem;

static void write_vol_to_file(char * vol){
        int vol_file_dstr;
        if ((vol_file_dstr = open("/home/exe/player/tmp/volume", O_NONBLOCK|O_WRONLY)) != -1) {
                write(vol_file_dstr, vol, 1);
                close(vol_file_dstr);
        }
}

static int get_volume(char *ret){
        int vol_file_dstr;
        if ((vol_file_dstr = open("/home/exe/player/tmp/volume", O_NONBLOCK|O_RDONLY)) == -1){
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
                if ((vol_file_dstr = open("/home/exe/player/tmp/volume", O_CREAT|O_NONBLOCK|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != -1) {
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

int main(int pnum, char * params[]){
	snd_mixer_t *mxr;
	if (snd_mixer_open(&mxr, 0)){
		pause();
	}
	if (snd_mixer_attach(mxr, params[1])){
		snd_mixer_close(mxr);
		pause();
	}
	if (snd_mixer_selem_register(mxr, NULL, NULL)){
		snd_mixer_close(mxr);
		pause();
	}
	if (snd_mixer_load(mxr)){
		snd_mixer_close(mxr);
		pause();
	}
	melem = snd_mixer_first_elem(mxr);
	while(1){
		set_volume();
		sleep(1);
	}
}
