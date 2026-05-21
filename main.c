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

        /* int new_width = bmp.width;
        int new_height = abs(bmp.height); */

        // width and height  already include the padding
        
        int new_width = get_padding_size(bmp.width, 16);
        int new_height = get_padding_size(abs(bmp.height), 16);

        printf("\nnew width %d new height: %d\n", new_width, new_height);

        unsigned char* y = malloc(new_width * new_height);
        unsigned char* cb = malloc(new_width * new_height);
        unsigned char* cr = malloc(new_width * new_height);
        ALLOC_ISSUE(y, bmp, "Error when allocating memory for component y\n");
        ALLOC_ISSUE(cb, bmp, "Error when allocating memory for component cb\n");
        ALLOC_ISSUE(cr, bmp, "Error when allocating memory for component cr\n");

        get_components(&bmp, y, cb, cr, new_height, new_width);

        // all obtained components get padded before downsampling

        pad_component(y, abs(bmp.height), bmp.width, new_height, new_width);
        pad_component(cr, abs(bmp.height), bmp.width, new_height, new_width);
        pad_component(cb, abs(bmp.height), bmp.width, new_height, new_width);

        downsample_component(cb, cb, new_height, new_width);
        downsample_component(cr, cr, new_height, new_width);

        double block[8][8];
        normalize_block(y, block, new_height, new_width, new_height - 8, new_width - 8);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                printf("%3.2f ", block[i][j]);
            }
            printf("\n");
        }

        unsigned char* pixel = bmp.pixel;
        for (int i = 0; i < abs(bmp.height); i++) {
            for (int j = 0; j < bmp.width; j++) {
                pixel = get_pixel_pointer(&bmp, i, j);
                pixel[0] = y[RM_INDEX(new_width, i, j)];
                pixel[1] = y[RM_INDEX(new_width, i, j)];
                pixel[2] = y[RM_INDEX(new_width, i, j)];
            }
        }

        /* for (int i = abs(bmp.height) - 8; i < new_height; i++) {
            printf("%d: %d\n", i, cb[RM_INDEX(new_width, i, 0)]);
        }
        printf("------\n");
        for (int i = bmp.width - 8; i < new_width; i++) {
            printf("%d: %d\n", i, cb[RM_INDEX(new_width, 0, i)]);
        }
        printf("------\n");
        for (int i = bmp.width - 8; i < new_width; i++) {
            printf("%d: %d\n", i, cb[RM_INDEX(new_width, new_height, i)]);
        } */



        /* pixel = get_pixel_pointer(&bmp, abs(bmp.height) / 2 - 1, bmp.width / 2 - 1);
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