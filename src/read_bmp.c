#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "bmp.h"

#define N 1000000

int init_bmp(BMP* bmp, FILE* file) {
    int ret;
    bmp -> info_header = NULL;
    bmp -> color_tab = NULL;
    bmp -> pixel = NULL;
    // reading file header
    ret = fread(bmp -> file_header, 1, 14, file);
    if (ret != 14) {
        return -1;
    }

    // reading information header
    ret = fread(&(bmp -> info_header_size), sizeof(int), 1, file);
    if (ret != 1) {
        return -1;
    }
    bmp -> info_header = malloc(bmp -> info_header_size);
    if (bmp -> info_header == NULL) {
        return -2;
    }
    *((int*)bmp -> info_header) = bmp -> info_header_size;
    ret = fread(bmp -> info_header + sizeof(int), 1, bmp -> info_header_size - sizeof(int), file);
    if (ret != bmp -> info_header_size - sizeof(int)) {
        return -1;
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
            return -2;
        }
        ret = fread(bmp -> color_tab, 1, bmp -> color_tab_size, file);
        if (ret != bmp -> color_tab_size) {
            return -1;
        }
    }
    else if (bmp -> color_tab_size < 0) {
        return -1;
    }

    // reading pixel data
    bmp -> file_size = *((int*)(bmp -> file_header + 2));
    bmp -> pixel_data_size = bmp -> file_size - bmp -> offset;
    bmp -> pixel = malloc(bmp -> pixel_data_size);
    if (bmp -> pixel == NULL) {
        return -2;
    }
    ret = fread(bmp -> pixel, 1, bmp -> pixel_data_size, file);
    if (ret != bmp -> pixel_data_size) {
        return -1;
    }
    return 0;
}

int store_bmp(BMP* bmp, FILE* file) {
    int ret;
    ret = fwrite(bmp -> file_header, 1, 14, file);
    if (ret != 14) {
        return -1;
    }
    ret = fwrite(bmp -> info_header, 1, bmp -> info_header_size, file);
    if (ret != bmp -> info_header_size) {
        return -1;
    }
    ret = fwrite(bmp -> color_tab, 1, bmp -> color_tab_size, file);
    if (ret != bmp -> color_tab_size) {
        return -1;
    }
    ret = fwrite(bmp -> pixel, 1, bmp -> pixel_data_size, file);
    if (ret != bmp -> pixel_data_size) {
        return -1;
    }
    return 0l;
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

/* char file[N];
BMP bmp;

int main() {
    FILE* filein = fopen("sample.bmp", "rb");
    init_bmp(&bmp, filein);
    fclose(filein);

    printf("%d %d %d %d %d %d\n", bmp.bit_depth,
                               bmp.info_header_size,
                               bmp.width,
                               bmp.height,
                               bmp.offset,
                               bmp.color_tab_size);
    
    for (int i = 0; i < 300; i += 3) {
        bmp.pixel[i] = 255;
        bmp.pixel[i + 1] = 255;
        bmp.pixel[i + 2] = 255;
    }

    FILE* fileout = fopen("output.bmp", "wb");
    store_bmp(&bmp, fileout);
    fclose(fileout);
    destroy_bmp(&bmp);
    return 0;
} */