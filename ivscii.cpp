#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <thread>

#include "utils/classes.hpp"
#include "utils/conv.hpp"
#include "utils/constants.hpp"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "lib/stb_image_resize2.h"

const std::string GREYSCALE_CHARS = " .'`^\",:;Il!i><~+_-?][}{1)(|\\\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

int main(int argc, char *argv[]) {

    Args ivscii_args(argc, argv);
    char *img_path = argv[argc-1];

    // create a lookup table to directly map pixel data with ascii character. saves many division operations
    // the magic value '4' in this case is the interval of the how many channel data can be represented in a single ascii character
    // for example, for each 4 color channel value can be represented in an ascii table that has 70 characters
    char lookup_table[256];

    if (ivscii_args.mode == IVSCII_NORMAL_MODE) {
        for (size_t i = 0; i < 256; i++) {
            lookup_table[i] = GREYSCALE_CHARS[i / (IVSCII_QUANTIZATION_FACTOR + ivscii_args.sharpness)];

        }

    // this inverted rendering mode. basically the reverse of the normal mode. black become lighter and white becomes darker
    } else {
        for (size_t i = 0; i < 256; i++) {
            lookup_table[i] = GREYSCALE_CHARS[(255 - i) / (IVSCII_QUANTIZATION_FACTOR + ivscii_args.sharpness)];
        }

    }

    // image loading here
    Image og_image(img_path);

    if (og_image.fail) {
        std::cerr << "[X] Failed to load image data: " << std::endl;
        return 1;
    }

    std::cout << "[!] Input resolution: " << og_image.width << "x" << og_image.height << std::endl;
    std::cout << "[!] Output resolution: " << ivscii_args.nwidth << "x" << ivscii_args.nheight << std::endl;
    std::cout << "[!] Sharpness: " << ivscii_args.sharpness << std::endl;
    std::cout << "[!] Mode: " << ivscii_args.mode << std::endl;
    std::cout << "[!] No art display: " << ((ivscii_args.no_output) ? "yes" : "no") << std::endl;
    std::cout << "[!] Accurate grayscale: " << ((ivscii_args.accurate) ? "yes" : "no") << std::endl;
    std::cout << "[!] Color mode: " << ivscii_args.color << std::endl << std::endl;

    std::cout << "[V] Load completed!" << std::endl;

    // resize image here
    // rz stands for resized :)
    Image rz_image(ivscii_args.nwidth, ivscii_args.nheight, IVSCII_RGB);

    if (rz_image.fail) {
        std::cerr << "[X] Failed to allocate memory for resized image data!" << std::endl;
        return 1;
    }

    // resized the image to match the width and the height of the specified resolution for easier parsing
    stbir_resize_uint8_linear(og_image.data,og_image.width, og_image.height, 0, rz_image.data, rz_image.width, rz_image.height, 0, STBIR_RGB);
    std::cout << "[V] Resize completed!" << std::endl;

    // add some more space for escape code
    int esc_code_len = IVSCII_NO_COLOR_STRIDE;

    if (ivscii_args.color == IVSCII_8BIT_COLOR_MODE) {
        esc_code_len = IVSCII_8BIT_COLOR_STRIDE;

    } else if (ivscii_args.color == IVSCII_TRUE_COLOR_MODE) {
        esc_code_len = IVSCII_TRUE_COLOR_STRIDE;
    }

    Image art(rz_image.width, rz_image.height, esc_code_len);

    if (art.fail) {
        std::cerr << "[X] Failed to allocate memory for ASCII art" << std::endl;
        return 1;
    }

    // process pixel data in chunks and in parallel, to speed up processing
    // int nproc = (ivscii_args.color == IVSCII_TRUE_COLOR_MODE) ? 1 : std::thread::hardware_concurrency();
    int nproc = std::thread::hardware_concurrency();
    uint32_t chunk_size = art.pixels / nproc;
    std::thread jobs[nproc];

    for (int i = 0; i < nproc; i++) {
        uint32_t start_chunk = i * chunk_size;
        uint32_t end_chunk = (i == nproc-1) ? art.pixels : start_chunk + chunk_size;

        if (ivscii_args.color == IVSCII_NO_COLOR_MODE && !ivscii_args.accurate) {
            jobs[i] = std::thread(rgb_to_gr_to_art_chunk, art.data, rz_image.data, lookup_table, rz_image.channel, start_chunk, end_chunk);

        } else if (ivscii_args.color == IVSCII_NO_COLOR_MODE && ivscii_args.accurate) {
            jobs[i] = std::thread(rgb_to_agr_to_art_chunk, art.data, rz_image.data, lookup_table, rz_image.channel, start_chunk, end_chunk);

        } else if (ivscii_args.color == IVSCII_TRUE_COLOR_MODE && !ivscii_args.accurate) {
            jobs[i] = std::thread(rgb_to_gr_to_truecolor_art_chunk, art.data, rz_image.data, lookup_table, rz_image.channel, start_chunk, end_chunk);

        } else if (ivscii_args.color == IVSCII_TRUE_COLOR_MODE && ivscii_args.accurate) {
            jobs[i] = std::thread(rgb_to_agr_to_truecolor_art_chunk, art.data, rz_image.data, lookup_table, rz_image.channel, start_chunk, end_chunk);

        }

    }

    for (size_t i = 0; i < nproc; i++) {
        jobs[i].join();
    }

    std::cout << "[V] Grayscale and art drawing completed!" << std::endl;

    // display start here
    std::string output;
    output.reserve(art.size + art.height);

    if (!ivscii_args.no_output) {

        for (size_t i = 0; i < art.height; i++) {
            output.append((char *)&art.data[i * art.channel * art.width], art.channel * art.width);
            output += '\n';
        }

        std::cout << output;

    }

    return 0;

}
