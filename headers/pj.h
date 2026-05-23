#ifndef PJ_H
#define PJ_H

#include <stdint.h>

typedef struct pseudo_jpeg_file_object_s {
    uint32_t file_size;
    // sizes of compressed components
    uint32_t y_size;
    uint32_t cb_size;
    uint32_t cr_size;
    // original image resolution
    uint32_t height;
    uint32_t width;
    // padded image resolution
    uint32_t new_height;
    uint32_t new_width;
    // pointers to compressed image components
    char* y_component;
    char* cb_component;
    char* cr_component;
} PJ;

void create_pj(PJ* pj, int size_y, int size_cb, int size_cr, char* y, char* cb, char* cr, int original_height,
                                                                                  int original_width,
                                                                                  int new_height,
                                                                                  int new_width);
int write_pj(PJ* pj, FILE* fileout);

#endif