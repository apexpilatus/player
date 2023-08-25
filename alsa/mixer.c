#include "shares.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/pcm.h>
#include <alsa/control.h>
#include <alsa/mixer.h>

static volatile long *target_vol_ptr;
static volatile long *max_vol_ptr;
static char *data_addr;
static int target_vol_size = sizeof(long);
static int max_vol_size = sizeof(long);
long minvol, maxvol;
snd_mixer_elem_t *melem;

void set_volume(int signum)
{
	long curr_vol;
	if (*target_vol_ptr < minvol)
	{
		*target_vol_ptr = minvol;
	}
	if (*target_vol_ptr > maxvol)
	{
		*target_vol_ptr = maxvol;
	}
	snd_mixer_selem_get_playback_volume(melem, -1, &curr_vol);
	if (curr_vol != *target_vol_ptr)
	{
		snd_mixer_selem_set_playback_volume(melem, -1, *target_vol_ptr);
	}
}

int main(void)
{
	int shd = shm_open(shm_file, O_RDWR, 0);
	if (shd < 0)
	{
		*max_vol_ptr = -1;
		return 1;
	}
	void *shd_addr = mmap(NULL, shm_size(), PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED)
	{
		*max_vol_ptr = -1;
		return 1;
	}
	target_vol_ptr = shd_addr;
	max_vol_ptr = target_vol_ptr + 1;
	data_addr = (char *)shd_addr + target_vol_size + max_vol_size;
	snd_mixer_t *mxr;
	if (snd_mixer_open(&mxr, 0))
	{
		*max_vol_ptr = -1;
		return 1;
	}
	if (snd_mixer_attach(mxr, data_addr))
	{
		*max_vol_ptr = -1;
		return 1;
	}
	if (snd_mixer_selem_register(mxr, NULL, NULL))
	{
		*max_vol_ptr = -1;
		return 1;
	}
	if (snd_mixer_load(mxr))
	{
		*max_vol_ptr = -1;
		return 1;
	}
	if (!(melem = snd_mixer_last_elem(mxr)))
	{
		*max_vol_ptr = -1;
		return 1;
	}
	snd_mixer_selem_get_playback_volume_range(melem, &minvol, &maxvol);
	*max_vol_ptr = maxvol;
	signal(SIGUSR1, set_volume);
	while (*max_vol_ptr)
		;
	snd_mixer_close(mxr);
	return 0;
}
