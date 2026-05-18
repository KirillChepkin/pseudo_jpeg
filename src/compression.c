#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bmp.h"
#include "macros.h"

int get_padding(int x, int d) {
    
}

void downsample_component(unsigned char* c, unsigned char* dest, int height, int width) {
    /* for (int i = 0; i < height - height % 2; i += 2) {
        for (int j = 0; j < width - width % 2; j += 2) {
            dest[RM_INDEX(width, i / 2, j / 2)] = (c[RM_INDEX(width, i, j)] +
                                                   c[RM_INDEX(width, i + 1, j) +
                                                   c[RM_INDEX(width, i, j + 1)]] +
                                                   c[RM_INDEX(width, i + 1, j + 1)]) / 4;
        }
    } */
    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width / 2; j++) {
            dest[RM_INDEX(width, i, j)] = (c[RM_INDEX(width, i * 2, j * 2)] +
                                           c[RM_INDEX(width, i * 2, j * 2 + 1)] +
                                           c[RM_INDEX(width, i * 2 + 1, j * 2)] +
                                           c[RM_INDEX(width, i * 2 + 1, j * 2 + 1)]) / 4;
        }
    }
}