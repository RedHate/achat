
/**
  * Debug print buffer byte for byte as hex
  * 
  * @param buffer    - buffer to be printed
  * @param size      - size in bytes
  */
void debug_print_buf(uint8_t *buffer, uint32_t size);

/**
  * Draw a waterfall audio wave effect to stdout
  * 
  * @param buffer     - buffer to be printed
  * @param size       - size in bytes
  * @param color_mode - 0 or 1
  */
void audio_visualiser(short *buffer, size_t size, int color_mode);

