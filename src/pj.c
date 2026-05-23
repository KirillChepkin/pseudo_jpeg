#include <stdio.h>
#include "constants.h"
#include "pj.h"

void create_pj(PJ* pj, int size_y, int size_cb, int size_cr, char* y, char* cb, char* cr, int original_height,
                                                                                  int original_width,
                                                                                  int new_height,
                                                                                  int new_width) {
    // the created object contains pointers allocated outside of it's methods and deleted there
    // component sizes here are different from the sizes allocated for components, they mean the 
    // number of meaningful elements
    pj -> file_size = 8 * 4 + size_y + size_cr + size_cb;
    pj -> y_component = y;
    pj -> cb_component = cb;
    pj -> cr_component = cr;
    pj -> height = original_height;
    pj -> width = original_width;
    pj -> new_height = new_height;
    pj -> new_width = new_width;
    pj -> y_size = size_y;
    pj -> cb_size = size_cb;
    pj -> cr_size = size_cr;
    /* printf("%d\n", pj -> file_size); */
}

int write_pj(PJ* pj, FILE* fileout) {
    int ret;
    ret = fwrite(&(pj -> file_size), sizeof(int), 1, fileout);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }

    ret = fwrite(&(pj -> y_size), sizeof(int), 1, fileout);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(&(pj -> cb_size), sizeof(int), 1, fileout);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(&(pj -> cr_size), sizeof(int), 1, fileout);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }

    ret = fwrite(&(pj -> height), sizeof(int), 1, fileout);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(&(pj -> width), sizeof(int), 1, fileout);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(&(pj -> new_height), sizeof(int), 1, fileout);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(&(pj -> new_width), sizeof(int), 1, fileout);
    if (ret != 1) {
        return INVALID_FILE_ERROR_CODE;
    }

    ret = fwrite(pj -> y_component, 1, pj -> y_size, fileout);
    if (ret != pj -> y_size) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(pj -> cb_component, 1, pj -> cb_size, fileout);
    if (ret != pj -> cb_size) {
        return INVALID_FILE_ERROR_CODE;
    }
    ret = fwrite(pj -> cr_component, 1, pj -> cr_size, fileout);
    if (ret != pj -> cr_size) {
        return INVALID_FILE_ERROR_CODE;
    }
    return 0;
}