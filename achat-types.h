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

#include "achat-cdefs.h"

// Microphone
typedef struct micinfo{
	float  gain;  // controlled with ',' and '.' keys
	int    busy;     // var for microphone ptt lockout when receiving
	int    monitor;     // var for toggling microphone monitor with 'm'
	int    key_up;     // toggle with spacebar
	time_t key_up_start;     // used to count time for mic timeout
	time_t busy_start;       // timer for microphone ptt lockout when receiving
}micinfo;


/**
* not implemented yet, still debating if i want usernames.
*/
typedef struct payload{
	char     *username;  // username buffer 16 byte names max
	uint8_t  opus[FRAME_SIZE];        // opus data
	uint32_t opus_size;               // size of opus data in bytes
}payload;

