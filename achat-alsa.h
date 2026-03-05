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

/**
  * Alsa handle init
  * 
  * @param handle - Alsa device handle pointer
  * @param device - Device name eq: plug:hw:1 or default
  * @param stream - SND_PCM_STREAM_CAPTURE or SND_PCM_STREAM_PLAYBACK
  * @param channels - Amount of channels
  * @return 0 on success, an error if greater than 0.
  */
int init_alsa(snd_pcm_t **handle, const char *device, snd_pcm_stream_t stream, uint32_t channels);

/**
  * Alsa handle deinit
  * 
  * @param handle - Alsa device handle pointer
  * @return 0 on success, an error if greater than 0.
  */
int deinit_alsa(snd_pcm_t **handle);

/**
  * List all devices
  * 
  * @param stream - SND_PCM_STREAM_CAPTURE or SND_PCM_STREAM_PLAYBACK
  */
void device_list(snd_pcm_stream_t stream);
