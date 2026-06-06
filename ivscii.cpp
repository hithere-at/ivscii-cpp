#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cerrno>
#include <thread>
#include <vector>

#include "utils/conv.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "lib/stb_image_resize2.h"

const std::string GREYSCALE_CHARS = " .'`^\",:;Il!i><~+_-?][}{1)(|\\\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

struct args {
    int sharpness = 4;
    int nwidth = 100;
    int nheight = 36;
    int mode = 2;
    int color = 0;
    bool no_output = false;
    bool accurate = false;
};

// parse integer inputs from command args with all bound checkings
long parse_int(int lower, int upper, int def, const char *to_parse) {
    char *endptr;
    const long temp = std::strtol(to_parse, &endptr, 10);
    return (
        (errno == ERANGE) ||
        (endptr == to_parse) ||
        (temp < lower) ||
        (temp > upper)
    ) ? def : temp;

}

// command args parser
// TODO: use strncmp for safety reasons
// TODO: exit when argument is invalid
struct args parse_argv(int count, char *args[]) {

    struct args args_stc;

    for (size_t i = 0; i < count; i++) {

        // parse sharpness arg
        if (strcmp(args[i], "-s") == 0) {
            args_stc.sharpness = parse_int(0, 255, 4, args[++i]);

        // parse width arg
        } else if (strcmp(args[i], "-w") == 0) {
            args_stc.nwidth = parse_int(0, 1000, 100, args[++i]);

        // parse height arg
        } else if (strcmp(args[i], "-h") == 0) {
            args_stc.nheight = parse_int(0, 1000, 36, args[++i]);

        // parse mode arg
        } else if (strcmp(args[i], "-m") == 0) {
            args_stc.mode = parse_int(1, 2, 1, args[++i]);

        } else if (strcmp(args[i], "-n") == 0) {
            args_stc.no_output = true;

        } else if (strcmp(args[i], "-a") == 0) {
            args_stc.accurate = true;

        } else if (strcmp(args[i], "-c") == 0) {
            args_stc.color = parse_int(0, 2, 0, args[++i]);

        }

    }

    return args_stc;
}

int main(int argc, char *argv[]) {

    struct args ivscii_args = parse_argv(argc, argv);
    char *img_path = argv[argc-1];

    // create a lookup table to directly map pixel data with ascii character. saves many division operations
    // the magic value '4' in this case is the interval of the how many channel data can be represented in a single ascii character
    // for example, for each 4 color channel value can be represented in an ascii table that has 70 characters
    char lookup_table[256];

    if (ivscii_args.mode == 1) {
        for (size_t i = 0; i < 256; i++) {
            lookup_table[i] = GREYSCALE_CHARS[i / (4 + ivscii_args.sharpness)];

        }

    // this inverted rendering mode. basically the reverse of the normal mode. black become lighter and white becomes darker
    } else {
        for (size_t i = 0; i < 256; i++) {
            lookup_table[i] = GREYSCALE_CHARS[(255 - i) / (4 + ivscii_args.sharpness)];
        }

    }

    // loading image here
    int width, height, channel;
    unsigned char *img_data = stbi_load(img_path, &width, &height, &channel, 3);

    if (img_data == NULL) {
        std::cerr << "[X] Failed to load image data: ";
        std::cerr << stbi_failure_reason() << std::endl;
        return 1;
    }

    std::cout << "[!] Input resolution: " << width << "x" << height << std::endl;
    std::cout << "[!] Output resolution: " << ivscii_args.nwidth << "x" << ivscii_args.nheight << std::endl;
    std::cout << "[!] Sharpness: " << ivscii_args.sharpness << std::endl;
    std::cout << "[!] Mode: " << ivscii_args.mode << std::endl;
    std::cout << "[!] No art display: " << ((ivscii_args.no_output) ? "yes" : "no") << std::endl;
    std::cout << "[!] Accurate grayscale: " << ((ivscii_args.accurate) ? "yes" : "no") << std::endl;
    std::cout << "[!] Color mode: " << ivscii_args.color << std::endl << std::endl;

    std::cout << "[V] Load completed!" << std::endl;

    // resize image here
    // rz stands for resized :)
    int rz_height = ivscii_args.nheight;
    int rz_width = ivscii_args.nwidth;
    int rz_img_pixels = rz_width * rz_height;
    int rz_channel = 3;
    int rz_img_size = rz_width * rz_height * rz_channel;
    unsigned char *rz_img_data = (unsigned char *)std::malloc(rz_img_size);

    if (rz_img_data == NULL) {
        std::cerr << "[X] Failed to allocate memory for resized image data!" << std::endl;
        stbi_image_free(img_data);
        return 1;
    }

    // resized the image to match the width and the height of the specified resolution for easier parsing
    stbir_resize_uint8_linear(img_data, width, height, 0, rz_img_data, rz_width, rz_height, 0, STBIR_RGB);
    std::cout << "[V] Resize completed!" << std::endl;

    // grayscaling and art drawing starts here
    uint32_t art_size = rz_img_pixels;
    int art_data_stride = 1;

    // add some more space for escape code
    if (ivscii_args.color == 1) {
        art_size += (art_size * 11); // space for 256 color mode. value 11 stands for the amount of bytes needed for the color escape code
        art_data_stride = 12;

    } else if (ivscii_args.color == 2) {
        art_size += (art_size * 19); // space for 24-bit color mode. value 19 stands for the amount of bytes needed for the color escape code
        art_data_stride = 20;

    }

    char *art_data = (char *)std::malloc(art_size);
    std::cout << art_size << std::endl;

    if (art_data == NULL) {
        std::cerr << "[X] Failed to allocate memory for ASCII art" << std::endl;
        stbi_image_free(img_data);
        free(rz_img_data);
        return 1;
    }

    // process pixel data in chunks and in parallel, to speed up processing
    int nproc = (ivscii_args.color == 2) ? 1 : std::thread::hardware_concurrency();
    uint32_t chunk_size = rz_img_pixels / nproc;
    std::vector<std::thread> jobs;

    for (int i = 0; i < nproc; i++) {
        uint32_t start_chunk = i * chunk_size;
        start_chunk = (i == 0) ? start_chunk : start_chunk;
        uint32_t end_chunk = (i == nproc-1) ? rz_img_pixels : start_chunk + chunk_size;

        if (ivscii_args.color == 0 && !ivscii_args.accurate) {
            jobs.push_back(std::thread(rgb_to_gr_to_art_chunk, art_data, rz_img_data, lookup_table, rz_channel, start_chunk, end_chunk));

        } else if (ivscii_args.color == 0 && ivscii_args.accurate) {
            jobs.push_back(std::thread(rgb_to_agr_to_art_chunk, art_data, rz_img_data, lookup_table, rz_channel, start_chunk, end_chunk));

        } else if (ivscii_args.color == 2 && !ivscii_args.accurate) {
            jobs.push_back(std::thread(rgb_to_gr_to_truecolor_art_chunk, art_data, rz_img_data, lookup_table, rz_channel, start_chunk, end_chunk, i));

        } else if (ivscii_args.color == 2 && ivscii_args.accurate) {
            jobs.push_back(std::thread(rgb_to_agr_to_truecolor_art_chunk, art_data, rz_img_data, lookup_table, rz_channel, start_chunk, end_chunk, i));

        }

    }

    for (size_t i = 0; i < nproc; i++) {
        jobs[i].join();
    }

    std::cout << "[V] Grayscale and art drawing completed!" << std::endl;

    // display start here
    std::string output;
    output.reserve(art_size + rz_height);

    if (!(ivscii_args.no_output)) {

        for (size_t i = 0; i < rz_height; i++) {
            output.append(&art_data[i * art_data_stride * rz_width], art_data_stride * rz_width);
            output += '\n';
        }

        std::cout << output;

    }

    free(rz_img_data);
    free(art_data);
    stbi_image_free(img_data);

    return 0;

}
