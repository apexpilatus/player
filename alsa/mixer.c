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

static long *target_vol_addr;
static long *max_vol_addr;
static char *data_addr;
static int target_vol_size = sizeof(long);
static int max_vol_size = sizeof(long);

static void set_volume(snd_mixer_elem_t *melem)
{
	long curr_vol, minvol;
	snd_mixer_selem_get_playback_volume_range(melem, &minvol, max_vol_addr);
	if (*target_vol_addr < minvol)
	{
		*target_vol_addr = minvol;
	}
	if (*target_vol_addr > *max_vol_addr)
	{
		*target_vol_addr = *max_vol_addr;
	}
	snd_mixer_selem_get_playback_volume(melem, -1, &curr_vol);
	if (curr_vol != *target_vol_addr)
	{
		snd_mixer_selem_set_playback_volume(melem, -1, *target_vol_addr);
	}
}

int main(void)
{
	int shd = shm_open(shm_file, O_RDWR, 0);
	if (shd < 0)
	{
		return 1;
	}
	int page_size = getpagesize();
	void *shd_addr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
	if (shd_addr == MAP_FAILED)
	{
		return 1;
	}
	target_vol_addr = shd_addr;
	max_vol_addr = target_vol_addr + 1;
	data_addr = (char *)shd_addr + target_vol_size + max_vol_size;
	snd_mixer_t *mxr;
	if (snd_mixer_open(&mxr, 0))
	{
		return 1;
	}
	if (snd_mixer_attach(mxr, data_addr))
	{
		return 1;
	}
	if (snd_mixer_selem_register(mxr, NULL, NULL))
	{
		return 1;
	}
	if (snd_mixer_load(mxr))
	{
		return 1;
	}
	snd_mixer_elem_t *melem = snd_mixer_last_elem(mxr);
	set_volume(melem);
	return 0;
}