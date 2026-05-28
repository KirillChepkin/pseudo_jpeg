#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bmp.h"
#include "constants.h"
#include "macros.h"

int init_bmp(BMP* bmp, FILE* file) {
    // reads the .bmp file into a bmp object and allocates memory for file contents
    int ret;
    bmp -> info_header = NULL;
    bmp -> color_tab = NULL;
    bmp -> pixel = NULL;
    // reading file header
    ret = fread(bmp -> file_header, 1, 14, file);
    if (ret != 14) {
        return INVALID_FILE_ERROR_CODE;
    }

    // reading information header
    ret = fread(&(bmp -> info_header_size), sizeof(int), 1, file);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }
    bmp -> info_header = malloc(bmp -> info_header_size);
    if (bmp -> info_header == NULL) {
        return MEMORY_ISSUE_ERROR_CODE;
    }
    *((int*)bmp -> info_header) = bmp -> info_header_size;
    ret = fread(bmp -> info_header + sizeof(int), 1, bmp -> info_header_size - sizeof(int), file);
    if (ret != bmp -> info_header_size - sizeof(int)) {
        return INVALID_FILE_ERROR_CODE;
    }

    // storing some of the file information header fields
    bmp -> width = ((int*)bmp -> info_header)[1];
    bmp -> height = ((int*)bmp -> info_header)[2];
    bmp -> bit_depth = *((uint16_t*)(bmp -> info_header + 14));
    bmp -> offset = *((int*)(bmp -> file_header + 10));

    // reading color table
    bmp -> color_tab_size = bmp -> offset - (bmp -> info_header_size + 14);
    bmp -> color_tab = NULL;
    if (bmp -> color_tab_size > 0) {
        bmp -> color_tab = malloc(bmp -> color_tab_size);
        if (bmp -> color_tab == NULL) {
            return MEMORY_ISSUE_ERROR_CODE;
        }
        ret = fread(bmp -> color_tab, 1, bmp -> color_tab_size, file);
        if (ret != bmp -> color_tab_size) {
            return INVALID_FILE_ERROR_CODE;
        }
    }
    else if (bmp -> color_tab_size < 0) {
        return INVALID_FILE_ERROR_CODE;
    }

    // reading pixel data
    bmp -> file_size = *((int*)(bmp -> file_header + 2));
    bmp -> pixel_data_size = bmp -> file_size - bmp -> offset;
    bmp -> pixel = malloc(bmp -> pixel_data_size);
    if (bmp -> pixel == NULL) {
        return MEMORY_ISSUE_ERROR_CODE;
    }
    ret = fread(bmp -> pixel, 1, bmp -> pixel_data_size, file);
    if (ret != bmp -> pixel_data_size) {
        return INVALID_FILE_ERROR_CODE;
    }
    return 0;
}

int store_bmp(BMP* bmp, FILE* file) {
    // writes a bmp object into memory in .bmp file format
    int ret;
    ret = fwrite(bmp -> file_header, 1, 14, file);
    if (ret != 14) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(bmp -> info_header, 1, bmp -> info_header_size, file);
    if (ret != bmp -> info_header_size) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(bmp -> color_tab, 1, bmp -> color_tab_size, file);
    if (ret != bmp -> color_tab_size) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(bmp -> pixel, 1, bmp -> pixel_data_size, file);
    if (ret != bmp -> pixel_data_size) {
        return INVALID_FILE_ERROR_CODE;
    }
    return 0;
}

void destroy_bmp(BMP* bmp) {
    // frees memory allocated for the bmp object
    free(bmp -> info_header);
    free(bmp -> color_tab);
    free(bmp -> pixel);
}

int get_row_size(BMP* bmp) {
    // computes the row size of the matrix which stores original pixel data considering padding
    return ((bmp -> width * bmp -> bit_depth + 31) / 32) * 4;
}

int is_valid_bmp(BMP* bmp) {
    // checks if a program will be able to compress the already read file
    if ((bmp -> bit_depth != 24 && bmp -> bit_depth != 32) || bmp -> color_tab_size != 0 ||
        bmp -> width < 16 || bmp -> width < 8) {
        return UNSUPPORTED_INPUT_ERROR_CODE;
    }
    if (bmp -> file_header[0] != 'B' || bmp -> file_header[1] != 'M' ||
        (*((int*)(bmp -> file_header + 6))) != 0 ||
        bmp -> pixel_data_size <= 0) {
        return INVALID_FILE_ERROR_CODE;
    }
    return 0;
}

unsigned char* get_pixel_pointer(BMP* bmp, int row, int column) {
    // calculates the pixel index from matrix indices
    if (bmp -> width <= column || abs(bmp -> height) <= row) {
        return NULL;
    }
    if (bmp -> height < 0) {
        return bmp -> pixel + row * get_row_size(bmp) + (bmp -> bit_depth / 8) * column;
    }
    else {
        return bmp -> pixel + (bmp -> height - row - 1) * get_row_size(bmp) + (bmp -> bit_depth / 8) * column;
    }
}

void get_components(BMP* bmp, unsigned char* y, unsigned char* cb, unsigned char* cr, int height, int width) {
    // extracts the luminance (Y) component from the image
    // bmp is stored in the BGR - blue, gree and red
    // the resulting image has (height % 8 and width % 8) >= (bmp -> height and bmp -> width)
    unsigned char* pixel;
    unsigned char lum;
    unsigned char chroma_blue;
    unsigned char chroma_red;
    for (int i = 0; i < abs(bmp -> height); i++) {
        for (int j = 0; j < bmp -> width; j++) {
            pixel = get_pixel_pointer(bmp, i, j);
            lum = 16 + (25 * pixel[0] + 129 * pixel[1] + 66 * pixel[3]) / 256;
            chroma_blue = 128 + (112 * pixel[0] - 74 * pixel[1] - 38 * pixel[3]) / 256;
            chroma_red = 128 + ((-18) * pixel[0] - 94 * pixel[1] + 112 * pixel[3]) / 256;
            y[RM_INDEX(width, i, j)] = lum;
            cb[RM_INDEX(width, i, j)] = chroma_blue;
            cr[RM_INDEX(width, i, j)] = chroma_red;
            /* pixel[0] = chroma_blue;
            pixel[1] = 0;
            pixel[2] = 0; */
        }
    }
    /* for (int i = 0; i < ) */
}

int bmp_from_decompressed(BMP* bmp, int padded_height, int padded_width, int height, int width,
                           unsigned char* y, unsigned char* cb, unsigned char* cr) {
    // argument matrices have padded resolution
    // cb and cr matrices have passed downsampling and their dimensions are half those of y
    // this method will create a bmp without the color table
    // info header of the file will be 40 pixels (least standard info header size)
    // depth of the pixel will be set to be 24 bits
    // height will be negative (starting from the top)
    bmp -> info_header = NULL;
    bmp -> pixel = NULL;
    bmp -> color_tab = NULL;
    bmp -> file_header[0] = 'B';
    bmp -> file_header[1] = 'M';
    *((int*)(bmp -> file_header + 6)) = 0;
    *((int*)(bmp -> file_header + 10)) = 54;
    bmp -> offset = 54;
    bmp -> info_header = malloc(40);
    if (bmp -> info_header == NULL) {
        return MEMORY_ISSUE_ERROR_CODE;
    }

    bmp -> color_tab_size = 0;
    bmp -> color_tab = NULL;

    *((int*)(bmp -> info_header + 0)) = 40;
    bmp -> info_header_size = 40;
    *((int*)(bmp -> info_header + 4)) = width;
    bmp -> width = width;
    *((int*)(bmp -> info_header + 8)) = -1 * height;
    bmp -> height = -1 * height;
    *((int*)(bmp -> info_header + 12)) = 1;
    *((int*)(bmp -> info_header + 14)) = 24;
    bmp -> bit_depth = 24;
    *((int*)(bmp -> info_header + 16)) = 0;

    int row = get_row_size(bmp);
    bmp -> pixel_data_size = 3 * height * row;
    bmp -> pixel = malloc(bmp -> pixel_data_size);
    *((int*)(bmp -> info_header + 20)) = bmp -> pixel_data_size;

    if (bmp -> pixel == NULL) {
        return MEMORY_ISSUE_ERROR_CODE;
    }

    // finally writing the file size
    *((int*)(bmp -> file_header + 2)) = bmp -> pixel_data_size + bmp -> offset;

    /* printf("%d %d %d\n", *((int*)(bmp -> file_header + 2)), bmp -> info_header[14], bmp -> pixel_data_size); */

    unsigned char* pointer;
    for (int i = 0; i < abs(height); i++) {
        for (int j = 0; j < width; j++) {
            pointer = get_pixel_pointer(bmp, i, j);

            /* pointer[0] = 255;
            pointer[1] = 0;
            pointer[2] = 0; */

            pointer[0] = y[RM_INDEX(padded_width, i, j)];
            pointer[1] = y[RM_INDEX(padded_width, i, j)];
            pointer[2] = y[RM_INDEX(padded_width, i, j)];

            /* if (pointer == NULL) {
                printf("NULL %d %d %d %d %d\n", i, j, bmp -> height, bmp -> width, bmp -> pixel);
                return;
            } */
        }
    }
    return 0;
}