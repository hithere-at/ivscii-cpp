#ifndef _IVSCII_CLASSES_H_
#define _IVSCII_CLASSES_H_

#include <cstdint>

#include "constants.hpp"

// the Args class is used to store informations about all the arguments that is passed and used by the progra,
// we gave a default value for each options in case the user doenst pass anything or fail to input proprely
class Args {

    public:
        int sharpness = 4; // sharpness is the offset value used by the lookup tables. the higher the number the less accurate it is relative to the gray value, but produces much cleaner output
        int nwidth = 100; // the width of the ASCII art
        int nheight = 36; // the height of the ASCII art
        int mode = IVSCII_NORMAL_MODE; // grayscale render mode
        bool color = false; // color output type
        bool fill_bg = false;
        bool no_output = false; // display the ASCII art or not
        bool accurate = false; // use an accurate grayscale method or not
        bool multithread = false; // multithread ASCII art render or not

        // constructor for the class, as well as parsing the command line arguments
        Args(int count, char *args[]);

    private:
        long parse_int(int lower, int upper, int def, const char *to_parse);
        void show_help(char *name);

};

// Image is used to store the image data decoded by stb_image. this class can also be used to store the ASCII art itself
// to do that, the channel attribute is used to store the lenght of each ASCII art character. this could vary depending on the color output type
class Image {

    public:
        int width; // the width of the image
        int height; // the height of the image
        int channel; // the amount of channel/stride for each pixel
        uint32_t size; // the size of the image. calculated by multiplying width, height, and channel together
        uint32_t pixels; // the amount of pixels the image has. calculated by multiplying width and height
        unsigned char *data; // the pixel data of the image
        bool fail = false; // flag to determine whether an image is parsed correctly or not

        // deconstructor for the image clas. free's memory allocation after execution has finished
        ~Image();

        Image(char *img_path); // constructor to load an image from image path
        Image(int w, int h, int ch); // constructor to create an empty Image. used when manual handling of Image data is required

};

// ConvThreadInfo is used to store informations needed for worker threads when doing conversion
struct ConvThreadInfo {
    unsigned char *art_ptr; // pointer to the ASCII art buffer
    unsigned char *img_ptr; // pointer to the resized image buffer
    char *gray_table; // pointer to the generated lookup table
    int img_ch; // image channel
    uint32_t start; // index of the image pixel to start reading from
    uint32_t end; // index of the image pixel to stop reading from
    char fill_mode_id;
    bool is_acc_gray; // to determine whether to use an accurate grayscale formula

};

#endif
