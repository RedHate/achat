
/**
  * Opus decoder encoder init
  */
int init_opus();

/**
  * Opus decoder encoder shutdown
  */
void deinit_opus();


/**
  * Encode pcm buffer to opus buffer
  *
  * @param pcm - input buffer
  * @param frame_size - size in bytes (use framesize not sizeof)
  * @param data - opus buffer
  * @param max_size - size in bytes (use framesize not sizeof)
  * @return 0 on success, an error if greater than 0.
  */
int opus_encode_buffer(const opus_int16 *pcm_buf, int frame_size, unsigned char *opus_buf, opus_int32 max_size);

/**
  * Decode opus buffer to 16bit pcm buffer
  *
  * @param opus_buf - input buffer
  * @param len - size of opus packed buffer (must be known ahead of time but sizeof(buffer) should work)
  * @param pcm_buf - opus buffer
  * @param frame_size - size in bytes (use framesize not sizeof)
  * @return 0 on success, an error if greater than 0.
  */
int opus_decode_buffer(const unsigned char *opus_buf, opus_int32 len, opus_int16 *pcm_buf, int frame_size);	
