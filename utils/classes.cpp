#include "classes.hpp"

#include <cstdlib>
#include <cstring>
#include <cerrno>

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

// parse integer inputs from command args with all bound checkings
long Args::parse_int(int lower, int upper, int def, const char *to_parse) {
    char *endptr;
    const long temp = std::strtol(to_parse, &endptr, 10);
    return (
        (errno == ERANGE) ||
        (endptr == to_parse) ||
        (temp < lower) ||
        (temp > upper)
    ) ? def : temp;

}

// Args constructor, also works as a command line parser
// TODO: use strncmp for safety reasons
// TODO: exit when argument is invalid
Args::Args(int count, char *args[]) {

    for (size_t i = 0; i < count; i++) {

        // parse sharpness arg
        if (strcmp(args[i], "-s") == 0) {
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
            this->color = parse_int(0, 2, 0, args[++i]);

        // parse multithread args flag
        } else if (strcmp(args[i], "-p") == 0) {
            this->multithread = true;

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
