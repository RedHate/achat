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

#include <stdio.h>
#include <stdint.h>

void debug_print_buf(uint8_t *buffer, uint32_t size) {
	int c;
	for(c=0;c<size;c++) {
		printf("%02hX,", *(uint8_t*)&buffer[c]);
	}
	printf("\n");
}

void audio_visualiser(short *buffer, size_t size, int color_mode) {
	
	/*
	 * This is honestly one of the coolest things I have ever made
	 * such a simple thing and looks so neat
	 */
	
	// Visualiser array
	char visualiser_array[32][32]={
		"+",
		"++",
		"+++",
		"++++",
		"+++++",
		"++++++",
		"+++++++",
		"++++++++",
		"+++++++++",
		"++++++++++",
		"+++++++++++",
		"++++++++++++",
		"+++++++++++++",
		"++++++++++++++",
		"+++++++++++++++",
		"++++++++++++++++",
		"+++++++++++++++++",
		"++++++++++++++++++",
		"+++++++++++++++++++",
		"++++++++++++++++++++",
		"+++++++++++++++++++++",
		"++++++++++++++++++++++",
		"+++++++++++++++++++++++",
		"++++++++++++++++++++++++",
		"+++++++++++++++++++++++++",
		"++++++++++++++++++++++++++",
		"+++++++++++++++++++++++++++",
		"++++++++++++++++++++++++++++",
		"+++++++++++++++++++++++++++++",
		"++++++++++++++++++++++++++++++",
		"+++++++++++++++++++++++++++++++",
		"++++++++++++++++++++++++++++++++",
	};
	
	// Terminal color strings
	char color_strings[8][10]={
		"\033[1;37m",//WHITE
		"\033[1;34m",//BLUE
		"\033[1;36m",//CYAN
		"\033[1;32m",//GREEN
		"\033[1;33m",//YELLOW
		"\033[1;31m",//RED
		"\033[1;35m",//MAGENTA
		"\033[1;37m",//WHITE1
		//"\033[0m", //NOCOLOR
	};
	
	// Main visualiser loop
	int c;
	for(c=0;c<sizeof(buffer);c++) {
		// Type cast and reduce to 8 bit
		char byte = (char)(buffer[c]/255);
		// Check upper and lower signal boundaries
		if((byte >= 0) && (byte <= 127)) {
				// Print the pretty stuff
				printf("[%04d]%s                     | %s\033[0m\n", *(char*)&byte, (color_mode)? color_strings[6] : color_strings[((int)(byte/8))/2],   visualiser_array[(int)(byte/8)]);
		}
		// Check upper and lower signal boundaries
		if((byte <= 0) && (byte >= -127)) {
				// Print the pretty stuff
				printf("[%04d]%s%20s |\033[0m\n", *(char*)&byte, (color_mode)? color_strings[2] : color_strings[((int)(~byte/6))/2], visualiser_array[(int)(~byte/8)]);
		}
	}
			
}

