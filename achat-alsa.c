/*
    
    This file is part of achat

    achat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    achat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with achat; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


    Copyright 2026 Ultros (RedHate)
    https://github.com/redhate
    
*/


#include <stdint.h>
#include <alsa/asoundlib.h>
#include "achat-cdefs.h"

int init_alsa(snd_pcm_t **handle, const char *device, snd_pcm_stream_t stream, uint32_t channels) {
    
    // Local vars
    snd_pcm_hw_params_t *params;
    int err;
    
    // Init a handle for the PCM steam from a chosen device
    if ((err = snd_pcm_open(handle, device, stream, 0)) < 0) {
        fprintf(stderr, "ALSA open error: %s\n", snd_strerror(err));
        return 1;
    }
	
	// I assume this allocates memory for params, return doesn't allow error checking
	snd_pcm_hw_params_alloca(&params);
	
	// Init all channel params and error check
    if ((err = snd_pcm_hw_params_any(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_any error: %s\n", snd_strerror(err));
		return 2;
	}
	
	// (ignore that gcc throws warning on this or fix it lol)
	// set resample since we want to adjust the rate 
	uint32_t resample = 1;
    if ((err = snd_pcm_hw_params_set_rate_resample(*handle, params, *(unsigned int*)&resample)) < 0) {
        //fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate_resample error: %s\n", snd_strerror(err));
		return 3;
	}
	// Set access
    if ((err = snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_access error: %s\n", snd_strerror(err));
		return 4;
	}
	// Set format
    if ((err = snd_pcm_hw_params_set_format(*handle, params, FORMAT)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_format error: %s\n", snd_strerror(err));
		return 5;
	}
	
	// Set channels
    snd_pcm_hw_params_set_channels(*handle, params, channels);
    
	// Set rate
	uint32_t sample_rate = SAMPLE_RATE;
    if ((err = snd_pcm_hw_params_set_rate_near(*handle, params, &sample_rate, 0)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate error: %s\n", snd_strerror(err));
		return 6;
	}
	// Set hw params
    if ((err = snd_pcm_hw_params(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params error: %s\n", snd_strerror(err));
		return 7;
	}
	// Prepare for pcm
    if ((err = snd_pcm_prepare(*handle)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_prepare error: %s\n", snd_strerror(err));
		return 8;
	}

	// Print message
    printf("ALSA initialized at %u Hz for %d\n", sample_rate, stream);

    return 0;
    
}

int deinit_alsa(snd_pcm_t **handle) {
	// Local var
	int err;
	// Destroy handle
	if ((err = snd_pcm_close(*handle)) < 0) {
		return 1;
	}
	return 0;
}

void device_list(snd_pcm_stream_t stream) {
	
	// Local vars
	snd_ctl_t *handle;
	int dev;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);

	int card = -1;
	if (snd_card_next(&card) < 0 || card < 0) {
		fprintf(stderr, "no soundcards found...\n");
		return;
	}
	
	printf(("\n------ List of %s Hardware Devices ------\n"), snd_pcm_stream_name(stream));
	
	// loop through the devices
	while (card >= 0) {
		
		// Local vars
		int err;
		char name[32];
		sprintf(name, "hw:%d", card);
		dev = -1;
		
		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
			fprintf(stderr, "control open (%i): %s", card, snd_strerror(err));
			goto next_card;
		}
		
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
			fprintf(stderr, "control hardware info (%i): %s", card, snd_strerror(err));
			snd_ctl_close(handle);
			goto next_card;
		}

		while (1) {
			
			if (snd_ctl_pcm_next_device(handle, &dev) < 0)
				fprintf(stderr, "snd_ctl_pcm_next_device\n");
			
			if (dev < 0)
				break;
			
			printf(("plug:hw:%i:%i - %s\n"), card,  dev, snd_ctl_card_info_get_id(info));

		}
		
		snd_ctl_close(handle);
		
		next_card:

		if (snd_card_next(&card) < 0) {
			fprintf(stderr, "snd_card_next");
			break;
		}
		
	}
	
}

