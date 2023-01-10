#include "shares.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>
#include <alsa/control.h>
#include <alsa/mixer.h>

static snd_mixer_elem_t *melem;
static int ready;
void * shd_addr;

static void set_volume(void){
	long curr_vol, minvol, maxvol;
	snd_mixer_selem_get_playback_volume_range(melem, &minvol, &maxvol);
	if (*(char*)shd_addr < minvol) {
		*(char*)shd_addr = minvol;
	}
	if (*(char*)shd_addr > maxvol) {
		*(char*)shd_addr = maxvol;
	}
	snd_mixer_selem_get_playback_volume(melem, -1, &curr_vol);
	if (curr_vol != *(char*)shd_addr){
		snd_mixer_selem_set_playback_volume(melem, -1, *(char*)shd_addr);
	}
}

void usr_handle() {
	if (ready) {
		set_volume();
	}
}

int main(int pnum, char * params[]){
	signal(SIGUSR1, usr_handle);
	int shd = shm_open(shm_file, O_RDWR, S_IRUSR|S_IWUSR);
	if (shd < 0) {
		pause();
	}
	int page_size = getpagesize();
	shd_addr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED){
		pause();
	}
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
	ready++;
	while(1){
		pause();
	}
}
