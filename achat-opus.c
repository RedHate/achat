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

#include <opus/opus.h>
#include <stdio.h>
#include "achat-cdefs.h"

// Opus handles
OpusDecoder *decoder;
OpusEncoder *encoder;

// Opus decoder encoder init
int init_opus() {
    
    int err;
    
	// ENCODER
    // Create Opus encoder
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_AUDIO, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "Failed to create encoder: %s\n", opus_strerror(err));
        return 1;
    }

    // Set the lowest acceptable bitrate for high compression
    opus_encoder_ctl(encoder, OPUS_SET_APPLICATION(OPUS_APPLICATION_VOIP));
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(8000)); // mono voice
    opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10)); // optional, max compression efficiency
    opus_encoder_ctl(encoder, OPUS_SET_VBR(1));
	opus_encoder_ctl(encoder, OPUS_SET_VBR_CONSTRAINT(1));
	
	// probably dont need this (seems to be set by the above voip option)
	opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_NARROWBAND));

	// DECODER
	// Create Opus decoder
	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
	if (err != OPUS_OK) {
		fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(err));
		return 1;
	}


    return 0;
}

// Opus decoder encoder shutdown
void deinit_opus(){
	opus_encoder_destroy(encoder);
	opus_decoder_destroy(decoder);
}

// Encode a pcm stream to opus
int opus_encode_buffer(const short *pcm_buf, int frame_size, uint8_t *opus_buf, uint32_t max_size){
    
    //unsigned char opus_data[4000]; // buffer for encoded data
    int bytes_encoded = opus_encode(encoder, pcm_buf, frame_size, opus_buf, max_size);
    if (bytes_encoded < 0) {
        fprintf(stderr, "Encode failed: %s\n", opus_strerror(bytes_encoded));
        return 1;
    }

    //printf("Encoded %d bytes\n", bytes_encoded);
    return bytes_encoded;
    
}

// Decode an opus stream to pcm
int opus_decode_buffer(const uint8_t *opus_buf, uint32_t len, short *pcm_buf, int frame_size){

	int bytes_deccoded = opus_decode(decoder, opus_buf, len, pcm_buf, frame_size, 0);
	if (bytes_deccoded < 0) {
		fprintf(stderr, "Decode failed: %s\n", opus_strerror(bytes_deccoded));
		return 1;
	}
	
    //printf("Decoded %d bytes\n", bytes_deccoded);
    return bytes_deccoded;
}
