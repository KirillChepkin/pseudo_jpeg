#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bmp.h"
#include "headers/compression.h"
#include "constants.h"
#include "macros.h"
#include "pj.h"
#include "headers/memory.h"
#include "decompression.h"

double cosine[8][8];
double block[8][8];
double freq[8][8];

int main(int argc, char** argv) {
    int mode;
    int ret;
    if (argc < 2) {
        fprintf(stderr, "too few arguments\n");
        printf(HELP_MESSAGE);
        return -1;
    } 
    else if (strcmp(argv[1], "--help") == 0) {
        printf(HELP_MESSAGE); 
        return 0;
    }
    else if (argc < 4) {
        fprintf(stderr, "too few arguments\n");
        printf(HELP_MESSAGE);
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
        printf(HELP_MESSAGE);
        return -1;
    }
    char* filename_in = argv[2];
    char* filename_out = argv[3];
    printf("%s -> %s\n", filename_in, filename_out);
    if (mode == COMPRESS_MODE) {
        BMP bmp;
        PJ pj;
        MEM mem;
        FILE* filein = fopen(filename_in, "rb");
        FILE* fileout = fopen(filename_out, "wb");
        mem_init(&mem);
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
                pixel[0] = y[RM_INDEX(new_width, i, j)];
                pixel[1] = y[RM_INDEX(new_width, i, j)];
                pixel[2] = y[RM_INDEX(new_width, i, j)];
            }
        }
        ret = store_bmp(&bmp, file_debug);
        fclose(file_debug);

        char* result_y = malloc(new_width * new_height);
        char* result_cb = malloc(new_width * new_height);
        char* result_cr = malloc(new_width * new_height);
        append_pointer(&mem, result_y);
        append_pointer(&mem, result_cr);
        append_pointer(&mem, result_cb);
        if (result_y == NULL || result_cb == NULL || result_cr == NULL) {
            printf("failed to allocate memory for encoded results\n");
            free_all(&mem);
            destroy_bmp(&bmp);
            fclose(filein);
            fclose(fileout);
            return -1;
        }

        /* precompute_cosines(cosine);
        normalize_block(y, block, new_height, new_width, 0, 0);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                printf("%7.2f ", block[i][j] + 128);
            }
            printf("\n");
        }
        printf("-------\n");
        compute_block_DCT(block, cosine, freq);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                printf("%7.2f ", freq[i][j]);
            }
            printf("\n");
        } */


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

        if (ret == INVALID_FILE_ERROR_CODE) {
            printf("failed to output the compressed\n");
            free_all(&mem);
            fclose(filein);
            fclose(fileout);
            destroy_bmp(&bmp);
            return -1;
        }
        free_all(&mem);

        fclose(filein);
        fclose(fileout);
        destroy_bmp(&bmp);
    }
    else if (mode == DECOMPRESS_MODE) {
        PJ pj;
        MEM mem;
        BMP bmp;
        FILE* filein = fopen(filename_in, "rb");
        FILE* fileout = fopen(filename_out, "wb");
        mem_init(&mem);
        if (filein == NULL) {
            printf("Input file not found\n");
            printf(HELP_MESSAGE);
            fclose(filein);
            fclose(fileout);
            return -1;
        }

        int ret = read_pj(&pj, filein);
        if (ret == MEMORY_ISSUE_ERROR_CODE) {
            printf("Failed to allocate memory for the read file\n");
            fclose(filein);
            fclose(fileout);
            return -1;
        }
        else if (ret == INVALID_FILE_ERROR_CODE) {
            printf("The input file is invalid\n");
            fclose(filein);
            fclose(fileout);
        }
        printf("file size: %d\npadded height: %d\npadded width: %d\noriginal height: %d\noriginal width: %d\n",
                                                            pj.file_size,
                                                            pj.new_height,
                                                            pj.new_width,
                                                            pj.height,
                                                            pj.width);
        // allocating memory for decoded components
        unsigned char* y = malloc(pj.new_height * pj.new_width);
        unsigned char* cb = malloc((pj.new_height * pj.new_width) / 4);
        unsigned char* cr = malloc((pj.new_height * pj.new_width) / 4);
        append_pointer(&mem, y);
        append_pointer(&mem, cb);
        append_pointer(&mem, cr);
        if (y == NULL || cb == NULL || cr == NULL) {
            printf("Failed to allocate memory for decoded components\n");
            free_all(&mem);
            fclose(filein);
            fclose(fileout);
            destroy_pj(&pj);
            destroy_bmp(&bmp);
            return -1;
        }
        // cb and cr component matrices have x0.5 height and width of the y component matrix
        recover_component(pj.y_component, pj.y_size, y, pj.new_height, pj.new_width);
        recover_component(pj.cb_component, pj.y_size, cb, pj.new_height / 2, pj.new_width / 2);
        recover_component(pj.cr_component, pj.y_size, cr, pj.new_height / 2, pj.new_width / 2);
        destroy_pj(&pj);

        ret = bmp_from_decompressed(&bmp, pj.new_height, pj.new_width, pj.height, pj.width, y, cb, cr);
        if (ret == MEMORY_ISSUE_ERROR_CODE) {
            printf("Failed to allocate memory for decoded the bmp file object\n");
            free_all(&mem);
            fclose(filein);
            fclose(fileout);
            destroy_pj(&pj);
            destroy_bmp(&bmp);
            return -1;
        }
        ret = store_bmp(&bmp, fileout);
        if (ret == INVALID_FILE_ERROR_CODE) {
            printf("The program was unable to write the file\n");
            free_all(&mem);
            fclose(filein);
            fclose(fileout);
            destroy_pj(&pj);
            destroy_bmp(&bmp);
            return -1;
        }
        destroy_bmp(&bmp);

        /* int c = 0;
        for (int i = 0; i < pj.new_height - 1; i += 8) {
            for (int j = 0; j < pj.new_width - 1; j += 8) {
                // 2000, 4200
                if (c == 4319) {
                    for (int u = 0; u < 8; u++) {
                        for (int v = 0; v < 8; v++) {
                            printf("%7u", y[RM_INDEX(pj.new_width, i + u, j + v)]);
                        }
                        printf("\n");
                    }
                }
                c++;
            }
        }
        printf("%d", c); */

        free_all(&mem);
        fclose(filein);
        fclose(fileout);
    }
    return 0;
}