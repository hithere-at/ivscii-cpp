#include "conv.hpp"
#include "constants.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

// format a number to 3 digits, 5 becomes 005, 74 becomes 074, etc.
void format_num(char* p, uint8_t val) {
    *(p) = '0' + (val / 100);
    *(p + 1) = '0' + ((val / 10) % 10);
    *(p + 2) = '0' + (val % 10);
}

// the gray function. takes RGB input and calculate the grayscale
// if acc is true then it will use a more accurate grayscale method
// otherwise, use the average method
uint8_t grayscale(uint8_t r, uint8_t g, uint8_t b, bool acc) {
    return (acc) ? (((r * 0x4CD) >> 12) +
                    ((g * 0x972) >> 12) +
                    ((b * 0x1C4) >> 12))
                 :
                 ((r + g + b) / 3);
    ;

}

// RGB to grayscale to ASCII art function (no color)
void rgb_to_gr_to_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end, bool accurate) {

    for (size_t i = start; i < end; i++) {
        int px_idx = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        uint8_t r = p_img[px_idx];
        uint8_t g = p_img[px_idx + 1];
        uint8_t b = p_img[px_idx + 2];

        uint8_t gray = grayscale(r, g, b, accurate);

        p_art[i] = table[gray];

    }

}

// RGB to grayscale to 24-bit color ASCII art
// originally this function is using snprintf to to make implementation easier but after doing some profiling.
// we found that 25% of the time spent by the program is wasted on millions printf calls. so we decided to just build string manually instead
void rgb_to_gr_to_truecolor_art_chunk(unsigned char *p_art, unsigned char *p_img, char *table, int img_ch, uint32_t start, uint32_t end, bool accurate) {

    // initialize variables to help manual art building
    int iter = 0;
    uint32_t buf_len = ((end - start) * IVSCII_TRUE_COLOR_STRIDE);
    uint32_t write_offset = (start * IVSCII_TRUE_COLOR_STRIDE);
    char buf[buf_len];

    for (size_t i = start; i < end; i++) {
        int px_stride = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        int art_stride = iter * IVSCII_TRUE_COLOR_STRIDE; // the amount of stride for each character in the ascii art

        uint8_t r = p_img[px_stride];
        uint8_t g = p_img[px_stride + 1];
        uint8_t b = p_img[px_stride + 2];

        uint8_t gray = grayscale(r, g, b, accurate);

        // manually build string instead of using snprintf. shaves around 5ms of render time. worth it? not really but hey it looks cooler
        char *ptr = buf + art_stride;
        ptr[0] = '\x1b';
        ptr[1] = '[';
        ptr[2] = '3';
        ptr[3] = '8';
        ptr[4] = ';';
        ptr[5] = '2';
        ptr[6] = ';';
        format_num(ptr + 7, r);
        ptr[10] = ';';
        format_num(ptr + 11, g);
        ptr[14] = ';';
        format_num(ptr + 15, b);
        ptr[18] = 'm';
        ptr[19] = table[gray];

        // snprintf(buf + art_stride, 21, "\x1b[38;2;%03d;%03d;%03dm%c", r, g, b, table[gray]);
        iter++;

    }

    // lasly copy the thread buffer back to the art buffer
    std::memcpy(p_art + write_offset, buf, buf_len);

}
