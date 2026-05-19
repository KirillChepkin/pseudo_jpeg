#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bmp.h"
#include "macros.h"

int get_padding_size(int x, int d) {
    // computes a number which is >= x and % d
    if (x % d == 0) {
        return x;
    }
    return x + (d - x % d);
}

void pad_component(unsigned char* c, int height, int width, int new_height, int new_width) {
    // this function fills the image symetrically so that it has height % 8 and width % 8
    // height and width are from the original image
    // the c matrix has new_height and new_width
    // height and width are supposed to be positive

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < (new_height - height); j++) {
            c[RM_INDEX(new_width, height + j, i)] = c[RM_INDEX(new_width, height - j - 1, i)];
        }
    }
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < (new_width - width); j++) {
            c[RM_INDEX(new_width, i, width + j)] = c[RM_INDEX(new_width, i, width - j - 1)];
        }
    }
}

void downsample_component(unsigned char* c, unsigned char* dest, int height, int width) {
    // decreases the resolution of the specific image component by the factor of 2
    // expects to get the padded component, otherwise odd pixels will be ignored
    // is able to write the result both to the existing component row-major matrix
    // and to a separate piece of memory

    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width / 2; j++) {
            dest[RM_INDEX(width, i, j)] = (c[RM_INDEX(width, i * 2, j * 2)] +
                                           c[RM_INDEX(width, i * 2, j * 2 + 1)] +
                                           c[RM_INDEX(width, i * 2 + 1, j * 2)] +
                                           c[RM_INDEX(width, i * 2 + 1, j * 2 + 1)]) / 4;
        }
    }
}