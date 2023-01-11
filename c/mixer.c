#include "shares.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>
#include <alsa/control.h>
#include <alsa/mixer.h>

static void set_volume(snd_mixer_elem_t *melem, char *shd_addr){
	long curr_vol, minvol, maxvol;
	snd_mixer_selem_get_playback_volume_range(melem, &minvol, &maxvol);
	if (*shd_addr < minvol) {
		*shd_addr = minvol;
	}
	if (*shd_addr > maxvol) {
		*shd_addr = maxvol;
	}
	snd_mixer_selem_get_playback_volume(melem, -1, &curr_vol);
	if (curr_vol != *shd_addr){
		snd_mixer_selem_set_playback_volume(melem, -1, *shd_addr);
	}
}

int main(int pnum, char * params[]){
	int shd = shm_open(shm_file, O_RDWR, S_IRUSR|S_IWUSR);
	if (shd < 0) {
		return 1;
	}
	int page_size = getpagesize();
	void *shd_addr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED){
		return 1;
	}
	snd_mixer_t *mxr;
	if (snd_mixer_open(&mxr, 0)){
		return 1;
	}
	if (snd_mixer_attach(mxr, params[1])){
		return 1;
	}
	if (snd_mixer_selem_register(mxr, NULL, NULL)){
		return 1;
	}
	if (snd_mixer_load(mxr)){
		return 1;
	}
	snd_mixer_elem_t *melem = snd_mixer_first_elem(mxr);
	set_volume(melem, shd_addr);
	return 0;
}
