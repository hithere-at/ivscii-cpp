#ifndef _IVSCII_CONV_H_
#define _IVSCII_CONV_H_

#include <cstdint>

// all this functions takes resized image input of *p_img* with *img_ch* channel, from index *start* to *end*, and writes the ascii character defined in *table* to *p_art*
// lazy grayscale works by averaging all color channels per pixel. this is by far the fastest method but produces slightly inaccurate output
// precalculated lookup table is used to speed up grayscaled pixel to ascii conversion
// by default all functions written here will be run in parallel to speed up processing, but multithreading overhead kills the performance gain on small data. but we have the framework to do it in parallel at least, in case there's a lot of data to process

// RGB to grayscale to ASCII art function
void rgb_to_gr_to_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end, bool accurate);

// RGB to accurate grayscale to ASCII art function
// void rgb_to_agr_to_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end);

// RGB to grayscale to 24-bit colored ASCII art function
void rgb_to_gr_to_truecolor_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end, bool accurate, bool multithread);

#endif
