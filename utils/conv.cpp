#include "conv.hpp"
#include "constants.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

// takes resized image input of *p_img* with *img_ch* channel, from index *start* to *end*, and writes the ascii character defined in *table* to *p_art*
// lazy grayscale works by averaging all color channels per pixel. this is by far the fastest method but produces slightly inaccurate output
// precalculated lookup table is used to speed up grayscaled pixel to ascii conversion
// by default this function will be run in parallel to speed up processing, but multithreading overhead kills the performance gain on small data. but we have the framework to do it in parallel at least, in case there's a lot of data to process
void rgb_to_gr_to_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end) {

    for (size_t i = start; i < end; i++) {
        int px_idx = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        uint8_t r = p_img[px_idx];
        uint8_t g = p_img[px_idx + 1];
        uint8_t b = p_img[px_idx + 2];
        uint8_t avg = (r + g + b) / 3;

        p_art[i] = table[avg];

    }

}

void rgb_to_agr_to_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end) {

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

void rgb_to_gr_to_truecolor_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end) {

    int iter = 0;
    uint32_t buf_len = ((end - start) * IVSCII_TRUE_COLOR_STRIDE) + 1;
    uint32_t thread_offset = (start * IVSCII_TRUE_COLOR_STRIDE);
    char buf[buf_len];

    for (size_t i = start; i < end; i++) {
        int px_stride = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        int art_stride = iter * IVSCII_TRUE_COLOR_STRIDE; // the amount of stride for each character in the ascii art

        uint8_t r = p_img[px_stride];
        uint8_t g = p_img[px_stride + 1];
        uint8_t b = p_img[px_stride + 2];

        uint8_t gray =  (r + g + b) / 3;
        snprintf(buf + art_stride, 21, "\x1b[38;2;%03d;%03d;%03dm%c", r, g, b, table[gray]);
        iter++;

    }

    std::memcpy(p_art + thread_offset, buf, buf_len-1);

}

void rgb_to_agr_to_truecolor_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end) {

    int iter = 0;
    uint32_t buf_len = ((end - start) * IVSCII_TRUE_COLOR_STRIDE) + 1;
    uint32_t thread_offset = (start * IVSCII_TRUE_COLOR_STRIDE);
    char buf[buf_len];

    for (size_t i = start; i < end; i++) {
        int px_stride = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        int art_stride = iter * IVSCII_TRUE_COLOR_STRIDE; // the amount of stride for each character in the ascii art

        uint8_t r = p_img[px_stride];
        uint8_t g = p_img[px_stride + 1];
        uint8_t b = p_img[px_stride + 2];

        // https://stackoverflow.com/a/29221383
        uint8_t gray =  ((r * 0x4CD) >> 12) +
                        ((g * 0x972) >> 12) +
                        ((b * 0x1C4) >> 12);
        snprintf(buf + art_stride, 21, "\x1b[38;2;%03d;%03d;%03dm%c", r, g, b, table[gray]);
        iter++;

    }

    std::memcpy(p_art + thread_offset, buf, buf_len-1);

}
