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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "achat-xor.h"

/* ----------------------------------------*/
/*                   XOR                   */
/* ----------------------------------------*/

uint32_t keys[XOR_KEY_ARRAY][XOR_KEY_LEN] = {
	{
		0x5C681CA3, 
		0xCFAF8B12, 
		0x6304C2ED, 
		0x1583F0EF, 
		0xFB53E0EC, 
		0x05FE5D58, 
		0x11FB13FD, 
		0x22031F4E, 
		0x1730B6CB, 
		0xEBAF1097, 
		0x1D7B3868, 
		0x3B44C4E2, 
		0x984D20BC, 
		0x17772E7F, 
		0x0DB08431, 
		0xE4ECD63F, 
		0xDAF40788, 
		0xDBE843E0, 
		0xDC3C000B, 
		0x59A2F6E1, 
		0x62DAF66D, 
		0x1916A088, 
		0xE8910E40, 
		0xC1B912EE, 
		0x9F16112B, 
		0xB4AA85BA, 
		0x4F485550, 
		0xDEEEC9B0, 
		0xDAF40788, 
		0xDBE843E0, 
		0xDC3C000B, 
		0x59A2F6E1, 
		0xBB3EC976, 
		0x96658EA6, 
		0x3B559747, 
		0xC726038E, 
		0xE52A03B7, 
		0xD6D04797, 
		0xF3AA1947, 
		0x8675FEA8, 
		0x4978724A, 
		0x07AE229B, 
		0x0770D2DA, 
		0xAB485423, 
		0xEEB7EBD1, 
		0x288B6556, 
		0x3F693E79, 
		0x4CD6D4F8, 
		0x9F0F36BF, 
		0xFDB7631E, 
		0x29CD6ABB, 
		0xEE0B82EE, 
		0x49011AC8, 
		0xB7EACAC3, 
		0x4AECE49E, 
		0x6C389A97, 
		0x6485E497, 
		0xBE376DD9, 
		0xE2D25E86, 
		0x5D5E6589, 
		0x072FF55F, 
		0xA06B1E37, 
		0x93E8E6E1, 
		0x50DCA452, 
		0x1C81F1F6, 
		0x96D5DC30, 
		0xAE2E2214, 
		0x691EDB80, 
		0x1C2069A0, 
		0xDC4AB6E8, 
		0x2555A276, 
		0xD5E710CA, 
		0x2FEC9B20, 
		0xF7661A99, 
		0xF027F7D7, 
		0x1B455615, 
		0x7BBE0877, 
		0x7E32FEC2, 
		0x4D883B75, 
		0xF55EE023, 
		0xFFA58331, 
		0xB03C29FD, 
		0x40433EF8, 
		0xACD36D62, 
		0xD02A7AAB, 
		0x3DCBDB3C, 
		0x8022AD9E, 
		0x7162214C, 
		0xB48D77B7, 
		0xFDAF0E5A, 
		0xFA454643, 
		0xC00CF78C, 
		0x8D045407, 
		0x464964DC, 
		0x24DAC8FE, 
		0xC250368D, 
	},
	{
		0x2DC86E2F, 
		0xF111A75A, 
		0xEF17C959, 
		0x438C3353, 
		0x161C635C, 
		0x12072838, 
		0x56175C09, 
		0xAA0F6276, 
		0x4BF26DE7, 
		0x451352CB, 
		0x6897D9C8, 
		0xD657D76A, 
		0xF979A24C, 
		0x4AAB9783, 
		0x3B7294F5, 
		0x775F8EC3, 
		0xB12419A8, 
		0xB4494C60, 
		0xACCC0027, 
		0x3F292D65, 
		0xE9B12FD9, 
		0x7D4C22A8, 
		0x4AD53740, 
		0xC75D5956, 
		0xE34E5587, 
		0x66009352, 
		0x72690010, 
		0xA555EF70, 
		0xB12419A8, 
		0xB4494C60, 
		0xACCC0027, 
		0x3F292D65, 
		0x57C5ECAE, 
		0xCFF3B43E, 
		0xD603CA5B, 
		0xDBBE0DB6, 
		0x71820D6B, 
		0x8D9159CB, 
		0x3D027C5B, 
		0x9FA20408, 
		0x6C99BB62, 
		0x1916A8F7, 
		0x1AB399B2, 
		0x066904AF, 
		0x54684495, 
		0x8AA6F00E, 
		0xC2CDC79D, 
		0x7F8D8718, 
		0xE333EC43, 
		0x0B6AEF66, 
		0x8EF8C057, 
		0x56258956, 
		0x6D0571E8, 
		0x6841E1CF, 
		0x615F76E6, 
		0xDCDAF0CB, 
		0xF69276CB, 
		0x46EADABD, 
		0x699B249E, 
		0x2827F3AD, 
		0x1B902023, 
		0x21C766EB, 
		0xDC4B7D65, 
		0x13AE351A, 
		0x6E86362E, 
		0xCD82ACF0, 
		0x1696AA44, 
		0xCD65B580, 
		0x6CA1CF20, 
		0xAD606D48, 
		0xB0032BAE, 
		0x827B53E2, 
		0x905EF7A0, 
		0x2AFE70FD, 
		0x30B8288B, 
		0x76500E41, 
		0x954629AB, 
		0x86F905CA, 
		0x7BA8D6A1, 
		0x202560AF, 
		0x01338FF5, 
		0x70CC8E09, 
		0x414FC518, 
		0x1F9ED8EA, 
		0x90839007, 
		0xCAE4B7CC, 
		0x80A81BE6, 
		0xB4EAA47C, 
		0x66B8A96B, 
		0x0B133732, 
		0x13505F4F, 
		0xC03F29BC, 
		0xB915041B, 
		0x5F6CF7AC, 
		0xB7B1EB06, 
		0xCB10ECB9, 
	},
	{
		0xF14E8CDF, 
		0xE00BD2FA, 
		0x1E6B5CC9, 
		0x7B4F0663, 
		0x77DA569C, 
		0x3275ABB8, 
		0x37629C99, 
		0x0AF94516, 
		0xF4D4B397, 
		0x14267E7B, 
		0xE1EA4548, 
		0xB32AA1CA, 
		0x6EE3868C, 
		0xE012EFB3, 
		0x8C5BDBA5, 
		0x02A762F3, 
		0xA3658328, 
		0xF0DD8A60, 
		0x600C0257, 
		0xCDBBFB35, 
		0x72A3D249, 
		0xA98E0828, 
		0xE7864340, 
		0xB288CC36, 
		0xD7AB0DF7, 
		0x0609A672, 
		0x54F90110, 
		0xF00B1870, 
		0xA3658328, 
		0xF0DD8A60, 
		0x600C0257, 
		0xCDBBFB35, 
		0x2B9B264E, 
		0x30C8F7DE, 
		0xB63F6FEB, 
		0x605ED6D6, 
		0x69A2DBDB, 
		0x5484C57B, 
		0xED25B9EB, 
		0x65824488, 
		0xA5020D42, 
		0x887C2787, 
		0xB18A0292, 
		0x60F94E5F, 
		0x12EC0DC5, 
		0x20C9F0EE, 
		0xEE11BE4D, 
		0x8755F698, 
		0xD00D2873, 
		0xBDC41906, 
		0x6174C527, 
		0x347D1C36, 
		0xBD526F68, 
		0xEC5FFD3F, 
		0x74A81886, 
		0x1175FC7B, 
		0x9FB51A7B, 
		0x2847716D, 
		0xF0296D7E, 
		0xAA58C97D, 
		0xA2922213, 
		0x3DB1085B, 
		0x18FCAB35, 
		0x294D64BA, 
		0x86E554CE, 
		0x15A863F0, 
		0x7FFC0E04, 
		0x1B3EED80, 
		0xA6BD3D20, 
		0x7B66B9C8, 
		0xB031914E, 
		0xA5CE6DC2, 
		0x95B18DA0, 
		0x85197F2D, 
		0x3B3AA03B, 
		0x1350EA51, 
		0xC124B31B, 
		0xE969596A, 
		0xC125BCB1, 
		0x22736A5F, 
		0x120B70A5, 
		0x7C046E99, 
		0x55B39498, 
		0xE673564A, 
		0x98BA9077, 
		0x64AFCB0C, 
		0x8A29A586, 
		0xFA40E3BC, 
		0x0D323FDB, 
		0xBA204412, 
		0x2655ABBF, 
		0xC3CDB27C, 
		0x284545AB, 
		0xA9A38D6C, 
		0xCCAF5B66, 
		0x7A1E2729, 
	},
	{
		0xBFC253DF, 
		0xEBD928FA, 
		0x753795C9, 
		0x34496563, 
		0xAD8CCA9C, 
		0x47DE13B8, 
		0x55FE0599, 
		0xF3BC5316, 
		0x20672497, 
		0x3258057B, 
		0x0BAF0D48, 
		0x998B6BCA, 
		0x8D650A8C, 
		0xF2FD5CB3, 
		0xD7807EA5, 
		0xA5C591F3, 
		0xC6E6AB28, 
		0x2D57EA60, 
		0x6C0E5557, 
		0x7640CE35, 
		0xD1719B49, 
		0x27862028, 
		0x61C50340, 
		0x3A44FA36, 
		0x7CA6FAF7, 
		0x0FAFD472, 
		0xADF81110, 
		0xFB136870, 
		0xC6E6AB28, 
		0x2D57EA60, 
		0x6C0E5557, 
		0x7640CE35, 
		0xB0BD684E, 
		0xF83F29DE, 
		0x895084EB, 
		0x3E8800D6, 
		0xCB7900DB, 
		0xD041BE7B, 
		0xC89C52EB, 
		0xE7C6CC88, 
		0xA70F4F42, 
		0xF45BA087, 
		0x3B889092, 
		0x99B7115F, 
		0xFEE1C8C5, 
		0xE9391EEE, 
		0xFFAFF34D, 
		0xD2A36E98, 
		0xDD255B73, 
		0x79DD1F06, 
		0x15B1E227, 
		0x49612A36, 
		0xEF3D0768, 
		0xB3A2C23F, 
		0xDCB09E86, 
		0x6489877B, 
		0x2AAF617B, 
		0x6F361C6D, 
		0xD944137E, 
		0xF291B47D, 
		0x30B03113, 
		0x8CB9535B, 
		0xE4579E35, 
		0x6429DEBA, 
		0x63B19ACE, 
		0xBDCB93F0, 
		0x83F20A04, 
		0x25D36D80, 
		0x1B801D20, 
		0x1DDF71C8, 
		0x81A0DF4E, 
		0x6BA3AFC2, 
		0x243C2DA0, 
		0x9C66522D, 
		0x019A9B3B, 
		0x43BABB51, 
		0xE597A81B, 
		0x8030336A, 
		0xE4990DB1, 
		0x5119355F, 
		0x197BD5A5, 
		0x786AF799, 
		0xE6270C98, 
		0x95251C4A, 
		0x222AE777, 
		0xCB64C70C, 
		0xA38C2386, 
		0xBAA35FBC, 
		0x3F0DE4DB, 
		0x9A645612, 
		0x73FE14BF, 
		0x0E7FCE7C, 
		0x6D00EEAB, 
		0x0A2EE16C, 
		0x63F43D66, 
		0x64390E29, 
	},
};

// Print the key info
void print_keys() {
	// Loop through the keys
	int i, j;
	for (i = 0; i < XOR_KEY_ARRAY; i++) {
		// Loop through key length
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// print the bytes
			printf("0x%08X \n", (uint32_t)keys[i][j]);
		}
		// print a new line for each key
		printf("\n");
	}
}

// XOR the keyset with a passphrase
void xor_keys(const char *password) {
	// Set key position to 0
	int keypos = 0;
	// Loop through the keys
	int i, j;
	// positive
	for (i = 0; i < XOR_KEY_ARRAY-1; i++) {
		// Loop through selected key
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// XOR the key
			keys[i][j] = (uint32_t)((uint8_t)password[keypos] ^ (uint32_t)keys[i][j]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == strlen(password)) keypos = 0;
		}
	}
	// negative
	for (i = XOR_KEY_ARRAY-1; i > 0; i--) {
		// Loop through selected key
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// XOR the key
			keys[i][j] = (uint32_t)((uint8_t)password[keypos] ^ (uint32_t)keys[i][j]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == strlen(password)) keypos = 0;
		}
	}
}

// Directional XOR fnc (doesn't need to be global, even if that's how you think it should be done. i scope my code.)
void xor_directional(uint8_t *buffer, size_t size, uint32_t *key, int direction) {
	// Used for xor position
	int keypos = 0;
	// XOR forward / backward
	if (direction == XOR_FORWARD) {
		// Lazy XOR
		uint32_t i;
		for (i = 0; i < size; i++) {
			// XOR shift the data according to the key and its relative read position
			buffer[i] = (uint8_t)((uint8_t)key[keypos] ^ (uint8_t)buffer[i]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == XOR_KEY_LEN) keypos = 0;
		}
	}
	else if (direction == XOR_BACKWARD) {
		// Lazy XOR
		uint32_t i;
		for (i = size; i > 0; i--) {
			// XOR shift the data according to the key and its relative read position
			buffer[i] = (uint8_t)((uint8_t)key[keypos] ^ (uint8_t)buffer[i]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == XOR_KEY_LEN) keypos = 0;
		}
	}
}

// This adds a reversable layer of noise to our signal
void xor4x(uint8_t *buffer, size_t size) {
	// Set the direction to 0
	int direction = 0;
	// loop through keys
	int i;
	// positive
	for (i = 0; i < XOR_KEY_ARRAY-1; i++) {
		// odds
		if (i & 1)
			direction = XOR_FORWARD;
		// evens
		if (!(i & 1))
			direction = XOR_BACKWARD;
		//xor it in a direction
		xor_directional(buffer, size, keys[i], direction);
	}
	// Negative
	for (i = XOR_KEY_ARRAY-1; i > 0; i--) {
		// odds
		if (i & 1)
			direction = XOR_FORWARD;
		// evens
		if (!(i & 1))
			direction = XOR_BACKWARD;
		//xor it in a direction
		xor_directional(buffer, size, keys[i], direction);
	}
}

/* ----------------------------------------*/
/*           NIBBLE/BYTE SHUFFLE           */
/* ----------------------------------------*/

// 8bit nibble swap
uint8_t nibbleswap8(uint8_t x) {
	// Swap left and right nibbles
	x = ((x & 0x0F) << 4) | ((x & 0xF0) >> 4);
    // Return the nibble
    return x;
}

void nibbleswaparray8(uint8_t *buffer, uint32_t size){
	// Loop through the array
	int i;
	for(i=0;i<size;i++){
		// Shuffle and re-assign 4 bytes of the array
		buffer[i] = nibbleswap8(buffer[i]);
	}
}


// 16bit nibble swapping
uint16_t nibbleswap16(uint16_t block, int mode){
	// byte assignment
	uint16_t a = (block >> 0)  & 0xF,
			 b = (block >> 4)  & 0xF,
			 c = (block >> 8)  & 0xF,
			 d = (block >> 12) & 0xF;
	// Check mode
	if(mode)
		// mode 1 (inversion of mode 2)
		return (c<<0)|(a<<4)|(d<<8)|(b<<12);
	else
		// mode 2 (inversion of mode 1)
		return (b<<0)|(d<<4)|(a<<8)|(c<<12);
}

void nibbleswaparray16(uint8_t *buffer, size_t size, int mode){
	// Loop through the array
	int i;
	for(i=0;i<size;i+=2){
		// interleve nibbleswap16 mode
		if(!mode)
			mode = 1;
		else
			mode = 0;
		// Shuffle and re-assign nibbles of the array
		*((uint16_t*)&buffer[i]) = nibbleswap16(*((uint16_t*)&buffer[i]), mode);
	}
}


// 32bit nibble swapping
uint32_t nibbleswap32(uint32_t block, int mode){
	// byte assignment
	uint32_t a = (block >> 0)  & 0xF,
			 b = (block >> 4)  & 0xF,
			 c = (block >> 8)  & 0xF,
			 d = (block >> 12) & 0xF,
             e = (block >> 16) & 0xF,
			 f = (block >> 20) & 0xF,
			 g = (block >> 24) & 0xF,
			 h = (block >> 28) & 0xF;
	// Check mode
	if(mode)
		// mode 1 (inversion of mode 2) // weird last 4 nibbles are the same? huh?
		return (a<<28)|(b<<16)|(c<<0)|(d<<8)|(e<<4)|(f<<12)|(g<<20)|(h<<24);
	else
		// mode 2 (inversion of mode 1) // weird last 4 nibbles are the same? huh?
		return (h<<0)|(e<<4)|(a<<8)|(c<<12)|(b<<16)|(d<<20)|(f<<24)|(g<<28);
}

void nibbleswaparray32(uint8_t *buffer, size_t size, int mode){
	// Loop through the array
	int i;
	for(i=0;i<size;i+=4){
		// interleve nibbleswap32 mode
		if(!mode)
			mode = 1;
		else
			mode = 0;
		// Shuffle and re-assign nibbles of the array
		*((uint32_t*)&buffer[i]) = nibbleswap32(*((uint32_t*)&buffer[i]), mode);
	}
}


// 32bit byte swapping
uint32_t byteswap32(uint32_t block, int mode){
	// byte assignment
	uint32_t a = (block >> 0)  & 0xFF,
			 b = (block >> 8)  & 0xFF,
			 c = (block >> 16) & 0xFF,
			 d = (block >> 24) & 0xFF;
	// Check mode
	if(mode)
		// mode 1 (inversion of mode 2)
		return (c<<0)|(a<<8)|(d<<16)|(b<<24);
	else
		// mode 2 (inversion of mode 1)
		return (b<<0)|(d<<8)|(a<<16)|(c<<24);
}

void byteswaparray32(uint8_t *buffer, size_t size, int mode){
	// Loop through the array
	int i;
	for(i=0;i<size;i+=4){
		// interleve shuffle32 mode
		if(!mode)
			mode = 1;
		else
			mode = 0;
		// Shuffle and re-assign 4 bytes of the array
		*((uint32_t*)&buffer[i]) = byteswap32(*((uint32_t*)&buffer[i]), mode);
	}
}


// Main obfuscation function
void mutate_data(uint8_t *buffer, size_t size, int mode){
	
	/* there's no way some looky lou is just going to magically crack this */
	
	int i;
	
	for(i=0;i<5;i++){
		if(mode) {
			xor4x(buffer, size);
			nibbleswaparray8(buffer, size);
			xor4x(buffer, size);
			nibbleswaparray16(buffer, size, 1);
			xor4x(buffer, size);
			nibbleswaparray32(buffer, size, 1);
			xor4x(buffer, size);
			byteswaparray32(buffer, size, 1);
			xor4x(buffer, size);
			//debug_print_hex(buffer, size);
		}
		else {
			xor4x(buffer, size);
			byteswaparray32(buffer, size, 0);
			xor4x(buffer, size);
			nibbleswaparray32(buffer, size, 0);
			xor4x(buffer, size);
			nibbleswaparray16(buffer, size, 0);
			xor4x(buffer, size);
			nibbleswaparray8(buffer, size);
			xor4x(buffer, size);
			//debug_print_hex(buffer, size);
		}
	}
}


