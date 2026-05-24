#ifndef COMPRESSION_H
#define COMPRESSION_H

void downsample_component(unsigned char* c, unsigned char* dest, int height, int width);
int get_padding_size(int x, int d);
void pad_component(unsigned char* c, int height, int width, int new_height, int new_width);
void normalize_block(unsigned char* matrix, double block[8][8], int matrix_height,
    int matrix_width, int row, int column);
void precompute_cosines(double cosine[8][8]);
double get_coefficient(double block[8][8], double cosine[8][8], int u, int v);
void compute_block_DCT(double block[8][8], double cosine[8][8], double freq[8][8]);
void quantize_block(double freq[8][8], double Q[8][8]);
void round_block(double freq[8][8]);
int zig_zag_RLE(double freq[8][8], char* result);
int get_encoded_component(unsigned char* c, int height, int width, char* result, int d);

#endif