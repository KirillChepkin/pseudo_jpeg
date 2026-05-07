#ifndef INPUT_H
#define INPUT_H

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

#endif