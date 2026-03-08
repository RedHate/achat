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

/*
 * --------------------------------------------------------------------------
 *   About:
 * --------------------------------------------------------------------------
 * 
 * achat - An organically grown way of how not to create voice chat.
 * [16-Bit alsa / libopus voice chat]
 * 
 * c99 compliant
 * 
 * --------------------------------------------------------------------------
 *   Notes:
 * --------------------------------------------------------------------------
 * 
 *   You may encounter an issue with there being a douplicate time
 * definition in one of your alsa headers, alsa/config.h iirc. Comment
 * it out of that header. You have "time.h" in c99.
 * 
 *   May complain about implicit declaration of nanosleep. -It works 
 * on my end so test it before replacing it with some crap like sleep();
 * 
 * --------------------------------------------------------------------------
 *   Controls:
 * --------------------------------------------------------------------------
 * 
 * Input controls:
 *      Quit 'q'
 * 		Push to talk: 'spacebar'
 *      Mic monitor 'm'
 *      Enable stream masking 'x'
 *      Mic gain: '-' for less '+' for more
 * 		Highpass: '1' for less '2' for more
 * 		Lowpass:  '3' for less '4' for more
 * 		Pitch:    '5' for less '6' for more
 *      
 * --------------------------------------------------------------------------
 *   Tip:
 * --------------------------------------------------------------------------
 * 
 * Use plug devices! 
 * 			
 * 		plug:hw:1 rather than hw:1
 * 		NULL for no device
 * 
 *                               -Ultros was here in 2026
 *                                https://github.com/redhate
 * 
 */

/* ----------------------------------------*/
/*                  HEADERS                */
/* ----------------------------------------*/

// Headers
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <math.h>

/* ----------------------------------------*/
/*             PROJECT HEADERS             */
/* ----------------------------------------*/

// Project headers
#include "achat-cdefs.h"
#include "achat-types.h"
#include "achat-termios.h"
#include "achat-opus.h"
#include "achat-debug.h"
#include "achat-xor.h"
#include "achat-filters.h"
#include "achat-alsa.h"

/* ----------------------------------------*/
/*                   VARS                  */
/* ----------------------------------------*/

// Array for username
char   username[16];         // 16 bytes seems fair

// Var for shutdown
int    running      = 1;     // control var

// Var for debug
int    debug        = 1;

// For visualizer
int    color_mode   = 0;     // toggle color mode of the visualizer with 'c' key

// For filtering
float  highpass     = 0.0f;  // hipass filter threshold
float  lowpass      = 0.0f;  // lowpass filter threshold

// For voice pitch
float  voice_pitch  = 0.0f;

// For XOR / Byte/Nibbleshift
int mask_mode = 0;

// Mic data defined in achat-types.h
micinfo mic;

/* ----------------------------------------*/
/*                   SLEEP                 */
/* ----------------------------------------*/
void sleep_us(long microseconds) {
	
	// This may give an implicit declatation warning, but it seems to be functional try 1000000 as param
	
	// Local var
	struct timespec ts;
	// Set time granularity to the microsecond
	ts.tv_sec = microseconds / 1000000;
	ts.tv_nsec = (microseconds % 1000000) * 1000;
	// Sleep
	nanosleep(&ts, NULL);
}

/* ----------------------------------------*/
/*                ROGER BEEP               */
/* ----------------------------------------*/

int selected_roger_beep = 0;

void generate_tone(short *buffer, uint32_t size, int frequency){
	
    // Generate sine wave samples
    int i;
    for (i = 0; i < size; i++) {
        float t = (float)i / SAMPLE_RATE;
        float amplitude = sin(2 * M_PI * frequency * t);
        buffer[i] = (short)(amplitude * 32767); // 16-bit PCM
    }
    // Low pass filtering
	low_pass_filter(buffer, size, 0.125f);
}

void send_roger_beep(int sockfd, int beep_type){

	// Local vars
	uint16_t pcm[FRAME_SIZE];
	uint8_t opus[FRAME_SIZE];

	// generate tone
	int i;
	for(i = 0; i < 16; i++){	
		// check selected beep type
		switch(beep_type){
			case 0:{ /* nada */ }break;
			case 1:{ generate_tone(pcm, FRAME_SIZE, 100+(100*i));  }break;
			case 2:{ generate_tone(pcm, FRAME_SIZE, 1900-(100*i)); }break;
			case 3:{ generate_tone(pcm, FRAME_SIZE, 1000);         }break;
		}
		
		// opus encode
		int bytes_encoded = opus_encode_buffer(pcm, FRAME_SIZE, opus, FRAME_SIZE);
#ifdef _XOR_
		// if mask mode is enabled shuffle and xor
		if(mask_mode) xor_shuffle(opus, bytes_encoded, 1);
#endif
		// Send to sock
		ssize_t s = send(sockfd, opus, bytes_encoded, MSG_NOSIGNAL);
		if(s < 1){
			printf("Connection has died.\n");
			running = 0;
		}
	}
}


/* ----------------------------------------*/
/*                   ALSA                  */
/* ----------------------------------------*/

// ALSA handles
snd_pcm_t *capture_handle;
snd_pcm_t *playback_handle;

/*
 * Still need to seperate these into send / recv only and playback / capture only color threads.
 * I think I can check last packet time to defeat race conditions between threads
 */

// Thread for receiving audio and playing
void* receive_play_audio(void* arg) {
	// Get the socket fd
    int sockfd = *(int*)arg;
    // Create a buffer for opus data
    uint8_t opus[FRAME_SIZE];
	// Create a buffer for pcm
	short pcm[FRAME_SIZE];
	// Init mic.busy_start timer 
	mic.busy_start = time(NULL);
    // Receive some data
    while (running) {
		// Read into buffer from socket
		ssize_t r = recv(sockfd, opus, FRAME_SIZE, MSG_NOSIGNAL);
		if (r < 1) {
			printf("Connection has died.\n");
			running = 0;
		} 
		else if (r > 0) {
#ifdef _XOR_
			// if mask mode is enabled shuffle and xor
			if(mask_mode) xor_shuffle(opus, r, 0);
#endif
#ifdef _DEBUG_
			// debug
			if(debug){
				printf("\033[1;36mRecv) %ld\033[0m ", r);
				debug_print_hex(opus, r);
			}
#endif
			// opus decode
			opus_decode_buffer(opus, r, pcm, FRAME_SIZE);
			// write pcm to sound device
			ssize_t w = snd_pcm_writei(playback_handle, pcm, FRAME_SIZE);
			if (w == -EPIPE) {
				// Buffer underrun
				snd_pcm_prepare(playback_handle);
			} 
			else if (w < 0) {
				// Error
				fprintf(stderr, "Error reading PCM from device: %s\n", snd_strerror(w));
			}
			else if (w > 0) {
				// Set mic busy in the read thread (set not busy in the play thread since it doesnt bind there)
				mic.busy_start = time(NULL);
				mic.busy = 1;
			}
		}
    }
    // Return
    return NULL;
}

// Thread for capturing and sending
void* capture_send_audio(void* arg) {
	// Get the socket fd
    int sockfd = *(int*)arg;
    // Create a buffer for pcm
    short pcm[FRAME_SIZE];
	// Create a buffer for opus data
    uint8_t opus[FRAME_SIZE];
    // Send some data
    while (running) {
		// mic not busy timer checked with in this thread where it does not bind
		if((time(NULL)-mic.busy_start) > 0) {
			mic.busy = 0;
		}
		// This needs to stay reading
		// Read in buffer from capture handle
		ssize_t r = snd_pcm_readi(capture_handle, pcm, FRAME_SIZE);
		if (r == -EPIPE) {
			// Buffer underrun
			snd_pcm_prepare(capture_handle);
		} 
		else if (r < 0) {
			// Error
			fprintf(stderr, "Error writing PCM to device: %s\n", snd_strerror(r));
		}
		else if (r > 0) {
			// Are we keyed up? and is the mic busy?
			if((mic.key_up) && (!mic.busy)) {
				// Check the time 60 second time out
				if((time(NULL)-mic.key_up_start) < MIC_TIMEOUT_SECONDS) {
					// Detect speech
					if (voice_activation_detection(pcm, FRAME_SIZE, 0.045f)) {
						// ROFL! this is pricelessly funny but not very good, 0 - 1.0f
						pitch_shift(pcm, FRAME_SIZE, voice_pitch);
						// Noise suppression (meh makes the audio more choppy i dont like it)
						//noise_suppress(pcm, FRAME_SIZE, 300);
						// auto gain (sounds like fucking shit if you ask me, not a fan)
						agc(pcm, FRAME_SIZE, 100000.0f, mic.gain);
						// de-essing
						de_ess(pcm, FRAME_SIZE, 0.2f, 0.5f);
						// High pass filtering
						high_pass_filter(pcm, FRAME_SIZE, 1.0f-highpass);
						// Low pass filtering
						low_pass_filter(pcm, FRAME_SIZE, 1.0f-lowpass);
						// opus encode
						int bytes_encoded = opus_encode_buffer(pcm, FRAME_SIZE, opus, FRAME_SIZE);
#ifdef _DEBUG_
						// debug
						if(debug){
							printf("\033[1;35mSend) %d\033[0m ", bytes_encoded);
							debug_print_hex(opus, bytes_encoded);
						}
#endif
#ifdef _XOR_
						// if mask mode is enabled shuffle and xor
						if(mask_mode) xor_shuffle(opus, bytes_encoded, 1);
#endif
						// Send to sock
						ssize_t s = send(sockfd, (uint8_t*)opus, bytes_encoded, MSG_NOSIGNAL);
						if(s < 1){
							printf("Connection has died.\n");
							running = 0;
						} 
						else if(s > 0) {
							// data sent do debuging
						}
					
					} 
				}
				else{
					// turn off the mic after mic.key_up_start has passed MIC_TIMEOUT_SECONDS
					mic.key_up = 0;
					// Roger beep
					if(selected_roger_beep > 0) {
						// Send roger beep
				        send_roger_beep(sockfd, selected_roger_beep);
					    // Microsecond sleep
					    sleep_us(500000); //0.5 seconds (wait for roger beep to finish)
					}
					// Print debug
					printf("\033[1;36m[Mic off]\033[0m\n");
				}
				// Microphone monitor mode
				if(mic.monitor) {
					// write pcm to sound device
					ssize_t w = snd_pcm_writei(playback_handle, pcm, FRAME_SIZE);
					if (w == -EPIPE) {
						// Buffer underrun
						snd_pcm_prepare(playback_handle);
					} 
					else if (w < 0) {
						// Error
						fprintf(stderr, "Error reading PCM from device: %s\n", snd_strerror(w));
					}
				}
					
			}
		}
    }
    // Return
    return NULL;
}

/* ----------------------------------------*/
/*                  CLIENT                 */
/* ----------------------------------------*/

// Start chat 
int client(int argc, char *argv[]) {
    
    //printf("Type your username\n");
    //fgets(username, sizeof(username), stdin);
    
    // Local vars
    int capture = 0, 
        play = 0;
    
    // Check for capture device
    if(!strcmp(argv[3], "NULL")==0)
		capture = 1;
		
	// Check for playback device
	if(!strcmp(argv[4], "NULL")==0)
		play = 1;

	// Quick clear of the console
    system("clear");
    
    // Print legend 
    printf("\033[1;35m----------------------------------------------------\033[0m\n");
    printf("\n    \033[1;36mUltros \033[1;35mMaximus\n    \033[1;33mhttps://gitub.com/redhate\033[0m\n\n");

	// Print more legend 
	printf("\033[1;35m-----------------------AUDIO------------------------\033[0m\n");

	// If capture handle was initialized
	if(capture) {
		// Set capture device to selected capture device
		if(init_alsa(&capture_handle, argv[3], SND_PCM_STREAM_CAPTURE, CHANNELS) != 0) {
			// Return failure
			return 1;
		}
		
		// Apply the mic settings
		mic.gain     = 2.0f;
		mic.busy     = 0;
		mic.monitor  = 0;
		mic.key_up   = 0;
	}
	// If playback handle was initialized
	if(play) {
		// Set capture device to selected playback device
		if(init_alsa(&playback_handle, argv[4], SND_PCM_STREAM_PLAYBACK, CHANNELS) != 0) {
			// Return failure
			return 2;
		}
	}
    
    // Init opus encoder and decoder handles
    init_opus();
    
    // Init non-blocking input for spacebar on-off toggle
    enable_nonblocking_input();
    
#ifdef _XOR_
    // Mutate the included keys
    xor_keys("gofuckyourselfasshole!");
#endif
    
	// Print more legend 
	printf("\033[1;36m-----------------------STATUS-----------------------\033[0m\n");
    
	// Server info
    char *server_ip = argv[1];
    int port = atoi(argv[2]);

	// Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket failed");
        return 1;
    }

	// Set address family and port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);
	
    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connection failed");
        return 2;
    }
    
	// Debug
	printf("Connected!\n");
	
	// Set the socket fd
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd,&readfds);
	
	// Set socket timeout to 0
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	// Select the socket
	if(select(sockfd, &readfds, NULL, NULL, &tv) < 0) {
		perror("select");
		return 3;
	}
	
	// Set sock options
	int flag = 1;
	if(setsockopt(sockfd, IPPROTO_TCP,TCP_NODELAY,(char*)&flag,sizeof(int)) < 0) {
        perror("setsockopt");
        return 4;
    }
	
	// Print more legend 
	printf("\033[1;35m-----------------------THREADS----------------------\033[0m\n");
	
	// If playback handle was initialized
	if(play) {
		// init recv / playback thread
		pthread_t recv_thread;
		pthread_create(&recv_thread, NULL, receive_play_audio, &sockfd);
		printf("Receive thread started\n");
	}
	
	// If capture handle was initialized
	if(capture) {
		// init capture / send thread
		pthread_t send_thread;
		pthread_create(&send_thread, NULL, capture_send_audio, &sockfd);
		printf("Send thread started\n");
	}
	
	// Print more legend 
	printf("\033[1;36m----------------------NOW ONLINE--------------------\033[0m\n");
	printf("Press spacebar to start / stop the mic\nYou cannot mic up while someone else is talking\n");
	printf("\033[1;35m----------------------------------------------------\033[0m\n");
	
	// Loop de loop
	while (running) {
		// Input var
		char ch;
		// Read in byte from stdin
		int n = read(STDIN_FILENO, &ch, 1);
		if (n > 0 && ch == 'q') { // Quit
			// Shutdown
			running = 0;
		}
		if (n > 0 && ch == 'd') { // DEbug mode on / off
			// Swap debug display modes
			debug ++;
			// If its greater than 2, reset
			if(debug > 1)
				debug = 0;
			printf("debug mode %d\n", debug);
		}
		if (n > 0 && ch == 'x') { // Voice masking mode on / off
			// Toggle mask mode
			mask_mode ++;
			// If its greater than 2, reset
			if(mask_mode > 1)
				mask_mode = 0;
			printf("masking mode %d\n", mask_mode);
		}

		// If capture handle was initialized
		if(capture) {
			// Has spacebar been pressed?
			if (n > 0 && ch == ' ') { // Push to talk (latches on / off dont hold it)
				//Enable / disable the mic
				if((!mic.key_up) && (!mic.busy)) {
					// Debug
					printf("\033[1;35m[Mic on]\033[0m\n");
					/*
					// Roger beep
					if(selected_roger_beep > 0) {
						// Send roger beep
				        send_roger_beep(sockfd, selected_roger_beep);
					    // Microsecond sleep
					    sleep_us(500000); //0.5 seconds (wait for roger beep to finish)
					}
					*/
					// Mic on!
					mic.key_up = 1;
					// Start the timer
					mic.key_up_start = time(NULL);
				} 
				else if (mic.key_up) {
					// Mic off!
					mic.key_up = 0;
					// Roger beep
					if(selected_roger_beep > 0) {
						// Send roger beep
				        send_roger_beep(sockfd, selected_roger_beep);
					    // Microsecond sleep
					    sleep_us(500000); //0.5 seconds (wait for roger beep to finish)
					}
					// Debug
					printf("\033[1;36m[Mic off]\033[0m\n");
				}
				else{
					// Mic off!
					mic.key_up = 0;
					// Debug
					printf("\033[1;36m[Mic off]\033[0m\n");
				}
			}
			else if (n > 0 && ch == '-') { // Gain threshold down
				// If mic gain is greater than 0 decrement the gain
				if(mic.gain > 1.0f)
					mic.gain -= 1.0f;
				printf("mic.gain: %f\n", mic.gain);
			}
			else if (n > 0 && ch == '=') { // Gain threshold up
				// If mic gain is less than 3 increment the gain
				if(mic.gain < 10.0f)
					mic.gain += 1.0f;
				printf("mic.gain: %f\n", mic.gain);
			}
			else if (n > 0 && ch == '1') { // Highpass down
				// If highpass is greater than 0 decrement the gain
				if(highpass > 0.1f)
					highpass -= 0.1f;
				printf("highpass: %f\n", highpass);
			}
			else if (n > 0 && ch == '2') { // Highpass up
				// If highpass is less than 3 increment the gain
				if(highpass < 1.0f)
					highpass += 0.1f;
				printf("highpass: %f\n", highpass);
			}
			else if (n > 0 && ch == '3') { // Lowpass down
				// If lowpass is greater than 0 decrement the gain
				if(lowpass > 0.1f)
					lowpass -= 0.1f;
				printf("lowpass: %f\n", lowpass);
			}
			else if (n > 0 && ch == '4') { // Lowpass down
				// If lowpass is less than 3 increment the gain
				if(lowpass < 1.0f)
					lowpass += 0.1f;
				printf("lowpass: %f\n", lowpass);
			}
			else if (n > 0 && ch == '5') { // Pitch down
				// If highpass is greater than 0 decrement the gain
				if(voice_pitch > 0.1f)
					voice_pitch -= 0.1f;
				printf("voice_pitch: %f\n", voice_pitch);
			}
			else if (n > 0 && ch == '6') { // Pitch up
				// If highpass is less than 3 increment the gain
				if(voice_pitch < 1.0f)
					voice_pitch += 0.1f;
				printf("voice_pitch: %f\n", voice_pitch);
			}
			else if (n > 0 && ch == 'm') { // Microphone monitoring mode
				// Enable microphone monitoring mode
				if(!mic.monitor) {
					// Monitor on
					mic.monitor = 1;
					printf("mic.monitor: %d\n", mic.monitor);
				}
				else {
					// Monitor off
					mic.monitor = 0;
					printf("mic.monitor: %d\n", mic.monitor);
				}
			}
			else if (n > 0 && ch == 'b') { // Roger beep select
				if(selected_roger_beep < 3) selected_roger_beep ++;
				else selected_roger_beep = 0;
				printf("selected_roger_beep: %d\n", selected_roger_beep);
			}
			
		}
		// Microsecond sleep
		sleep_us(1000);
	}
	
	// Disable the mic so we dont segfault on the way out if quit is pressed while mic is keyed up
	mic.key_up = 0;
	
    // Destroy opus handles
    deinit_opus();
    // Destroy alsa handles
    deinit_alsa(&capture_handle);
    deinit_alsa(&playback_handle);
	// Close sockets
    close(sockfd);
    // Return the terminal to default status
    restore_terminal();
    
    // Exit clean
    return 0;
    
}

/* ----------------------------------------*/
/*                  SERVER                 */
/* ----------------------------------------*/

// Stream server
int server(int argc, char *argv[]) {
    
    // Check params
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

	// Local Vars
    int port = atoi(argv[1]);
    int listener, newfd, fdmax;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;
    fd_set master, read_fds;

	// Create a new socket called listener
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 2;
    }

	// Set the sock option to reuse the socket
    int yes = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
        perror("setsockopt");
        return 3;
    }

	// Address family, port stuff
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
	
	// Bind the listener socket
    if (bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 4;
    }

	// Now listen on the listener socket
    if (listen(listener, 10) < 0) {
        perror("listen");
        return 5;
    }

	// Zero some bytes, set and set fdmax
    FD_ZERO(&master);
    FD_SET(listener, &master);
    fdmax = listener;
	
	// Print a message
    printf("Server listening on port %d\n", port);	
                
	// Loop the socket management code
    while (running) {
		
		// set the read file descriptor
        read_fds = master;

		// Socket select
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            return 6;
        }

		// Loop through all the ports to from 0 - fdmax
		int i;
        for (i = 0; i <= fdmax; i++) {
			
			// Is this socket set?
            if (FD_ISSET(i, &read_fds)) {
				
				// If i is the listener
                if (i == listener) {
                    
                    // Accept a connection from the listener port
                    addrlen = sizeof(client_addr);
                    newfd = accept(listener, (struct sockaddr*)&client_addr, &addrlen);
					
					// If a new connection has been established on the listener
                    if (newfd != -1) {
						// Set the socked
                        FD_SET(newfd, &master);
                        // Check bounds set fdmax (increments it by doing so)
                        if (newfd > fdmax)
                            fdmax = newfd;

						// Debug
                        printf("New connection: socket fd %d, ip %s\n", newfd, inet_ntoa(client_addr.sin_addr));
                        printf("FDMAX: %d\n", fdmax);
                    }

                } 
                else {
                    
                    // Receive some data
					fd_set readfds;
					FD_ZERO(&readfds);
					FD_SET(i,&readfds);
					
					// Set delay to 0 for current sock
					struct timeval tv;
					tv.tv_sec = 0;
					tv.tv_usec = 0;
					select(i, &readfds, NULL, NULL, &tv);

					// Set socket option
					int flag = 1;
					setsockopt(i, IPPROTO_TCP,TCP_NODELAY,(char*)&flag,sizeof(int));

                    // Input buffer
                    uint16_t buffer[FRAME_SIZE];
                    
                    // Receive the buffer from the current socket
                    int r = recv(i, (uint8_t*)buffer, sizeof(buffer), MSG_NOSIGNAL);
                    if (r < 1) {
						// Close the current i index socket
                        close(i);
                        // Clear the socket
                        FD_CLR(i, &master);
                        // Debug
                        printf("Client disconnected\n");
					}
                    else if (r > 0) {

						// Debug
						printf("\033[1;36mRecv) %d\033[0m ", r);
						debug_print_hex((uint8_t*)buffer, r);
						
						// Send to all active sockets
						int j;
                        for (j = 0; j <= fdmax; j++) {
							// Check the socket is set, and not the listener or the current i index itteration
                            if (FD_ISSET(j, &master) && j != listener && j != i) {
								// Send the buffer to socket
                                int s = send(j,  (uint8_t*)buffer, r, MSG_NOSIGNAL);
                                if(s < 1){
									close(j);
									running = 0;
								}
								else if(s > 0){
									// Debug
									printf("\033[1;35mSend) %d\033[0m ", s);
									debug_print_hex((uint8_t*)buffer, s);
								}
                            }
                        }
                    } 
                }
            }
        }
    }

    return 0;
}

/*
// This is some other server code i was working on and just havent deleted
int server(int argc, char *argv[]) {
	
	//#define MAX_CLIENTS  FD_SETSIZE
	
    // Check params
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
	
	// Local Vars
    int port = atoi(argv[1]);
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[FRAME_SIZE];

    fd_set master_set, read_fds;
    int max_sd;

    int client_sockets[MAX_CLIENTS];

    // Initialize all client sockets to 0
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
        client_sockets[i] = 0;

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options (optional but recommended)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    max_sd = server_fd;

    while (1) {
        read_fds = master_set;

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select error");
        }

        // New incoming connection
        if (FD_ISSET(server_fd, &read_fds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
            if (new_socket < 0) {
                perror("accept");
                continue;
            }
            printf("New connection: socket fd %d, ip %s\n", new_socket, inet_ntoa(address.sin_addr));

            // Add new socket to array
            int i;
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
            FD_SET(new_socket, &master_set);
            if (new_socket > max_sd)
                max_sd = new_socket;
        }

		int i;
        // Check all clients for input
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0 && FD_ISSET(sd, &read_fds)) {
                int valread = read(sd, buffer, FRAME_SIZE);
                if (valread <= 0) {
                    // Client disconnected
                    printf("Client disconnected: socket fd %d\n", sd);
                    close(sd);
                    FD_CLR(sd, &master_set);
                    client_sockets[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    // Broadcast to others
                    int j;
                    for (j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] != 0 && client_sockets[j] != sd) {
                            send(client_sockets[j], buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
*/

/* ----------------------------------------*/
/*                   INFO                  */
/* ----------------------------------------*/

// Print the usage legend
static void print_usage(int argc, char *argv[]) {
	printf("\n    \033[1;36mUltros \033[1;35mMaximus\n    \033[1;33mhttps://gitub.com/redhate\033[0m\n");
	printf("\nUsage:\n    Server:\n        %s [listener_port]\n\n    Client:\n        %s [server_ip] [port] [capture device] [playback device]\n\n    Example:\n        Server:\n            %s 1122 \n\n        Client:\n            %s 127.0.0.1 1122 default default\n\n", argv[0], argv[0], argv[0], argv[0]);
}

/* ----------------------------------------*/
/*                   MAIN                  */
/* ----------------------------------------*/

// Program main
int main(int argc, char *argv[]) {
    
    // Print help with -h
	if (argc == 2) {
		if (strcmp(argv[1], "-h") == 0) {
			print_usage(argc, argv);
			return 1;
		}
		// Print device list with -l
		else if (strcmp(argv[1], "-l") == 0) {
			device_list(SND_PCM_STREAM_CAPTURE);
			device_list(SND_PCM_STREAM_PLAYBACK);
			return 1;
		}
		// Start server mode
		else {
			server(argc, argv);
		}
	}
	// Start client mode
	else if (argc == 5) {
        client(argc, argv);
    }
    // Print usage
    else {
        print_usage(argc, argv);
        return 1;
    }

    return 0;
    
}

