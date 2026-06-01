#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <thread>
#include <vector>

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
    bool no_output = false;
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

        }

    }

    return args_stc;
}

// takes resized image input of *p_img* with *img_ch* channel, from index *start* to *end*, and writes the ascii character defined in *table* to *p_art*
// lazy grayscale works by averaging all color channels per pixel. this is by far the fastest method but produces slightly inaccurate output
// precalculated lookup table is used to speed up grayscaled pixel to ascii conversion
// by default this function will be run in parallel to speed up processing, but multithreading overhead kills the performance gain on small data. but we have the framework to do it in parallel at least, in case there's a lot of data to process
void rgb_to_gr_to_art_chunk(char *p_art, unsigned char *p_img, char *table, int img_ch, size_t start, size_t end) {

    for (size_t i = start; i < end; i++) {
        int px_idx = i * img_ch; // this is the base offset value for pixel data. because *p_img pixel data is interleaved (e.g RGB RGB RGB) and we want to access 3 values at a time
        uint8_t r = p_img[px_idx];
        uint8_t g = p_img[px_idx + 1];
        uint8_t b = p_img[px_idx + 2];
        uint8_t avg = (r + g + b) / 3;
        p_art[i] = table[avg];

    }

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
    std::cout << "[!] No art display: " << ((ivscii_args.no_output) ? "yes" : "no") << std::endl << std::endl;

    std::cout << "[V] Load completed!" << std::endl;

    // resize image here
    // rz stands for resized :)
    int rz_height = ivscii_args.nheight;
    int rz_width = ivscii_args.nwidth;
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
    int art_size = rz_width * rz_height;
    char *art_data = (char *)std::malloc(art_size);

    if (art_data == NULL) {
        std::cerr << "[X] Failed to allocate memory for ASCII art" << std::endl;
        stbi_image_free(img_data);
        free(rz_img_data);
        return 1;
    }

    // process pixel data in chunks and in parallel, to speed up processing
    int nproc = std::thread::hardware_concurrency();
    int chunk_size = art_size / nproc;
    std::vector<std::thread> jobs;

    for (size_t i = 0; i < nproc; i++) {
        int start_chunk = i * chunk_size;
        int end_chunk = (i == nproc-1) ? art_size : start_chunk + chunk_size;
        jobs.push_back(std::thread(rgb_to_gr_to_art_chunk, art_data, rz_img_data, lookup_table, rz_channel, start_chunk, end_chunk));

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
            output.append(&art_data[i * rz_width], rz_width);
            output += '\n';
        }

        std::cout << output;

    }

    free(rz_img_data);
    free(art_data);
    stbi_image_free(img_data);

    return 0;

}
