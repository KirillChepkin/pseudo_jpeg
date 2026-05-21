#ifndef COMPRESSION_H
#define COMPRESSION_H

void downsample_component(unsigned char* c, unsigned char* dest, int height, int width);
int get_padding_size(int x, int d);
void pad_component(unsigned char* c, int height, int width, int new_height, int new_width);
void normalize_block(unsigned char* matrix, double block[8][8], int matrix_height, int matrix_width, int row, int column);

#endif