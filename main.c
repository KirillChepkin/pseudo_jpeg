#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bmp.h"
#include "compression.h"
#include "constants.h"
#include "macros.h"

#define N 1000

int main(int argc, char** argv) {
    int mode;
    int ret;
    BMP bmp;
    if (argc < 4) {
        fprintf(stderr, "too few arguments\n");
        return -1;
    }
    else if (strcmp(argv[1], "compress") == 0) {
        mode = COMPRESS_MODE;
    }
    else if (strcmp(argv[1], "decompress") == 0) {
        mode = DECOMPRESS_MODE;
    }
    else {
        fprintf(stderr, "pj does not support such command\n");
        return -1;
    }
    char* filename_in = argv[2];
    char* filename_out = argv[3];
    FILE* filein = fopen(filename_in, "rb");
    FILE* fileout = fopen(filename_out, "wb");
    if (filein == NULL) {
        fprintf(stderr, "Input file does not exist\n");
        return -1;
    }

    if (mode == COMPRESS_MODE) {
        ret = init_bmp(&bmp, filein);
        fclose(filein);
        ERROR_READING_FILE(ret, bmp, "Error when reading file\n");
        MEMORY_ISSUE(ret, bmp);
        printf("depth: %d\nheader_size: %d\nwidth: %d\nheight: %d\ncolor table size: %d\n",
                                bmp.bit_depth,
                                bmp.info_header_size,
                                bmp.width,
                                bmp.height,
                                bmp.color_tab_size);
        ret = is_valid_bmp(&bmp);
        ERROR_UNSUPPORTED_INPUT(ret, bmp, "Incompatible file\n");
        ERROR_READING_FILE(ret, bmp, "Invalid file fields\n");

        int width = bmp.width;
        int height = abs(bmp.height);

        /* int width = get_padding_size(bmp.width, 8);
        int height = get_padding_size(abs(bmp.height), 8); */

        unsigned char* y = malloc(width * height);
        unsigned char* cb = malloc(width * height);
        unsigned char* cr = malloc(width * height);
        ALLOC_ISSUE(y, bmp, "Error when allocating memory for component y\n");
        ALLOC_ISSUE(cb, bmp, "Error when allocating memory for component cb\n");
        ALLOC_ISSUE(cr, bmp, "Error when allocating memory for component cr\n");

        get_components(&bmp, y, cb, cr, height, width);
        downsample_component(cb, cb, height, width);
        downsample_component(cr, cr, height, width);
        unsigned char* pixel = bmp.pixel;
        for (int i = 0; i < abs(bmp.height); i++) {
            for (int j = 0; j < bmp.width; j++) {
                pixel = get_pixel_pointer(&bmp, i, j);
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = cr[RM_INDEX(width, i / 2, j / 2)];
            }
        }


        /* pixel = get_pixel_pointer(&bmp, abs(bmp.height) - 1, bmp.width - 1);
        pixel[0] = 255;
        pixel[1] = 255;
        pixel[2] = 255; */
        ret = store_bmp(&bmp, fileout);
        free(y);
        free(cr);
        free(cb);
        fclose(fileout);
        destroy_bmp(&bmp);
    }
    return 0;
}