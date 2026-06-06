#include "conv.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>

// takes resized image input of *p_img* with *img_ch* channel, from index *start* to *end*, and writes the ascii character defined in *table* to *p_art*
// lazy grayscale works by averaging all color channels per pixel. this is by far the fastest method but produces slightly inaccurate output
// precalculated lookup table is used to speed up grayscaled pixel to ascii conversion
// by default this function will be run in parallel to speed up processing, but multithreading overhead kills the performance gain on small data. but we have the framework to do it in parallel at least, in case there's a lot of data to process
void rgb_to_gr_to_art_chunk(char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end) {

    for (size_t i = start; i < end; i++) {
        int px_idx = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        uint8_t r = p_img[px_idx];
        uint8_t g = p_img[px_idx + 1];
        uint8_t b = p_img[px_idx + 2];
        uint8_t avg = (r + g + b) / 3;

        p_art[i] = table[avg];

    }

}

void rgb_to_agr_to_art_chunk(char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end) {

    for (size_t i = start; i < end; i++) {
        int px_idx = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        uint8_t r = p_img[px_idx];
        uint8_t g = p_img[px_idx + 1];
        uint8_t b = p_img[px_idx + 2];

        // https://stackoverflow.com/a/29221383
        uint8_t gray =  ((r * 0x4CD) >> 12) +
                        ((g * 0x972) >> 12) +
                        ((b * 0x1C4) >> 12);

        p_art[i] = table[gray];

    }

}

void rgb_to_gr_to_truecolor_art_chunk(char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end, int id) {

    for (size_t i = start; i < end; i++) {
        int px_stride = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        int art_stride = i * 20; // the amount of stride for each character in the ascii art

        uint8_t r = p_img[px_stride];
        uint8_t g = p_img[px_stride + 1];
        uint8_t b = p_img[px_stride + 2];

        uint8_t gray =  (r + g + b) / 3;
        snprintf(p_art + art_stride, 21, "\x1b[38;2;%03d;%03d;%03dm%c", r, g, b, table[gray]);

    }

}

void rgb_to_agr_to_truecolor_art_chunk(char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end, int id) {

    for (size_t i = start; i < end; i++) {
        int px_stride = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        int art_stride = i * 20; // the amount of stride for each character in the ascii art

        uint8_t r = p_img[px_stride];
        uint8_t g = p_img[px_stride + 1];
        uint8_t b = p_img[px_stride + 2];

        // https://stackoverflow.com/a/29221383
        uint8_t gray =  ((r * 0x4CD) >> 12) +
                        ((g * 0x972) >> 12) +
                        ((b * 0x1C4) >> 12);
        snprintf(p_art + art_stride, 21, "\x1b[38;2;%03d;%03d;%03dm%c", r, g, b, table[gray]);

    }

}
