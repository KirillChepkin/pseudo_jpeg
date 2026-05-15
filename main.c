#include <stdio.h>
#include <string.h>
#include <bmp.h>

#define N 1000

int main(int argc, char** argv) {
    char* filename_in = argv[1];
    char* filename_out = argv[2];
    int ret;
    FILE* filein = fopen(filename_in, "rb");
    FILE* fileout = fopen(filename_out, "wb");
    if (filein == NULL) {
        fprintf(stderr, "The file does not exist\n");
        return -1;
    }
    BMP bmp;
    ret = init_bmp(&bmp, filein);
    if (ret == -1) {
        fprintf(stderr, "Error reading file\n");
        destroy_bmp(&bmp);
        return -1;
    }
    if (ret == -2) {
        fprintf(stderr, "Memory issue\n");
        destroy_bmp(&bmp);
        return -1;
    }
    

    /* printf("%d %d %d %d %d %d\n", bmp.bit_depth,
                               bmp.info_header_size,
                               bmp.width,
                               bmp.height,
                               bmp.offset,
                               bmp.color_tab_size); */
    
    int row_size = get_row_size(&bmp);
    for (int i = row_size * 200; i < row_size * 201; i += 3) {
        bmp.pixel[i] = 255;
        bmp.pixel[i + 1] = 255;
        bmp.pixel[i + 2] = 255;
    }
    ret = store_bmp(&bmp, fileout);
    destroy_bmp(&bmp);
    if (ret == -1) {
        fprintf(stderr, "Error when writing the file\n");
        return -1;
    }
    return 0;
}