#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bmp.h"
#include "headers/compression.h"
#include "constants.h"
#include "macros.h"
#include "pj.h"
#include "headers/memory.h"

double cosine[8][8];
double block[8][8];
double freq[8][8];
double Q[8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                    {12, 12, 14, 19, 26, 58, 60, 55},
                    {14, 13, 16, 24, 40, 57, 69, 56},
                    {14, 17, 22, 29, 51, 87, 80, 62},
                    {18, 22, 37, 56, 68, 109, 103, 77},
                    {24, 35, 55, 64, 81, 104, 113, 92},
                    {49, 64, 78, 87, 103, 121, 120, 101},
                    {72, 92, 95, 98, 112, 100, 103, 99}};

int main(int argc, char** argv) {
    int mode;
    int ret;
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

    if (mode == COMPRESS_MODE) {
        printf("%s -> %s\n", filename_in, filename_out);
        BMP bmp;
        PJ pj;
        MEM mem;
        FILE* filein = fopen(filename_in, "rb");
        FILE* fileout = fopen(filename_out, "wb");
        if (filein == NULL) {
            fprintf(stderr, "Input file does not exist\n");
            fclose(filein);
            fclose(fileout);
            return -1;
        }
        ret = init_bmp(&bmp, filein);
        if (ret == INVALID_FILE_ERROR_CODE) {
            printf("the file provided is invalid\n");
            destroy_bmp(&bmp);
            fclose(filein);
            fclose(fileout);
            return -1;
        }
        if (ret == MEMORY_ISSUE_ERROR_CODE) {
            printf("failed to allocate memory for the input file\n");
            destroy_bmp(&bmp);
            fclose(filein);
            fclose(fileout);
            return -1;
        }
        printf("depth: %d\nheader_size: %d\nwidth: %d\nheight: %d\ncolor table size: %d\n",
                                bmp.bit_depth,
                                bmp.info_header_size,
                                bmp.width,
                                bmp.height,
                                bmp.color_tab_size);
        ret = is_valid_bmp(&bmp);
        if (ret == UNSUPPORTED_INPUT_ERROR_CODE) {
            printf("the probided bmp is impossible to process\n");
            destroy_bmp(&bmp);
            fclose(filein);
            fclose(fileout);
            return -1;
        }
        if (ret == INVALID_FILE_ERROR_CODE) {
            printf("the file provided is invalid\n");
            destroy_bmp(&bmp);
            fclose(filein);
            fclose(fileout);
            return -1;
        }
        

        /* int new_width = bmp.width;
        int new_height = abs(bmp.height); */

        // width and height are computed to include the padding
        
        int new_width = get_padding_size(bmp.width, 16);
        int new_height = get_padding_size(abs(bmp.height), 16);

        printf("\nnew width %d new height: %d\n", new_width, new_height);

        unsigned char* y = malloc(new_width * new_height);
        unsigned char* cb = malloc(new_width * new_height);
        unsigned char* cr = malloc(new_width * new_height);
        append_pointer(&mem, y);
        append_pointer(&mem, cb);
        append_pointer(&mem, cr);

        if (y == NULL || cb == NULL || cr == NULL) {
            printf("failed to allocate memory for components\n");
            free_all(&mem);
            destroy_bmp(&bmp);
            fclose(filein);
            fclose(fileout);
            return -1;
        }

        // transforms the color space of the image
        get_components(&bmp, y, cb, cr, new_height, new_width);

        // all obtained components get padded before downsampling
        pad_component(y, abs(bmp.height), bmp.width, new_height, new_width);
        pad_component(cr, abs(bmp.height), bmp.width, new_height, new_width);
        pad_component(cb, abs(bmp.height), bmp.width, new_height, new_width);

        // downsampling the components
        downsample_component(cb, cb, new_height, new_width);
        downsample_component(cr, cr, new_height, new_width);

        unsigned char* pixel = bmp.pixel;
        FILE* file_debug = fopen("outputs/file_debug.bmp", "wb");
        for (int i = 0; i < abs(bmp.height); i++) {
            for (int j = 0; j < bmp.width; j++) {
                pixel = get_pixel_pointer(&bmp, i, j);
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = cr[RM_INDEX(new_width, i, j)];
            }
        }
        ret = store_bmp(&bmp, file_debug);
        fclose(file_debug);

        free_pointer(&mem, 0);
        free_pointer(&mem, 1);
        free_pointer(&mem, 2);

        char* result_y = malloc(new_width * new_height);
        char* result_cb = malloc(new_width * new_height);
        char* result_cr = malloc(new_width * new_height);
        append_pointer(&mem, result_y);
        append_pointer(&mem, result_cr);
        append_pointer(&mem, result_cb);
        if (result_y == NULL || result_cb == NULL || result_cr == NULL) {
            printf("failed to allocate memory for the encoding results\n");
            free_all(&mem);
            destroy_bmp(&bmp);
            fclose(filein);
            fclose(fileout);
            return -1;
        }

        int encoded_y_size = get_encoded_component(y, new_height, new_width, result_y, 1);
        int encoded_cb_size = get_encoded_component(cb, new_height, new_width, result_cb, 2);
        int encoded_cr_size = get_encoded_component(cr, new_height, new_width, result_cr, 2);

        printf("original size: %d\n", 3 * bmp.width * abs(bmp.height));
        printf("encoded component sizes: y: %d cb: %d cr: %d, total: %d\n",
            encoded_y_size, encoded_cb_size, encoded_cr_size,
            encoded_y_size + encoded_cb_size + encoded_cr_size);

        create_pj(&pj, encoded_y_size, encoded_cb_size, encoded_cr_size,
                result_y, result_cb, result_cr, bmp.height, bmp.width,
                new_height, new_width);
        ret = write_pj(&pj, fileout);

        free_pointer(&mem, 3);
        free_pointer(&mem, 4);
        free_pointer(&mem, 5);

        /* for (int i = 0; i < 100; i++) {
            printf("%d\n", result_y[i]);
        } */

        fclose(filein);
        fclose(fileout);
        destroy_bmp(&bmp);
    }
    return 0;
}