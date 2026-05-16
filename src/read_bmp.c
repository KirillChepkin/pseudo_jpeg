#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bmp.h"
#include "constants.h"

#define N 1000000

int init_bmp(BMP* bmp, FILE* file) {
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
    if (bmp -> info_header != NULL) {
        free(bmp -> info_header);
    }
    if (bmp -> color_tab != NULL) {
        free(bmp -> color_tab);
    }
    if (bmp -> pixel != NULL) {
        free(bmp -> pixel);
    }
}

int get_row_size(BMP* bmp) {
    return ((bmp -> width * bmp -> bit_depth + 31) / 32) * 4;
}

int is_valid_bmp(BMP* bmp) {
    if ((bmp -> bit_depth != 24 && bmp -> bit_depth != 32) || bmp -> color_tab_size != 0) {
        return UNSUPPORTED_INPUT_ERROR_CODE;
    }
    if (bmp -> file_header[0] != 'B' || bmp -> file_header[1] != 'M' ||
        (*((int*)(bmp -> file_header + 6))) != 0 ||
        bmp -> pixel_data_size <= 0) {
        return INVALID_FILE_ERROR_CODE;
    }
    return 0;
}