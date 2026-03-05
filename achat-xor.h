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
  * c definitions
  */
#define XOR_KEY_LEN      96
#define XOR_KEY_ARRAY    4
#define XOR_FORWARD      0
#define XOR_BACKWARD     1
#define ENCODE           0
#define DECODE           1


/**
  * Print the key info
  */
void print_keys();

/**
  * XOR the keyset with a passphrase
  * 
  * @param password - a keyphrase to use to mutate xor keys
  */
void xor_keys(const char *password);

/**
  * Directional XOR fnc
  * 
  * @param buffer    - buffer to be transformed
  * @param size      - size in bytes
  * @param key       - uint32_t key array
  * @param direction - XOR_FORWARD or XOR_BACKWARD
  */
void xor_directional(uint8_t *buffer, size_t size, uint32_t *key, int direction);

/**
  * xor4x - main xor function
  * 
  * @param buffer - input buffer
  * @param size   - size in bytes
  */
void xor4x(uint8_t *buffer, size_t size);

/**
  * Shuffle bytes of 32bit word
  * 
  * @param block - uint32_t word
  * @param mode  - ENCODE or DECODE
  */
uint32_t shuffle32(uint32_t block, int mode);

/**
  * Byte flipping function
  * 
  * @param buffer - buffer to be transformed
  * @param size   - size in bytes
  * @param mode   - ENCODE or DECODE
  */
void bytefliparray(uint8_t *buffer, uint32_t size, int mode);

