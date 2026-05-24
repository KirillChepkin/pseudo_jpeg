#include <stdlib.h>
#include "memory.h"

void mem_init(MEM* mem) {
    mem -> count = 0;
}

void append_pointer(MEM* mem, void* pointer) {
    mem -> pointers[mem -> count] = pointer;
    mem -> count++;
}

void free_pointer(MEM* mem, int idx) {
    free(mem -> pointers[idx]);
    mem -> pointers[idx] = NULL;
}

void free_all(MEM* mem) {
    for (int i = 0; i < mem -> count; i++){
        free(mem -> pointers[i]);
    }
}