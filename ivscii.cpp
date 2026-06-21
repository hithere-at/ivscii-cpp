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
    // for example, for each 4 color channel value can be represented in an ascii table that has 71 characters
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
        std::cerr << "[X] Failed to load image data\n";
        return 1;
    }

    // this outputs the information of the ASCII art
    std::cerr << "[!] Input resolution: " << og_image.width << "x" << og_image.height << "\n";
    std::cerr << "[!] Output resolution: " << ivscii_args.nwidth << "x" << ivscii_args.nheight << "\n";
    std::cerr << "[!] Sharpness: " << ivscii_args.sharpness << "\n";
    std::cerr << "[!] Color mode: " << ivscii_args.color << "\n";
    std::cerr << "[!] Color fill: " << ((ivscii_args.fill_bg) ? "background" : "foreground") << "\n";
    std::cerr << "[!] Mode: " << ivscii_args.mode << "\n";
    std::cerr << "[!] No art display: " << ((ivscii_args.no_output) ? "yes" : "no") << "\n";
    std::cerr << "[!] Accurate grayscale: " << ((ivscii_args.accurate) ? "yes" : "no") << "\n";
    std::cerr << "[!] Multithread: " << ((ivscii_args.multithread) ? "yes" : "no") << "\n\n";

    std::cerr << "[V] Image loaded!\n";

    // resize image here
    // rz stands for resized :)
    Image rz_image(ivscii_args.nwidth, ivscii_args.nheight, IVSCII_RGB);

    if (rz_image.fail) {
        std::cerr << "[X] Failed to allocate memory for resized image data!\n";
        return 1;
    }

    // resize the image to match the width and the height of the specified resolution for easier parsing
    stbir_resize_uint8_linear(og_image.data,og_image.width, og_image.height, 0, rz_image.data, rz_image.width, rz_image.height, 0, STBIR_RGB);
    std::cerr << "[V] Resize completed!" << std::endl;

    // add some more space for escape code
    int stride = (ivscii_args.color) ? IVSCII_TRUE_COLOR_STRIDE : IVSCII_NO_COLOR_STRIDE;

    Image art(rz_image.width, rz_image.height, stride);

    if (art.fail) {
        std::cerr << "[X] Failed to allocate memory for ASCII art\n";
        return 1;
    }

    // calculate how much thread should be used
    // the extra code on the if statement is used to handle when std::thread::hardware_concurrency returns

    if (ivscii_args.multithread) {

        // calculate how much thread should be used
        // the extra code on the if statement is used to handle when std::thread::hardware_concurrency returns
        int nproc = (std::thread::hardware_concurrency() != 0) ? std::thread::hardware_concurrency() : 1;

        // process pixel data in chunks and in parallel, to speed up processing
        // also calculates how much pixels should be processed by each thread
        // and which functions to run depending on the color output type specified by the user
        uint32_t chunk_size = art.pixels / nproc;
        std::thread jobs[nproc];

        for (int i = 0; i < nproc; i++) {
            uint32_t start_chunk = i * chunk_size;
            uint32_t end_chunk = (i == nproc-1) ? art.pixels : start_chunk + chunk_size;

            struct ConvThreadInfo conv_info = {
                .art_ptr = art.data,
                .img_ptr = rz_image.data,
                .gray_table = lookup_table,
                .img_ch = rz_image.channel,
                .start = start_chunk,
                .end = end_chunk,
                .fill_mode_id = (ivscii_args.fill_bg) ? IVSCII_BG_COLOR_FILL : IVSCII_FG_COLOR_FILL,
                .is_acc_gray = ivscii_args.accurate

            };

            if (ivscii_args.color) {
                jobs[i] = std::thread(rgb_to_gr_to_truecolor_art_chunk, conv_info);

            } else {
                jobs[i] = std::thread(rgb_to_gr_to_art_chunk, conv_info);

            }

        }

        // start the threads
        for (int i = 0; i < nproc; i++) {
            jobs[i].join();
        }

    // do not use std::thread and call the function directly insteda when user does not want to multithread. this, hopefully, reduces overhead
    } else {

        struct ConvThreadInfo conv_info = {
            .art_ptr = art.data,
            .img_ptr = rz_image.data,
            .gray_table = lookup_table,
            .img_ch = rz_image.channel,
            .start = 0,
            .end = art.pixels,
            .fill_mode_id = (ivscii_args.fill_bg) ? IVSCII_BG_COLOR_FILL : IVSCII_FG_COLOR_FILL,
            .is_acc_gray = ivscii_args.accurate,

        };

        if (ivscii_args.color) {
            rgb_to_gr_to_truecolor_art_chunk(conv_info);

        } else {
            rgb_to_gr_to_art_chunk(conv_info);

        }

    }

    std::cerr << "[V] Grayscale and art drawing completed!\n";

    // display start here
    std::string output;
    output.reserve(art.size + art.height);

    if (!ivscii_args.no_output) {

        for (int i = 0; i < art.height; i++) {
            // this code prints the ASCII art per line
            output.append((char *)&art.data[i * art.channel * art.width], art.channel * art.width);
            output += '\n';
        }

        std::cout << output;

    }

    return 0;

}
