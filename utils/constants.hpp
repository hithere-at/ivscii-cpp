#ifndef _IVSCII_CONSTANTS_H_
#define _IVSCII_CONSTANTS_H_

// the quantization factor is the amount gray value that can be represented by each ASCII character
// this means that for each 4 gray value, it can be represented by an ASCII character
#define IVSCII_QUANTIZATION_FACTOR 4

// this should be pretty obvious. but the user need to pass the number when specify modes using the -m flag
#define IVSCII_NORMAL_MODE 1

// this one too.
#define IVSCII_INVERSED_MODE 2

// and also this one. but this time its for -c flag
#define IVSCII_NO_COLOR_MODE 0

// yes, this one too
#define IVSCII_8BIT_COLOR_MODE 1

// you're right again.
#define IVSCII_TRUE_COLOR_MODE 2

// this defines the amount of strides needed to reach the next ASCII character
// when the user specify color mode of 0 (IVSCII_NO_COLOR_MODE), then the stride to reah the next character is 1
#define IVSCII_NO_COLOR_STRIDE 1

// for 8 bit colors (256), then the amount of stride needed to reach the next character is 12
// this is because the escape code ""\x1b[38;5;NNNm" (without quotes) length is 11 + one ASCII character for the art
#define IVSCII_8BIT_COLOR_STRIDE 12

// for 24 bit colors (16.7 million colors), then the amount of stride needed to reach the next character is 19
// this is because the escape code ""\x1b[38;2;RRR;GGG;BBBm" (without quotes) length is 19 + one ASCII character for the art
#define IVSCII_TRUE_COLOR_STRIDE 20

// the default color channel to be used by the program
#define IVSCII_RGB 3

#endif
