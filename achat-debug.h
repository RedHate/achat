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
    along with Foobar; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


    Copyright 2026 Ultros (RedHate)
    https://github.com/redhate
    
*/

/**
  * Debug print buffer byte for byte as hex
  * 
  * @param buffer    - buffer to be printed
  * @param size      - size in bytes
  */
void debug_print_buf(uint8_t *buffer, uint32_t size);

/**
  * Draw a waterfall audio wave effect to stdout
  * 
  * @param buffer     - buffer to be printed
  * @param size       - size in bytes
  * @param color_mode - 0 or 1
  */
void audio_visualiser(short *buffer, size_t size, int color_mode);

