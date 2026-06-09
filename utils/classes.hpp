#ifndef _IVSCII_CLASSES_H_
#define _IVSCII_CLASSES_H_

#include <cstdint>

#include "constants.hpp"

class Args {

    public:
        int sharpness = 4;
        int nwidth = 100;
        int nheight = 36;
        int mode = IVSCII_NORMAL_MODE;
        int color = IVSCII_NO_COLOR_MODE;
        bool no_output = false;
        bool accurate = false;

        //
        Args(int count, char *args[]);

    private:
        long parse_int(int lower, int upper, int def, const char *to_parse);

};

class Image {

    public:
        int width;
        int height;
        int channel;
        uint32_t size;
        uint32_t pixels;
        unsigned char *data;
        bool fail = false;

        ~Image();

        Image(char *img_path);
        Image(int w, int h, int ch);

};

#endif
