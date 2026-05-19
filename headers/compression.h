#ifndef COMPRESSION_H
#define COMPRESSION_H

void downsample_component(unsigned char* c, unsigned char* dest, int height, int width);
int get_padding_size(int x, int d);
void pad_component(unsigned char* c, int height, int width, int new_height, int new_width);

#endif