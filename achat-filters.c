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

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

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

// Auto gain
void auto_gain(int16_t *buffer, size_t size, int16_t threshold) {
    
    // Find the maximum absolute value in the buffer
    int16_t max_val = 0;
    for (size_t i = 0; i < size; i++) {
        if (abs(buffer[i]) > max_val) {
            max_val = abs(buffer[i]);
        }
    }

    // Avoid division by zero
    if (max_val == 0) {
        return; // Buffer is silent, no gain adjustment needed
    }

    // Calculate the gain factor
    double gain = (double)threshold / max_val;

    // Apply gain to all samples
    for (size_t i = 0; i < size; i++) {
        double scaled = buffer[i] * gain;

        // Clip to int16 range
        if (scaled > 32767.0) scaled = 32767.0;
        if (scaled < -32768.0) scaled = -32768.0;

        buffer[i] = (int16_t)scaled;
    }
}

// Basic biquad filter with fixed coefficients for voice range at 16kHz (NEEDS WORK)
void simple_voice_bandpass(int16_t *buffer, size_t size) {
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
