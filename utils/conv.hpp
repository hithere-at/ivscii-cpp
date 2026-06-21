#ifndef _IVSCII_CONV_H_
#define _IVSCII_CONV_H_

#include "classes.hpp"

// all this functions takes resized image input of *p_img* with *img_ch* channel, from index *start* to *end*, and writes the ascii character defined in *table* to *p_art*
// lazy grayscale works by averaging all color channels per pixel. this is by far the fastest method but produces slightly inaccurate output
// precalculated lookup table is used to speed up grayscaled pixel to ascii conversion
// by default all functions written here will be run in parallel to speed up processing, but multithreading overhead kills the performance gain on small data. but we have the framework to do it in parallel at least, in case there's a lot of data to process

// RGB to grayscale to ASCII art function
void rgb_to_gr_to_art_chunk(struct ConvThreadInfo info);

// RGB to grayscale to 24-bit colored ASCII art function
void rgb_to_gr_to_truecolor_art_chunk(struct ConvThreadInfo info);

#endif
