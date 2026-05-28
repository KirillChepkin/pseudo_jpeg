#include <assert.h>
#include <math.h>
#include "compression.h"
#include "decompression.h"
#include "pj.h"
#include "macros.h"
#include "constants.h"

int decode_block(char* source, char block[8][8]) {
    // puts elements from the encoded component into the block in zig zag pattern untill it reaches 127
    // fills the rest of the block with zeroes
    // returns the (number of elements before 127) + 1
    // this function is not very memory safe -
    // the caller needs to check if char* source has enought elements left
    int c = 0;
    int i;
    for (int d = 0; d < 15; d++) {
        if (d >= 8) {
            i = 7;
        }
        else {
            i = d;
        }
        if (d % 2 == 0) {
            while (i >= 0 && (d - i) < 8) {
                if (source[c] == 127) {
                    block[i][d - i] = 0;
                }
                else {
                    block[i][d - i] = source[c];
                    c++;
                }
                i--;
            }
        }
        else {
            while (i >= 0 && (d - i) < 8) {
                if (source[c] == 127) {
                    block[d - i][i] = 0;
                }
                else {
                    block[d - i][i] = source[c];
                    c++;
                }
                i--;
            }
        }
    }
    return c + 1;
}

void dequantize_block(char block[8][8], double freq[8][8], double Q[8][8]) {
    // block is decoded directly from the file, thus it's double
    // freq is the resulting frequency matrix
    // Q contains constants
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            freq[i][j] = (double)(block[i][j]) * Q[i][j];
        }
    }
}

char get_value(double freq[8][8], double cosine[8][8], int x, int y) {
    // computes the cosine sum function value for the given 8x8 block cell
    // freq is the frequency matrix, cosines are precomputed before calling
    // x and y are coordinates within the block
    double sum = 0;
    double add = 0;
    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            add = freq[u][v] * cosine[x][u] * cosine[y][v];
            if (v == 0) {
                add /= sqrt(2);
            }
            if (u == 0) {
                add /= sqrt(2);
            }
            sum += add;
        }
    }
    sum *= 0.25;
    return (char)sum;
}

void reverse_DCT(double freq[8][8], char block[8][8], double cosine[8][8]) {
    // puts the cosine sum function values to their block cells
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            block[x][y] = get_value(freq, cosine, x, y);
        }
    }
}

void denormalize_block(char block[8][8], unsigned char result[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            result[i][j] = (unsigned char)((int)block[i][j] + 128);
        }
    }
}

void recover_component(char* encoded_c, int length, unsigned char* c, int height, int width) {
    // this function will recover original component of the image block by block
    // in case of cb and cr components height and width must be divided by 2
    // height and width must include the image padding
    static double freq[8][8];
    static char block[8][8];
    static unsigned char result[8][8];
    static double cosine[8][8];
    static double Q[8][8] = Q_MATRIX;
    precompute_cosines(cosine);
    int p = 0;
    for (int i = 0; i < height; i += 8) {
        for (int j = 0; j < width; j += 8) {
            if (p > length) {
                assert(1 == 2);
            }
            p += decode_block(encoded_c + p, block);
            dequantize_block(block, freq, Q);
            reverse_DCT(freq, block, cosine);
            denormalize_block(block, result);
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    c[RM_INDEX(width, i + x, j + y)] = result[x][y];
                }
            }
        }
    }
}