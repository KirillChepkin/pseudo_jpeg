#ifndef MACROS_H
#define MACROS_H

#define MEMORY_ISSUE(ret, bmp) {\
    if ((ret) == MEMORY_ISSUE_ERROR_CODE) {\
        fprintf(stderr, "Memory issue\n");\
        destroy_bmp(&(bmp));\
        return -1;\
    }\
}

#define ERROR_READING_FILE(ret, bmp, text) {\
    if ((ret) == INVALID_FILE_ERROR_CODE) {\
        fprintf(stderr, (text));\
        destroy_bmp(&bmp);\
        return -1;\
    }\
}

#define ERROR_UNSUPPORTED_INPUT(ret, bmp, text) {\
    if ((ret) == UNSUPPORTED_INPUT_ERROR_CODE) {\
        fprintf(stderr, (text));\
        destroy_bmp(&bmp);\
        return -1;\
    }\
}

#define ALLOC_ISSUE(pointer, bmp, text) {\
    if ((pointer) == NULL) {\
        destroy_bmp(&bmp);\
        fprintf(stderr, (text));\
        return -1;\
    }\
}

#define RM_INDEX(width, i, j) ((i) * (width) + j)

#endif