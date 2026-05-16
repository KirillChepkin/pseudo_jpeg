#include <stdio.h>
#include <string.h>

#include "bmp.h"
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
    /* int row_size = get_row_size(&bmp);
    for (int i = row_size * 10; i < row_size * 10 + 3 * bmp.width; i += 3) {
        bmp.pixel[i] = 255;
        bmp.pixel[i + 1] = 255;
        bmp.pixel[i + 2] = 255;
    } */

    if (mode == COMPRESS_MODE) {
        ret = init_bmp(&bmp, filein);
        fclose(filein);
        ERROR_READING_FILE(ret, bmp, "Error when reading file\n");
        MEMORY_ISSUE(ret, bmp);
        /* printf("%d %d %d %d %d %d %d\n", bmp.bit_depth,
                                bmp.info_header_size,
                                bmp.width,
                                bmp.height,
                                bmp.offset,
                                bmp.color_tab_size,
                                (*((int*)(bmp.file_header + 6))));
        printf("%c %c %d\n", bmp.file_header[0], bmp.file_header[1], (*((int*)(bmp.file_header + 6)))); */
        ret = is_valid_bmp(&bmp);
        /* printf("%d\n", ret); */
        ERROR_UNSUPPORTED_INPUT(ret, bmp, "Incompatible file\n");
        ERROR_READING_FILE(ret, bmp, "Invalid file fields\n");
        ret = store_bmp(&bmp, fileout);
        fclose(fileout);
        destroy_bmp(&bmp);
    }
    return 0;
}