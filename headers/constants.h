#ifndef CONSTANTS_H
#define CONSTANTS_H

#define COMPRESS_MODE 0
#define DECOMPRESS_MODE 1

// error codes
#define INVALID_FILE_ERROR_CODE -1
#define MEMORY_ISSUE_ERROR_CODE -2
#define UNSUPPORTED_INPUT_ERROR_CODE -3

#define HELP_MESSAGE "This program is designed to convert\
.bmp images with pixel depth 24 bpp or 32bpp to a custom .pj file format using the lossy compression algorithm\
based on the Discrete Cosine Transform (DCT). It can convert a generated .pj file back to bmp with losses. \
Use the fololowing command format:\n\
./pj compress <path to the input bmp> <desired path to the compressed file>\n\
./pj decompress <path to the generated file> <desired path to the recovered image>\n\
./pj --help\n"

#define Q_MATRIX {{16, 11, 10, 16, 24, 40, 51, 61},\
                {12, 12, 14, 19, 26, 58, 60, 55},\
                {14, 13, 16, 24, 40, 57, 69, 56},\
                {14, 17, 22, 29, 51, 87, 80, 62},\
                {18, 22, 37, 56, 68, 109, 103, 77},\
                {24, 35, 55, 64, 81, 104, 113, 92},\
                {49, 64, 78, 87, 103, 121, 120, 101},\
                {72, 92, 95, 98, 112, 100, 103, 99}};

#endif