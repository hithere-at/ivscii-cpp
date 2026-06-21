#include "classes.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

// parse integer inputs from command args with all bound checkings
long Args::parse_int(int lower, int upper, int def, const char *to_parse) {
    char *endptr;
    const long temp = std::strtol(to_parse, &endptr, 10);

    if (endptr == to_parse) {
        std::cerr << "Switch argument " << to_parse << " is not a valid number\n";
        std::exit(EXIT_FAILURE);
    }

    if ((temp < lower) || (temp > upper)) {
        std::cerr << "Value " << to_parse << " is out of range. Valid range is a value between " << lower << " to " << upper << "\n";
        std::exit(EXIT_FAILURE);

    } else {
        return temp;

    }

}

void Args::show_help(char *name) {
    std::cerr << "Usage: " << name << " [OPTIONS] IMAGE_FILE\n\n";
    std::cerr << "Options:\n";
    std::cerr << "    -w <num>       width of the art. Width limit is 1500\n";
    std::cerr << "    -h <num>       height of the art. Height limit is 200\n";
    std::cerr << "    -s <num>       \"sharpness\" value of the art, controls how gray value should be represented.\n";
    std::cerr << "                   the higher the value the more defined the art is but less accurate in terms of brightness.\n";
    std::cerr << "                   the lower the value the less defined the art is but more accurate it is on the brightness.\n";
    std::cerr << "    -m <1|2>       the render mode of the art. Value of 1 is the normal, and value of 2 is inversed\n";
    std::cerr << "    -n             do not display the ASCII art. Useful to test render time or debugging\n";
    std::cerr << "    -a             use a more accurate grayscale formula. might help with accuracy\n";
    std::cerr << "    -c             colorize the ASCII art. requires a terminal that support 24-bit color (e.g. alacritty, konsole. kitty)\n";
    std::cerr << "    -p             use multithreading to possibly speed up renders. it may or may not speed up the render\n";
    std::cerr << "    -b             fill the color on the background instead of foreground. highly recommended, but this is disabled by default\n";
    std::cerr << "    --help         show this help message\n";

}

// Args constructor, also works as a command line parser
// TODO: use strncmp for safety reasons
// TODO: exit when argument is invalid
Args::Args(int count, char *args[]) {

    if (count <= 2) {
        show_help(args[0]);
        std::exit(EXIT_SUCCESS);

    }

    // loop through all switches
    for (size_t i = 1; i < count-1; i++) {

        // parse help flag
        if (strcmp(args[i], "--help") == 0) {
            show_help(args[0]);
            std::exit(EXIT_SUCCESS);

        // parse sharpness arg
        } else if (strcmp(args[i], "-s") == 0) {
            this->sharpness = parse_int(0, 255, 4, args[++i]);

        // parse width arg
        } else if (strcmp(args[i], "-w") == 0) {
            this->nwidth = parse_int(0, 1500, 100, args[++i]);

        // parse height arg
        } else if (strcmp(args[i], "-h") == 0) {
            this->nheight = parse_int(0, 200, 36, args[++i]);

        // parse mode arg
        } else if (strcmp(args[i], "-m") == 0) {
            this->mode = parse_int(1, 2, 1, args[++i]);

        // parse no output args flag
        } else if (strcmp(args[i], "-n") == 0) {
            this->no_output = true;

        // parse accurate grayscale args flag
        } else if (strcmp(args[i], "-a") == 0) {
            this->accurate = true;

        // parse color output type args
        } else if (strcmp(args[i], "-c") == 0) {
            this->color = true;

        // parse multithread args flag
        } else if (strcmp(args[i], "-p") == 0) {
            this->multithread = true;

        // parse background color fill args flag
        } else if (strcmp(args[i], "-b") == 0) {
            this->fill_bg = true;

        } else {
            std::cerr << "Unknown option \"" << args[i] << "\"\n";
            std::cerr << "Please run \"" << args[0] << " --help\" for more information\n";
            std::exit(EXIT_FAILURE);

        }

    }

}

// constructors for creating an Image from image path
Image::Image(char *img_path) {
    this->data = stbi_load(img_path, &this->width, &this->height, &this->channel, 3);
    this->pixels = this->width * this->height;
    this->size = this->pixels * this->channel;

    if (this->data == NULL) {
        this->fail = true;

    }

}

// constructors for creating an empty Image
Image::Image(int w, int h, int ch) {
    this->width = w;
    this->height = h;
    this->channel = ch;
    this->pixels = w * h;
    this->size = this->pixels * ch;
    this->data = (unsigned char *)std::malloc(this->size);

    if (this->data == NULL) {
        this->fail = true;

    }

}

// deconstructor to deallocate memory
// its probably safer deallocate a memory based on the way how the Image class is constructored
// because stbi_load naturally should be deallocated by stbi_image_free. but stbi_header said that STBI_MALLOC os just normal malloc
// sooo it should be okay to use stdlib free()
Image::~Image() {
    free(this->data);
}
