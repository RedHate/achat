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
  * VAD voice activation detection
  * 
  * @param buffer - pointer to pcm buffer
  * @param size   - size of buffer
  * @return       - 0 no speech, 1 speech detected
  * 
  * @usage -
  *
  * // Detect speech
  * if (voice_activation_detection(pcm, FRAME_SIZE, 0.05f)) {
  *     printf("Speech detected.\n");
  * } else {
  *		printf("Silence.\n");
  * }
  * 
  */
int voice_activation_detection(short *buffer, int size, float threshold);

/**
  * De-esser
  * 
  * @param buffer    - pointer to pcm buffer
  * @param size      - size of buffer
  * @param threshold - target threshhold eg 0.2f
  * @param reduction - the amount of reduction to apply eg 0.5f
  */
void de_ess(int16_t *buffer, size_t size, float threshold, float reduction);

/**
  * Noise suppression (ironically adds noise if autogained)
  * 
  * @param buffer    - pointer to pcm buffer
  * @param size      - size of buffer
  * @param threshold - target threshhold 500 is a nice starting point
  */
void noise_suppress(int16_t *buffer, size_t size, int16_t threshold);

/**
  * Auto gain
  * 
  * @param buffer     - pointer to pcm buffer
  * @param size       - size of buffer
  * @param target_rms - desired RMS level (e.g., 10000.0)
  * @param max_gain   - maximum gain factor to prevent over-amplification
  */
void agc(int16_t *buffer, size_t size, float target_rms, float max_gain);

/**
  * Manual gain
  * 
  * @param buffer    - pointer to pcm buffer
  * @param size      - size of buffer
  * @param threshold - target threshhold 500 is a nice starting point
  */
void manual_gain(int16_t *buffer, size_t size, float threshold);

/**
  * Band pass filtering (BROKEN)
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  */

void band_pass_filter(int16_t *buffer, size_t size);

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

/**
  * distortion
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  */
void distortion(int16_t *buffer, size_t size);

/**
  * Diode_clipper
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  * @param threshold - distortion intensity
  */
void diode_clipper(int16_t *buffer, size_t size, float threshold);

/**
  * Pitch shifter
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  * @param pitch_factor - scale by ???
  */
void pitch_shift(int16_t *buffer, size_t size, float pitch_factor);

/**
  * Bit reducer
  * 
  * @param buffer - pointer to pcm buffer
  * @param size - size of buffer
  * @param bits_to_retain - number of bits to retain lol
  */
void bit_reducer(int16_t *buffer, size_t size, int bits_to_retain);

// Noise gate parameters
typedef struct noise_gate_params_t{
    float threshold;      // Threshold level (0.0 to 1.0)
    float attack_time;    // Attack time in seconds
    float release_time;   // Release time in seconds
    float sample_rate;    // Samples per second
} noise_gate_params_t;

/**
  * Noise gate
  * 
  * @param buffer - pointer to pcm buffer
  * @param size   - size of buffer
  * @param params - noise_gate_params_t*
  */
void noise_gate(int16_t *buffer, size_t size, noise_gate_params_t *params);

// Compressor parameters
typedef struct vca_compressor_params_t{
    float threshold;    // Threshold in dB
    float ratio;        // Compression ratio
    float attack_time;  // Attack time in seconds
    float release_time; // Release time in seconds
    float makeup_gain;  // Makeup gain in dB
} vca_compressor_params_t;

/**
  * VCA Compressor
  * 
  * @param buffer - pointer to pcm buffer
  * @param size   - size of buffer
  * @param params - vca_compressor_params_t*
  * 
  * @usage -
  *  
  *  // Set compressor parameters
  *  vca_compressor_params_t params = {
  *      .threshold = -10.0f,   // Threshold at -10 dB
  *      .ratio = 4.0f,         // 4:1 ratio
  *      .attack_time = 0.01f,  // 10 ms attack
  *      .release_time = 0.1f,  // 100 ms release
  *      .makeup_gain = 0.0f    // No makeup gain
  *  };
  *
  * // Apply compression
  * vca_compression(audio_buffer, size, &params);
  * 
  */
void vca_compression(int16_t *buffer, size_t size, vca_compressor_params_t *params);

// FET compressor parameters
typedef struct fet_compressor_params_t{
    float threshold;     // Threshold in dB
    float ratio;         // Compression ratio
    float attack_time;   // Attack time in seconds
    float release_time;  // Release time in seconds
    float knee_width;    // Knee width in dB for soft knee
    float makeup_gain;   // Makeup gain in dB
} fet_compressor_params_t;

/**
  * FET Compressor
  * 
  * @param buffer - pointer to pcm buffer
  * @param size   - size of buffer
  * @param params - fet_compressor_params_t*
  * 
  * @usage -
  *	
  * // Set FET compressor parameters
  * fet_compressor_params_t params = {
  *     .threshold = -10.0f,     // Threshold at -10 dB
  *     .ratio = 4.0f,           // 4:1 ratio
  *     .attack_time = 0.001f,   // 1 ms attack
  *     .release_time = 0.05f,   // 50 ms release
  *     .knee_width = 6.0f,      // 6 dB soft knee
  *     .makeup_gain = 0.0f      // No makeup gain
  * };
  *
  * // Apply FET compression
  * fet_compression(audio_buffer, size, &params)
  * 
  */
void fet_compression(int16_t *buffer, size_t size, fet_compressor_params_t *params) ;

// Tube compressor parameters
typedef struct tube_compressor_params_t{
    float threshold;      // Threshold in dB
    float ratio;          // Compression ratio
    float attack_time;    // Attack time in seconds
    float release_time;   // Release time in seconds
    float knee_width;     // Knee width in dB for soft knee
    float makeup_gain;    // Makeup gain in dB
} tube_compressor_params_t;

/**
  * TUBE Compressor
  * 
  * @param buffer - pointer to pcm buffer
  * @param size   - size of buffer
  * @param params - tube_compressor_params_t*
  * 
  * @usage -
  * 
  * // Define parameters
  * tube_compressor_params_t params = {
  *     .threshold = -12.0f,     // -12 dB
  *     .ratio = 4.0f,           // 4:1 ratio
  *     .attack_time = 0.005f,   // 5 ms attack
  *     .release_time = 0.05f,   // 50 ms release
  *     .knee_width = 6.0f,      // 6 dB soft knee
  *     .makeup_gain = 0.0f      // No makeup gain
  * };
  *
  * // Apply the "tube" compressor
  * tube_compression(audio_buffer, size, &params);
  * 
  */
void tube_compression(int16_t *buffer, size_t size, tube_compressor_params_t *params);
