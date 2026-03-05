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

/* I wont lie ai gave me all this crap lol */

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "achat-cdefs.h"
#include "achat-filters.h"

/*
 * --------------- SIGNAL TURD POLISHING ---------------
 */

// doesnt need exposed to header
// Function to calculate the energy of a frame
float calculate_energy(short *buffer, int size) {
    float sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += buffer[i] * buffer[i];
    }
    return sum;
}

// Basic voice activation detection function
int voice_activation_detection(short *buffer, int size, float threshold) {
	
/*
    // USAGE:
    
	// Detect speech
	if (voice_activation_detection(pcm, FRAME_SIZE, 0.05f)) {
		printf("Speech detected.\n");
	} else {
		printf("Silence.\n");
	}
*/
	
	// get the energy of the frame
    float energy = calculate_energy(buffer, size);
    
    // debug it
    //printf("not shit, energy! %f\n", energy);
    
    // check threshold
    if (energy > (1000000000.0f * threshold)) {
        return 1; // Speech detected
    } else {
        return 0; // No speech
    }
}

// Simple high-frequency detection and reduction
void de_ess(int16_t *buffer, size_t size, float threshold, float reduction) {
    
    // For simplicity, this example applies a basic high-frequency emphasis reduction
    // based on amplitude. A real de-esser would analyze frequency content,
    // e.g., via FFT, and reduce only sibilant frequencies.

    for (size_t i = 0; i < size; i++) {
        float sample = buffer[i];

        // Basic detection: if sample amplitude exceeds threshold
        if (fabsf(sample) / 32768.0f > threshold) {
            // Apply reduction
            sample *= reduction;
        }

        // Clip to int16 range
        if (sample > 32767.0f) sample = 32767.0f;
        if (sample < -32768.0f) sample = -32768.0f;

        buffer[i] = (int16_t)sample;
    }
}

// Noise suppression based on threshold
void noise_suppress(int16_t *buffer, size_t size, int16_t threshold) {
    for (size_t i = 0; i < size; i++) {
        if (abs(buffer[i]) < threshold) {
            buffer[i] = 0; // Suppress (mute) noise below threshold
        }
        // Else, leave the sample unchanged
    }
}

/* --------------- GAIN --------------- */

// AGC function: adjusts gain while avoiding clipping
// buffer: input/output buffer
// size: number of samples
// target_rms: desired RMS level (e.g., 10000.0)
// max_gain: maximum gain factor to prevent over-amplification
void agc(int16_t *buffer, size_t size, float target_rms, float max_gain) {
    
    // Calculate current RMS
    double sum_squares = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum_squares += buffer[i] * buffer[i];
    }
    double rms = sqrt(sum_squares / size);

    // Determine gain factor
    float gain = 1.0f;
    if (rms > 0.0) {
        gain = target_rms / rms;
        if (gain > max_gain) {
            gain = max_gain;
        }
    }

    // Apply gain with clipping prevention
    for (size_t i = 0; i < size; i++) {
        int32_t sample = (int32_t)(buffer[i] * gain);
        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;
        buffer[i] = (int16_t)sample;
    }
}

// manual gain
void manual_gain(int16_t *buffer, size_t size, float threshold){

	// Adjust mic gain before modulation
	int c;
	for (c = 0; c < size; c++) {
		buffer[c] = (short)(buffer[c] * threshold);
	}

}

/* --------------- FILTERING --------------- */

// Basic biquad filter with fixed coefficients for voice range at 16kHz (NEEDS WORK)
void band_pass_filter(int16_t *buffer, size_t size) {
    // Coefficients for a simple bandpass filter (approximate)
    // These are example coefficients; for best results, generate proper ones.
    const double b0 = 0.0675;
    const double b1 = 0.0;    // Using a simplified filter, so set to 0
    const double b2 = -0.0675;
    const double a1 = -1.1429;
    const double a2 = 0.9565;

    double z1 = 0.0;
    double z2 = 0.0;

    for (size_t i = 0; i < size; i++) {
        double input = buffer[i];

        // Basic filtering
        double output = b0 * input + z1;

        // Update states
        z1 = b1 * input + z2 - a1 * output;
        z2 = b2 * input - a2 * output;

        // Clip output to int16 range
        if (output > 32767.0) output = 32767.0;
        if (output < -32768.0) output = -32768.0;

        buffer[i] = (int16_t)output;
    }
}

// Low-pass filter
void low_pass_filter(int16_t *buffer, size_t size, float alpha) {
    float prev_output = 0.0f;
    for (int i = 0; i < size; i++) {
        float current_input = (float)buffer[i];
        float filtered = alpha * current_input + (1 - alpha) * prev_output;
        buffer[i] = (int16_t)filtered;
        prev_output = filtered;
    }
}

// High-pass filter
void high_pass_filter(int16_t *buffer, size_t size, float alpha) {
    static float prev_input = 0.0f;
    static float prev_output = 0.0f;
    for (int i = 0; i < size; i++) {
        float current_input = (float)buffer[i];
        float filtered = alpha * (prev_output + current_input - prev_input);
        buffer[i] = (int16_t)filtered;
        prev_input = current_input;
        prev_output = filtered;
    }
}

/* --------------- DISTORTION --------------- */

// Function to apply distortion to a buffer of 16-bit PCM samples
void distortion(int16_t *buffer, size_t size) {
	
	#define CLIP_THRESHOLD 30000  // Threshold for clipping, less than max 32767
	
    for (size_t i = 0; i < size; i++) {
        int32_t sample = buffer[i];

        // Soft clipping
        if (sample > CLIP_THRESHOLD) {
            sample = CLIP_THRESHOLD + (sample - CLIP_THRESHOLD) / 2;
        } else if (sample < -CLIP_THRESHOLD) {
            sample = -CLIP_THRESHOLD + (sample + CLIP_THRESHOLD) / 2;
        }

        // Clamp to 16-bit range
        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;

        buffer[i] = (int16_t)sample;
    }
}

// doesnt need exposed to header
// Diode clipper function
float process_diode_clip(float sample, float threshold) {
    if (sample > threshold) {
        // Hard clipping at positive threshold
        return threshold;
    } else if (sample < -threshold) {
        // Hard clipping at negative threshold
        return -threshold;
    } else {
        // Soft clipping region (using a smooth approximation)
        // You can also use other nonlinear functions here
        return sample;
    }
}

// Apply diode clipping to buffer
void diode_clipper(int16_t *buffer, size_t size, float threshold) {
	
/*
    // USAGE
    
    float threshold = 0.06f;

    // Apply diode clipper
    diode_clipper(audio_buffer, size, threshold);    
*/
	
    for (size_t i = 0; i < size; i++) {
        float sample = (float)buffer[i] / 32768.0f;
        // Apply diode clipper
        float clipped = process_diode_clip(sample, threshold);
        // Convert back to int16_t
        buffer[i] = (int16_t)(clipped * 32767);
    }
}

/* --------------- PITCH SHIFTING --------------- */

// Naive pitch shifter: resample audio to change pitch
void pitch_shift(int16_t *buffer, size_t size, float pitch_factor) {
    
/*
    // USAGE
    
    float pitch_factor = 1.5f; // shift pitch up by 1.5x
    pitch_shift(input_buffer, output_buffer, input_length, pitch_factor);
*/
    
    // pitch_factor > 1.0 = higher pitch, < 1.0 = lower pitch
    float index = 0.0f;
    float step = 1.0f / pitch_factor; // determines how fast we move through input

    for (size_t i = 0; i < size; i++) {
        size_t idx = (size_t)index;
        float frac = index - idx;

        if (idx + 1 < size) {
            // Linear interpolation
            float sample = (1 - frac) * buffer[idx] + frac * buffer[idx + 1];
            // Clamp to 16-bit range
            if (sample > 32767) sample = 32767;
            if (sample < -32768) sample = -32768;
            buffer[i] = (int16_t)sample;
        } else {
            buffer[i] = 0; // zero padding at the end
        }
        index += step;
    }
}

/* --------------- BIT REDUCTION --------------- */

// Function to apply bit reduction
void bit_reducer(int16_t *buffer, size_t size, int bits_to_retain) {
    // Ensure bits_to_retain is between 1 and 16
    if (bits_to_retain < 1) bits_to_retain = 1;
    if (bits_to_retain > 16) bits_to_retain = 16;

    // Calculate the mask for the bits to keep
    uint16_t mask = ((1 << bits_to_retain) - 1) << (16 - bits_to_retain);

    for (size_t i = 0; i < size; i++) {
        int16_t sample = buffer[i];
        // Convert to unsigned for masking
        uint16_t unsigned_sample = (uint16_t)sample + 32768;
        // Apply mask to reduce bits
        unsigned_sample &= mask;
        // Convert back to signed
        buffer[i] = (int16_t)(unsigned_sample - 32768);
    }
}

// doesnt need exposed to header
// Convert dB to linear scale
float db_to_linear(float db) {
    return powf(10.0f, db / 20.0f);
}

// doesnt need exposed to header
// Convert linear scale to dB
float linear_to_db(float linear) {
    return 20.0f * log10f(linear);
}

// doesnt need exposed to header
// RMS or peak detection could be used; here, we'll use a simple level detector
float detect_level(int16_t sample) {
    return fabsf((float)sample / 32768.0f);
}

// Noise gate parameters
typedef struct {
    float threshold;      // Threshold level (0.0 to 1.0)
    float attack_time;    // Attack time in seconds
    float release_time;   // Release time in seconds
    float sample_rate;    // Samples per second
} noise_gate_params_t;

// Apply noise gate
void noise_gate(int16_t *buffer, size_t size, noise_gate_params_t *params) {
    
/*
    // USAGE
    
    // Set noise gate parameters
    noise_gate_params_t params = {
        .threshold = 0.05f,     // Threshold at 5% of max amplitude
        .attack_time = 0.001f,  // 1 ms attack
        .release_time = 0.05f, // 50 ms release
        .sample_rate = 44100.0f
    };

    // Apply noise gate
    apply_noise_gate(audio_buffer, size, &params);
*/
    
    float attack_coeff = expf(-1.0f / (params->sample_rate * params->attack_time));
    float release_coeff = expf(-1.0f / (params->sample_rate * params->release_time));
    float gain = 1.0f;

    for (size_t i = 0; i < size; i++) {
        
        float level = detect_level(buffer[i]);
        float target_gain = (level < params->threshold) ? 0.0f : 1.0f;

        // Smooth gain with attack/release
        if (target_gain < gain) {
            gain = attack_coeff * (gain - target_gain) + target_gain;
        } else {
            gain = release_coeff * (gain - target_gain) + target_gain;
        }

        // Apply gain
        float sample = (float)buffer[i] / 32768.0f;
        sample *= gain;

        // Clamp
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;

        buffer[i] = (int16_t)(sample * 32767);
    }
}

/* --------------- VCA AUDIO COMPRESSOR --------------- */

// Apply compressor
void vca_compression(int16_t *buffer, size_t size, vca_compressor_params_t *params) {
    
/*
    // USAGE
    
    // Set compressor parameters
    vca_compressor_params_t params = {
        .threshold = -10.0f,   // Threshold at -10 dB
        .ratio = 4.0f,         // 4:1 ratio
        .attack_time = 0.01f,  // 10 ms attack
        .release_time = 0.1f,  // 100 ms release
        .makeup_gain = 0.0f    // No makeup gain
    };

    // Apply compression
    vca_compression(audio_buffer, size, &params);
*/

    // Calculate attack and release coefficients
    float attack_coeff = expf(-1.0f / (SAMPLE_RATE * params->attack_time));
    float release_coeff = expf(-1.0f / (SAMPLE_RATE * params->release_time));

    // Convert gain parameters
    float threshold_linear = db_to_linear(params->threshold);
    float makeup_linear = db_to_linear(params->makeup_gain);

    float gain = 1.0f; // Initial gain

    for (size_t i = 0; i < size; i++) {
        float level = detect_level(buffer[i]);
        float gain_reduction = 1.0f;

        // Detect if level exceeds threshold
        if (level > threshold_linear) {
            // Compute gain reduction based on ratio
            float compressed_level = threshold_linear + (level - threshold_linear) / params->ratio;
            gain_reduction = compressed_level / level;
        }

        // Smooth gain with attack/release
        if (gain_reduction < gain) {
            // Attack (gain decreasing)
            gain = attack_coeff * (gain - gain_reduction) + gain_reduction;
        } else {
            // Release (gain increasing)
            gain = release_coeff * (gain - gain_reduction) + gain_reduction;
        }

        // Apply gain with makeup gain
        float applied_gain = gain * makeup_linear;

        // Apply gain to sample
        float sample = (float)buffer[i] / 32768.0f;
        sample *= applied_gain;

        // Clamp to 16-bit range
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;

        buffer[i] = (int16_t)(sample * 32767);
    }
}

/* --------------- FET AUDIO COMPRESSOR --------------- */

// FET compressor gain curve function
float fet_gain_curve(float level, float threshold, float ratio, float knee_width) {
    
    float level_db = linear_to_db(level);
    float threshold_db = threshold;
    float delta = level_db - threshold_db;
    float gain_db;

    if (knee_width > 0.0f) {
        // Soft knee
        float half_knee = knee_width / 2.0f;
        if (delta < -half_knee) {
            gain_db = 0.0f;
        } else if (delta > half_knee) {
            gain_db = delta - (delta / ratio);
        } else {
            // Within knee region
            float knee_fraction = (delta + half_knee) / knee_width;
            gain_db = knee_fraction * (delta - (delta / ratio));
        }
    } else {
        // Hard knee
        if (delta > 0) {
            gain_db = delta - (delta / ratio);
        } else {
            gain_db = 0.0f;
        }
    }

    float gain_linear = db_to_linear(gain_db);
    return gain_linear;
}

// Apply FET compression
void fet_compression(int16_t *buffer, size_t size, fet_compressor_params_t *params) {

/*
	// USAGE
	
    // Set FET compressor parameters
    fet_compressor_params_t params = {
        .threshold = -10.0f,     // Threshold at -10 dB
        .ratio = 4.0f,           // 4:1 ratio
        .attack_time = 0.001f,   // 1 ms attack
        .release_time = 0.05f,   // 50 ms release
        .knee_width = 6.0f,      // 6 dB soft knee
        .makeup_gain = 0.0f      // No makeup gain
    };

    // Apply FET compression
    fet_compression(audio_buffer, size, &params)
*/


    float attack_coeff = expf(-1.0f / (SAMPLE_RATE * params->attack_time));
    float release_coeff = expf(-1.0f / (SAMPLE_RATE * params->release_time));
    float makeup_linear = db_to_linear(params->makeup_gain);

    float gain = 1.0f; // Initial gain

    for (size_t i = 0; i < size; i++) {
        float level = detect_level(buffer[i]);
        float target_gain = fet_gain_curve(level, params->threshold, params->ratio, params->knee_width);

        // Smooth gain with attack/release
        if (target_gain < gain) {
            gain = attack_coeff * (gain - target_gain) + target_gain;
        } else {
            gain = release_coeff * (gain - target_gain) + target_gain;
        }

        // Apply gain with makeup gain
        float sample = (float)buffer[i] / 32768.0f;
        sample *= gain * makeup_linear;

        // Clamp to 16-bit range
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;

        buffer[i] = (int16_t)(sample * 32767);
    }
}

/* --------------- TUBE AUDIO COMPRESSOR --------------- */

// Nonlinear saturation curve for tube emulation
float tube_saturation(float sample) {
    // Soft clipping curve
    float clipped = sample;
    float abs_sample = fabsf(sample);
    if (abs_sample > 1.0f) {
        clipped = copysignf(1.0f, sample);
    } else {
        // Apply soft tube-like saturation
        clipped = sample - (powf(sample, 3) / 3.0f);
    }
    return clipped;
}

// Main compressor function
void tube_compression(int16_t *buffer, size_t size, tube_compressor_params_t *params) {
	
/*
    // USAGE
    
    // Define parameters
    tube_compressor_params_t params = {
        .threshold = -12.0f,     // -12 dB
        .ratio = 4.0f,           // 4:1 ratio
        .attack_time = 0.005f,   // 5 ms attack
        .release_time = 0.05f,   // 50 ms release
        .knee_width = 6.0f,      // 6 dB soft knee
        .makeup_gain = 0.0f      // No makeup gain
    };

    // Apply the "tube" compressor
    tube_compression(audio_buffer, size, &params);
*/

	
    float attack_coeff = expf(-1.0f / (SAMPLE_RATE * params->attack_time));
    float release_coeff = expf(-1.0f / (SAMPLE_RATE * params->release_time));
    float makeup_linear = db_to_linear(params->makeup_gain);
    float gain = 1.0f;

    float threshold_linear = db_to_linear(params->threshold);

    for (size_t i = 0; i < size; i++) {
        float level = detect_level(buffer[i]);
        float gain_reduction = 1.0f;

        // Detect if level exceeds threshold
        if (level > threshold_linear) {
            float delta = linear_to_db(level) - params->threshold;
            float gain_db;
            if (params->knee_width > 0.0f) {
                float half_knee = params->knee_width / 2.0f;
                if (delta < -half_knee) {
                    gain_db = 0.0f;
                } else if (delta > half_knee) {
                    gain_db = delta - (delta / params->ratio);
                } else {
                    // Soft knee region
                    float knee_fraction = (delta + half_knee) / params->knee_width;
                    gain_db = knee_fraction * (delta - (delta / params->ratio));
                }
            } else {
                // Hard knee
                gain_db = delta > 0 ? delta - (delta / params->ratio) : 0.0f;
            }
            gain_reduction = db_to_linear(gain_db);
        }

        // Smooth gain
        if (gain_reduction < gain) {
            gain = attack_coeff * (gain - gain_reduction) + gain_reduction;
        } else {
            gain = release_coeff * (gain - gain_reduction) + gain_reduction;
        }

        // Apply nonlinear saturation (tube emulation)
        float sample = (float)buffer[i] / 32768.0f;
        sample *= gain * makeup_linear;
        sample = tube_saturation(sample);

        // Clamp
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;

        buffer[i] = (int16_t)(sample * 32767);
    }
}

