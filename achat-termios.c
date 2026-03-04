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

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/* ----------------- TERM -----------------*/

// Local Var
struct termios term;
struct termios term_orig;

// Restore original terminal properties
void restore_terminal(void) {
	tcsetattr(STDIN_FILENO,TCSANOW,&term_orig);
}

// Set terminal to non-blocking input
void enable_nonblocking_input(void) {
    // Get attributes
    tcgetattr(STDIN_FILENO, &term);
	// Get attributes
    tcgetattr(STDIN_FILENO, &term_orig);
    // Set exit
    atexit(restore_terminal);
    // Set Flags
    term.c_lflag &= ~(ICANON | ECHO); // raw input
    // Set attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    // Set stdin to non blocking
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}
