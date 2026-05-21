#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "bmp.h"
#include "macros.h"

int get_padding_size(int x, int d) {
    // computes a number which is >= x and % d

    if (x % d == 0) {
        return x;
    }
    return x + (d - x % d);
}

void pad_component(unsigned char* c, int height, int width, int new_height, int new_width) {
    // this function fills the image symetrically so that it has height % 8 and width % 8
    // height and width are from the original image
    // the c matrix has new_height and new_width
    // height and width are supposed to be positive

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < (new_height - height); j++) {
            c[RM_INDEX(new_width, height + j, i)] = c[RM_INDEX(new_width, height - j - 1, i)];
        }
    }
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < (new_width - width); j++) {
            c[RM_INDEX(new_width, i, width + j)] = c[RM_INDEX(new_width, i, width - j - 1)];
        }
    }
}

void downsample_component(unsigned char* c, unsigned char* dest, int height, int width) {
    // decreases the resolution of the specific image component by the factor of 2
    // expects to get the padded component, otherwise odd pixels will be ignored
    // is able to write the result both to the existing component row-major matrix
    // and to a separate piece of memory

    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width / 2; j++) {
            dest[RM_INDEX(width, i, j)] = (c[RM_INDEX(width, i * 2, j * 2)] +
                                           c[RM_INDEX(width, i * 2, j * 2 + 1)] +
                                           c[RM_INDEX(width, i * 2 + 1, j * 2)] +
                                           c[RM_INDEX(width, i * 2 + 1, j * 2 + 1)]) / 4;
        }
    }
}

void normalize_block(unsigned char* matrix, double block[8][8], int matrix_height, int matrix_width,
    int row,
    int column) {
    // normalizes matrix's 8*8 block values to be around zero,
    // converts them to double and writes into the double block[8][8].
    // block size is hardcoded to be 8*8.
    // each 8*8 block of the original image will be processed separately
    // to decrease the memory consumption because sizeof(double) == 8 on most systems
    // row and column are indices of the block's top-left corner

    /* printf("%d %d %d %d\n", row, column, matrix_height, matrix_width); */
    if (row + 8 > matrix_height || column + 8 > matrix_width || row < 0 || column < 0) {
        assert(1 == 2);
    }
    for (int i = row; i < row + 8; i++) {
        for (int j = column; j < column + 8; j++) {
            block[i - row][j - column] = ((double)matrix[RM_INDEX(matrix_width, i, j)] - 128.0);
            /* printf("%d ", matrix[RM_INDEX(matrix_width, i, j)]); */
        }
    }
}

void precompute_cosines(double cosine[8][8]) {
    // get_coefficient function calls the cosine function cos((2 * x + 1) * u * M_PI)
    // this function precomputes values for x and y both in N[0;8]
    for (int x = 0; x < 8; x++) {
        for (int u = 0; u < 8; u++) {
            cosine[x][u] = cos(((2 * x + 1) * u * M_PI) / 16);
        }
    }
}

double get_coefficient(double block[8][8], double cosine[8][8], int u, int v) {
    // computes the coefficient for a given (u, v) frequency basis.
    // uses precomputed cosind values for each pixel
    double sum = 0;
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            sum += block[x][y] * cosine[x][u] * cosine[y][v];
        }
    }
    sum *= 0.25;
    if (u == 0) {
        sum /= sqrt(2);
    }
    if (v == 0) {
        sum /= sqrt(2);
    }
    return sum;
}

void compute_block_DCT(double block[8][8], double cosine[8][8], double freq[8][8]) {
    // iterates through all the possible (u, v) frequency pairs and computes the frequency matrix
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            freq[u][v] = get_coefficient(block, cosine, u, v);
        }
    }
}

void quantize_block(double freq[8][8], double Q[8][8]) {
    // divides each frequency in the frequency matrix by a constant
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            freq[i][j] /= Q[i][j];
        }
    }
}

void round_block(double freq[8][8]) {
    // rounds the block values
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            freq[i][j] = round(freq[i][j]);
        }
    }
}

/* double block[8][8] = {{-76, -73, -67, -62, -58, -67, -64, -55},
                      {-65, -69, -73, -38, -19, -43, -59, -56},
                      {-66, -69, -60, -15, 16, -24, -62, -55},
                      {-65, -70, -57, -6, 26, -22, -58, -59},
                      {-61, -67, -60, -24, -2, -40, -60, -58},
                      {-49, -63, -68, -58, -51, -60, -70, -53},
                      {-43, -57, -64, -69, -73, -67, -63, -45},
                      {-41, -49, -59, -60, -63, -52, -50, -34}};
double Q[8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                  {12, 12, 14, 19, 26, 58, 60, 55},
                  {14, 13, 16, 24, 40, 57, 69, 56},
                  {14, 17, 22, 29, 51, 87, 80, 62},
                  {18, 22, 37, 56, 68, 109, 103, 77},
                  {24, 35, 55, 64, 81, 104, 113, 92},
                  {49, 64, 78, 87, 103, 121, 120, 101},
                  {72, 92, 95, 98, 112, 100, 103, 99}}; */
double freq[8][8];