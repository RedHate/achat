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
  * De-esser
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  * @param threshold - target threshhold 500 is a nice starting point 0.2f
  * @param reduction - the amount of reduction to apply  0.5f
  * @return 0 on success, an error if greater than 0.
  */
void de_ess(int16_t *buffer, size_t size, float threshold, float reduction);

/**
  * Noise suppression (ironically adds noise if autogained)
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  * @param threshold - target threshhold 500 is a nice starting point
  * @return 0 on success, an error if greater than 0.
  */
void noise_suppress(int16_t *buffer, size_t size, int16_t threshold);

/**
  * Auto gain (This screams so loud without AEC, not implemented)
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  * @param target_peak - target peak signed short 0 - 32767
  * @return 0 on success, an error if greater than 0.
  */
void auto_gain(int16_t *buffer, size_t size, int16_t target_peak);

/**
  * Band pass filtering (BROKEN)
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  */
void simple_voice_bandpass(int16_t *buffer, size_t size);

/**
  * Low pass filtering
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  * @param alpha - alpha for filter
  * @return 0 on success, an error if greater than 0.
  */
void low_pass_filter(int16_t *buffer, size_t size, float alpha);

/**
  * High pass filtering
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  * @param alpha - alpha for filter
  * @return 0 on success, an error if greater than 0.
  */
void high_pass_filter(int16_t *buffer, size_t size, float alpha);
