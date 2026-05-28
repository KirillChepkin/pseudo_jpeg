#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "bmp.h"
#include "macros.h"
#include "compression.h"
#include "constants.h"

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

    if (row + 8 > matrix_height || column + 8 > matrix_width || row < 0 || column < 0) {
        assert(1 == 2);
    }
    for (int i = row; i < row + 8; i++) {
        for (int j = column; j < column + 8; j++) {
            block[i - row][j - column] = ((double)matrix[RM_INDEX(matrix_width, i, j)] - 128.0);
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
    // writing it into freq
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
            /* if (freq[i][j] > 0) {
                freq[i][j] = ceil(freq[i][j]);
            }
            else {
                freq[i][j] = floor(freq[i][j]);
            } */
        }
    }
}

int zig_zag_RLE(double freq[8][8], char* result) {
    // converts the frequency to char and stores coefficients ignoring zeros at the end
    // returns the number of coefficients stored in result
    static char rle[64] = {};
    int c = 0;
    int i;
    char cur;
    int zeroes = 0;
    for (int d = 0; d < 15; d++) {
        if (d >= 8) {
            i = 7;
        }
        else {
            i = d;
        }
        if (d % 2 == 0) {
            while (i >= 0 && (d - i) < 8) {
                cur = (char)freq[i][d - i];
                rle[c] = cur;
                c++;
                i--;
            }
        }
        else {
            while (i >= 0 && (d - i) < 8) {
                cur = (char)freq[d - i][i];
                rle[c] = cur;
                c++;
                i--;
            }
        }
    }
    for (int i = 63; i >= 0; i--) {
        if (rle[i] == 0) {
            zeroes++;
        }
        else {
            break;
        }
    }
    zeroes = 64 - zeroes;
    c = 0;
    /* printf("%d\n", zeroes); */
    // 127 will serve as the end of block value
    // 127 is very unlikely to appear among coefficients
    while (zeroes != c) {
        result[c] = rle[c];
        if (result[c] == 127) {
            result[c]--;
        }
        c++;
    }
    return c;
}

int get_encoded_component(unsigned char* c, int height, int width, char* result, int d) {
    // heighth and width are provided for the component before downsampling.
    // d is the factor by which the resolution was divided during downsampling
    // height and width are supposed to be divisible by 16
    static double block[8][8];
    static double freq[8][8];
    static double cosine[8][8];
    static double Q[8][8] = Q_MATRIX;
    int num = 0;
    precompute_cosines(cosine);
    for (int i = 0; i <= (height / d) - 1; i += 8) {
        for (int j = 0; j <= (width / d) - 1; j += 8) {
            normalize_block(c, block, height, width, i, j);
            compute_block_DCT(block, cosine, freq);
            quantize_block(freq, Q);
            round_block(freq);
            num += zig_zag_RLE(freq, result + num);
            result[num] = 127;
            num++;
            // 127 marks the end of the block
        }
    }
    return num;
}