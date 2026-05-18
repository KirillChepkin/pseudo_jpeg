#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdio.h>

typedef struct bmp_image_s {
    unsigned char file_header[14];
    uint32_t info_header_size;
    unsigned char* info_header;
    int color_tab_size;
    unsigned char *color_tab;
    int32_t width;
    int32_t height;
    uint16_t bit_depth;
    uint32_t offset;
    uint32_t file_size;
    uint32_t pixel_data_size;
    unsigned char* pixel;
} BMP;

int init_bmp(BMP* bmp, FILE* file);
int store_bmp(BMP* bmp, FILE* file);
void destroy_bmp(BMP* bmp);
int get_row_size(BMP* bmp);
int is_valid_bmp(BMP* bmp);
unsigned char* get_pixel_pointer(BMP* bmp, int row, int column);
void get_components(BMP* bmp, unsigned char* y, unsigned char* cb, unsigned char* cr);
void cut_resolution(BMP* bmp);

#endif