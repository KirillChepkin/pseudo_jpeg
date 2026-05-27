#ifndef DECOMPRESSION_H
#define DECOMPRESSION_H

int decode_block(char* source, char block[8][8]);
void dequantize_block(char block[8][8], double freq[8][8], double Q[8][8]);
char get_value(double freq[8][8], double cosine[8][8], int x, int y);
void reverse_DCT(double freq[8][8], char block[8][8], double cosine[8][8]);
void recover_component(char* encoded_c, int length, unsigned char* c, int height, int width);

#endif