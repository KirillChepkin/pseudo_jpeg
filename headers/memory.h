#ifndef MEMORY_H
#define MEMORY_H

#define N 100

typedef struct memory_holder_t {
    void* pointers[N];
    int count;
} MEM;

void mem_init(MEM* mem);
void append_pointer(MEM* mem, void* pointer);
void free_pointer(MEM* mem, int idx);
void free_all(MEM* mem);

#endif