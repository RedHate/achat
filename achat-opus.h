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
  * Opus decoder encoder init
  */
int init_opus();

/**
  * Opus decoder encoder shutdown
  */
void deinit_opus();

/**
  * Encode pcm buffer to opus buffer
  *
  * @param pcm - input buffer
  * @param frame_size - size in bytes (use framesize not sizeof)
  * @param data - opus buffer
  * @param max_size - size in bytes (use framesize not sizeof)
  * @return 0 on success, an error if greater than 0.
  */
int opus_encode_buffer(const short *pcm_buf, int frame_size, uint8_t *opus_buf, uint32_t max_size);

/**
  * Decode opus buffer to 16bit pcm buffer
  *
  * @param opus_buf - input buffer
  * @param len - size of opus packed buffer (must be known ahead of time but sizeof(buffer) should work)
  * @param pcm_buf - opus buffer
  * @param frame_size - size in bytes (use framesize not sizeof)
  * @return 0 on success, an error if greater than 0.
  */
int opus_decode_buffer(const uint8_t *opus_buf, uint32_t len, short *pcm_buf, int frame_size);	
