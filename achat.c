/*
 * 
 * vchat
 * [16-Bit alsa / libopus voice chat]
 * 
 * An organically grown way of how not to create voice chat.
 * 
 * c99 compliant
 * 
 * **NOTE**
 * 
 *   You may encounter an issue with there being a douplicate time
 * definition in one of your alsa headers, alsa/config.h iirc. Comment
 * it out of that header. You have "time.h" in c99.
 * 
 *   Also may complain about implicit declaration of nanosleep. -It
 * works on my end so test it before replacing it with some crap like 
 * sleep();
 * 
 * Latest changes, if a user is currently mic'd up the program will
 * not allow you to mic up over them. Now there is proper shared channel
 * style chat.
 * 
 * Input controls:
 * 		Push to talk: 'spacebar'
 * 		Show debug: 'd'
 *      Change color of visualiser: 'c'
 *      Mic gain: '-' for less '+' for more
 *      Mic monitor 'm'
 *      Quit 'q'
 * 
 * 
 * Use plug devices! 
 * 			
 * 		plug:hw:1 rather than hw:1
 * 		NULL for no device
 * 
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <alloca.h>
#include <termios.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <opus/opus.h>

/* ----------------------------------------*/
/*             PROJECT HEADERS             */
/* ----------------------------------------*/

// Project headers
#include "achat-termios.h"
#include "achat-opus.h"
#include "achat-cdefs.h"
#include "achat-debug.h"
#include "achat-xor.h"

/* ----------------------------------------*/
/*                   VARS                  */
/* ----------------------------------------*/

/*
* not implemented yet, still debating if i
* want usernames.
*/
/*
// Payload data to be sent over socket in future edit, subject to change
typedef struct payload{
	char username[MAX_NAME_LEN]; // username buffer 16 byte names max
	uint8_t opus[FRAME_SIZE];    // opus data
}payload;
*/

// Var for shutdown
int running = 1;         // control var

// For visualizer
int debug      = 0;      // toggle with 'd' button
int color_mode = 0;      // toggle color mode of the visualizer with 'c' key

// Microphone
float  mic_gain = 1.0f;  // controlled with ',' and '.' keys
int    mic_busy = 0;     // var for microphone ptt lockout when receiving
time_t mic_busy_start;   // timer for microphone ptt lockout when receiving
int    mic_monitor = 0;  // var for toggling microphone monitor with 'm'

// Key up
int    key_up   = 0;     // toggle with spacebar
time_t key_up_start;     // used to count time for mic timeout

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
/*                   ALSA                  */
/* ----------------------------------------*/

// ALSA handles
snd_pcm_t *capture_handle;
snd_pcm_t *playback_handle;

// Initialize ALSA for capture or playback
int init_alsa(snd_pcm_t **handle, const char *device, snd_pcm_stream_t stream, uint32_t channels) {
    
    // Local vars
    snd_pcm_hw_params_t *params;
    int err;
    
    // Init a handle for the PCM steam from a chosen device
    if ((err = snd_pcm_open(handle, device, stream, 0)) < 0) {
        fprintf(stderr, "ALSA open error: %s\n", snd_strerror(err));
        return 1;
    }
	
	// I assume this allocates memory for params, return doesn't allow error checking
	snd_pcm_hw_params_alloca(&params);
	
	// Init all channel params and error check
    if ((err = snd_pcm_hw_params_any(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_any error: %s\n", snd_strerror(err));
		return 2;
	}
	
	// (ignore that gcc throws warning on this or fix it lol)
	// set resample since we want to adjust the rate 
	uint32_t resample = 1;
    if ((err = snd_pcm_hw_params_set_rate_resample(*handle, params, *(unsigned int*)&resample)) < 0) {
        //fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate_resample error: %s\n", snd_strerror(err));
		return 3;
	}
	// Set access
    if ((err = snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_access error: %s\n", snd_strerror(err));
		return 4;
	}
	// Set format
    if ((err = snd_pcm_hw_params_set_format(*handle, params, FORMAT)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_format error: %s\n", snd_strerror(err));
		return 5;
	}
	
	// Set channels
    snd_pcm_hw_params_set_channels(*handle, params, channels);
    
	// Set rate
	uint32_t sample_rate = SAMPLE_RATE;
    if ((err = snd_pcm_hw_params_set_rate_near(*handle, params, &sample_rate, 0)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate error: %s\n", snd_strerror(err));
		return 6;
	}
	// Set hw params
    if ((err = snd_pcm_hw_params(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params error: %s\n", snd_strerror(err));
		return 7;
	}
	// Prepare for pcm
    if ((err = snd_pcm_prepare(*handle)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_prepare error: %s\n", snd_strerror(err));
		return 8;
	}

	// Print message
    printf("ALSA initialized at %u Hz for %d\n", sample_rate, stream);

    return 0;
    
}

// Destroy alsa handles
int deinit_alsa(){
	// Local var
	int err;
	// Destroy capture handle
	if ((err = snd_pcm_close(capture_handle)) < 0) {
		return 1;
	}
	// Destroy playback handle
	if ((err = snd_pcm_close(playback_handle)) < 0) {
		return 2;
	}
	return 0;
}

// Thread for receiving audio and playing
void* receive_play_audio(void* arg) {
	// Get the socket fd
    int sockfd = *(int*)arg;
    // Create a buffer for opus data
    uint8_t opus[FRAME_SIZE];
	// Create a buffer for pcm
	short pcm[FRAME_SIZE];
	// Init mic_busy_start timer 
	mic_busy_start = time(NULL);
    // Receive some data
    while (running) {
		// Read into buffer from socket
		ssize_t r = read(sockfd, opus, FRAME_SIZE);
		if (r > 0) {
#ifdef _DEBUG_
			// Debug
			if(debug == 2) {
				printf("\033[1;36mRecv) %ld\033[0m ", r);
				debug_print_buf((uint8_t*)opus, r);
			}
#endif
#ifdef _XOR_
			// Byte flipping
			bytefliparray((uint8_t*)opus, sizeof(opus), 0);
			// Apply XOR
			xor4x((uint8_t*)opus, sizeof(opus));
#endif
			// opus decode
			opus_decode_buffer(opus, r, pcm, FRAME_SIZE);			
			// write pcm to sound device
			r = snd_pcm_writei(playback_handle, pcm, FRAME_SIZE);
			if (r == -EPIPE) {
				// Buffer underrun
				snd_pcm_prepare(playback_handle);
			} 
			else if (r < 0) {
				// Error
				fprintf(stderr, "Error reading PCM from device: %s\n", snd_strerror(r));
			}
#ifdef _DEBUG_
			else if (r > 0){
				// Print debug (toggled with d)
				if(debug == 1) {
					printf("\033[1;36mPCM) %d\033[0m ", FRAME_SIZE);
					debug_print_buf((uint8_t*)pcm, FRAME_SIZE);
				}
				// Print visualiser (toggled with d)
				if(debug == 3) {
					audio_visualiser(pcm, sizeof(pcm), color_mode);
				}
				// Set mic busy in the read thread (set not busy in the play thread since it doesnt bind there)
				mic_busy_start = time(NULL);
				mic_busy = 1;
			}
#endif
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
		if((time(NULL)-mic_busy_start) > 0) {
			mic_busy = 0;
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
			if((key_up) && (!mic_busy)) {	
				// Check the time 60 second time out
				if((time(NULL)-key_up_start) < MIC_TIMEOUT_SECONDS) {
					int c;
					// Adjust mic gain before modulation
					for (c = 0; c < sizeof(pcm); c++) {
						pcm[c] = (opus_int16)(pcm[c] * mic_gain);
					}
#ifdef _DEBUG_
					if(debug == 1) {
					// Print debug
						printf("\033[1;35mPCM) %d\033[0m ", FRAME_SIZE);
						debug_print_buf((uint8_t*)pcm, FRAME_SIZE);
					}
#endif
					// opus encode
					int bytes_encoded = opus_encode_buffer(pcm, FRAME_SIZE, opus, FRAME_SIZE);
#ifdef _XOR_
					// Apply XOR
					xor4x((uint8_t*)opus, sizeof(opus));
					// Byte flipping
					bytefliparray((uint8_t*)opus, sizeof(opus), 1);
#endif
					// Send to sock
					write(sockfd, (uint8_t*)opus, bytes_encoded);
#ifdef _DEBUG_
					// Debug
					if(debug == 2) {
					// Print debug
						printf("\033[1;35mSend) %d\033[0m ", bytes_encoded);
						debug_print_buf((uint8_t*)opus, bytes_encoded);
					}
					if(debug == 3) {
						audio_visualiser(pcm, sizeof(pcm), color_mode);
					}
#endif
				}
				else{
					// turn off the mic after key_up_start has passed MIC_TIMEOUT_SECONDS
					key_up = 0;
					printf("\033[1;36m[Mic off]\033[0m\n");
				}
			
				// Microphone monitor mode
				if(mic_monitor) {
					// write pcm to sound device
					r = snd_pcm_writei(playback_handle, pcm, FRAME_SIZE);
					if (r == -EPIPE) {
						// Buffer underrun
						snd_pcm_prepare(playback_handle);
					} 
					else if (r < 0) {
						// Error
						fprintf(stderr, "Error reading PCM from device: %s\n", snd_strerror(r));
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
    
    // print legend 
    printf("\033[1;35m----------------------------------------------------\033[0m\n");
    printf("\n    \033[1;36mUltros \033[1;35mMaximus\n    \033[1;33mhttps://gitub.com/redhate\033[0m\n\n");

	// print more legend 
	printf("\033[1;35m------------------------ALSA------------------------\033[0m\n");

	// If capture handle was initialized
	if(capture) {
		// Set capture device to selected capture device
		if(init_alsa(&capture_handle, argv[3], SND_PCM_STREAM_CAPTURE, CHANNELS) != 0){
			// Return failure
			return 1;
		}
	}
	// If playback handle was initialized
	if(play) {
		// Set capture device to selected playback device
		if(init_alsa(&playback_handle, argv[4], SND_PCM_STREAM_PLAYBACK, CHANNELS) != 0){
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
	
	// print more legend 
	printf("\033[1;36m-----------------------STATUS-----------------------\033[0m\n");
	
    // Connect to server
    if (!connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        perror("Connection failed");
        return 1;
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
	select(sockfd, &readfds, NULL, NULL, &tv);

	// Set sock options
	int flag = 1;
	setsockopt(sockfd, IPPROTO_TCP,TCP_NODELAY,(char*)&flag,sizeof(int));

    // If client sock has been assigned
    if(sockfd >= 0) {
		
		// print more legend 
		printf("\033[1;35m-----------------------THREADS----------------------\033[0m\n");
		
		// If playback handle was initialized
		if(play) {
			// init alsa playback and capture threads and psockets
			pthread_t recv_thread;
			pthread_create(&recv_thread, NULL, receive_play_audio, &sockfd);
			printf("Receive thread started\n");
		}
		
		// If capture handle was initialized
		if(capture) {
			// init alsa playback and capture threads and psockets
			pthread_t send_thread;
			pthread_create(&send_thread, NULL, capture_send_audio, &sockfd);
			printf("Send thread started\n");
		}
		
		// print more legend 
		printf("\033[1;36m----------------------NOW ONLINE--------------------\033[0m\n");
		printf("Press spacebar to start / stop the mic\nYou cannot mic up while someone else is talking\n");
		printf("\033[1;35m----------------------------------------------------\033[0m\n");
		
		// Loop de loop
		while (running) {		
			// Input var
			char ch;
			// Read in byte from stdin
			int n = read(STDIN_FILENO, &ch, 1);
			if (n > 0 && ch == 'q') {
				// Shutdown
				running = 0;
			}
#ifdef _DEBUG_
			if (n > 0 && ch == 'd') {
				// Swap debug display modes
				debug ++;
				// If its greater than 2, reset
				if(debug > 3)
					debug = 0;
			}
			if (n > 0 && ch == 'c') {
				// Swap debug display modes
				color_mode ++;
				// If its greater than 2, reset
				if(color_mode > 1)
					color_mode = 0;
			}
#endif
			// If capture handle was initialized
			if(capture) {
				// Has spacebar been pressed?
				if (n > 0 && ch == ' ') {
					//Enable / disable the mic
					if((!key_up) && (!mic_busy)) {
						// Mic on!
						key_up = 1;
						// Start the timer
						key_up_start = time(NULL);
						// Debug
						printf("\033[1;35m[Mic on]\033[0m\n");
					} 
					else {
						// Mic off!
						key_up = 0;
						// Debug
						printf("\033[1;36m[Mic off]\033[0m\n");
					}
					
				}
				else if (n > 0 && ch == '-') {
					mic_gain -= 0.1f;
					printf("mic_gain: %f\n", mic_gain);
				}
				else if (n > 0 && ch == '=') {
					mic_gain += 0.1f;
					printf("mic_gain: %f\n", mic_gain);
				}
				else if (n > 0 && ch == 'm') {
					if(!mic_monitor) {
						mic_monitor = 1;
						printf("mic_monitor: %d\n", mic_monitor);
					}
					else {
						mic_monitor = 0;
						printf("mic_monitor: %d\n", mic_monitor);
					}
				}
			}
			// Microsecond sleep
			sleep_us(1000);
		}
		
	}
	
	// Close sockets
    close(sockfd);
    // Destroy opus handles
    deinit_opus();
    // Destroy alsa handles
    deinit_alsa();
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
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        return 2;
    }

	// Set the sock option to reuse the socket
    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	// Address family, port stuff
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
	
	// Bind the listener socket
    if (bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 3;
    }

	// Now listen on the listener socket
    if (listen(listener, 10) < 0) {
        perror("listen");
        return 4;
    }

	// Zero some bytes, set and set fdmax
    FD_ZERO(&master);
    FD_SET(listener, &master);
    fdmax = listener;
	
	// Print a message
    printf("Server listening on port %d\n", port);	
                
	// Loop the socket management code
    while (running) {
		
        read_fds = master;

		// Socket select
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            return 5;
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
                    int nbytes = read(i, (uint8_t*)buffer, sizeof(buffer));
                    if (nbytes > 0) {

						// Debug
						printf("\033[1;36mRecv) %d\033[0m ", nbytes);
						debug_print_buf((uint8_t*)buffer, nbytes);
						
						// Send to all active sockets
						int j;
                        for (j = 0; j <= fdmax; j++) {
							// Check the socket is set, and not the listener or the current i index itteration
                            if (FD_ISSET(j, &master) && j != listener && j != i) {
								// Send the buffer to socket
                                write(j,  (uint8_t*)buffer, nbytes);

                                // Debug
                                printf("\033[1;35mSend) %d\033[0m ", nbytes);
								debug_print_buf((uint8_t*)buffer, nbytes);

                            }
                        }
                    } 
                    else {
						// Close the current i index socket
                        close(i);
                        // Clear the socket
                        FD_CLR(i, &master);
                        // Debug
                        printf("Client disconnected\n");
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
	printf("\nUsage:\n    Server:\n        %s [listener_port] [capture device] [playback device]\n\n    Client:\n        %s [server_ip] [port] [capture device] [playback device]\n\n    Example:\n        Server:\n            %s 1122 default default\n\n        Client:\n            %s 127.0.0.1 1122 default default\n\n", argv[0], argv[0], argv[0], argv[0]);
}

// Borrowed from aplay.c (why reinvent the wheel?)
static void device_list(snd_pcm_stream_t stream) {
	
	// Local vars
	snd_ctl_t *handle;
	int dev;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);

	int card = -1;
	if (snd_card_next(&card) < 0 || card < 0) {
		fprintf(stderr, "no soundcards found...\n");
		return;
	}
	
	printf(("\n------ List of %s Hardware Devices ------\n"), snd_pcm_stream_name(stream));
	
	while (card >= 0) {
		
		int err;
		char name[32];
		sprintf(name, "hw:%d", card);
		dev = -1;
		
		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
			fprintf(stderr, "control open (%i): %s", card, snd_strerror(err));
			goto next_card;
		}
		
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
			fprintf(stderr, "control hardware info (%i): %s", card, snd_strerror(err));
			snd_ctl_close(handle);
			goto next_card;
		}

		while (1) {
			
			if (snd_ctl_pcm_next_device(handle, &dev) < 0) {
				fprintf(stderr, "snd_ctl_pcm_next_device\n");
			}
			
			if (dev < 0) {
				break;
			}
			
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, stream);
			
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
				if (err != -ENOENT)
					fprintf(stderr, "control digital audio info (%i): %s\n", card, snd_strerror(err));
				continue;
			}
			
			printf(("card %i: %s [%s], device %i: %s [%s]\n"),
					card, snd_ctl_card_info_get_id(info), snd_ctl_card_info_get_name(info),
					dev,
					snd_pcm_info_get_id(pcminfo),
					snd_pcm_info_get_name(pcminfo));

		}
		
		snd_ctl_close(handle);
		
		next_card:

		if (snd_card_next(&card) < 0) {
			fprintf(stderr, "snd_card_next");
			break;
		}
		
	}
	
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

