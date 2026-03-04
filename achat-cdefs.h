/* ----------------------------------------*/
/*                   CDEFS                 */
/* ----------------------------------------*/

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
