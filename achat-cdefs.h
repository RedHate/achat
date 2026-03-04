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


// Uncomment to enable debugging // comment to turn off
#define _DEBUG_ 1
#define  _XOR_  1

// Alsa related definitions
#define SAMPLE_RATE 48000 //libopus expects this, it will resample for you if you tell it to.
#define CHANNELS    1     //single channel
#define FRAME_SIZE  960   //libopus expects this or a multiple or division of it
#define FORMAT      SND_PCM_FORMAT_S16_LE //libopus expects 16bit audio
#define MIC_TIMEOUT_SECONDS 60
//#define MAX_CLIENTS  FD_SETSIZE
