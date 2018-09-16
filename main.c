#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#define META_SIZE sizeof(block_meta)

void *global_base = NULL;

typedef struct block_meta{
    size_t size;
    block_meta *next;
    int free;
    int magic;
}block_meta;


block_meta *find_free_block(block_meta **last, size_t size){
    block_meta *current = global_base;
    while(current && !(current->free && current->size >= size)){
        *last = current;
        current = current->next;
    }
    return current;
}
block_meta *request_space(block_meta *last, size_t size){
    block_meta *block;
    block = sbrk(0);
    void *request = sbrk(size + META_SIZE);
    assert((void*)block == request);
    if(request == (void*) -1){
        return NULL;
     }
    if(last){
         last->next = block;
     }

     block->size = size;
     block->next = NULL;
     block->free = 0;
     block->magic = 0x12345678;
     return block;
     
}
void* malloc(size_t size){
    block_meta *block;
    // TODO: align size?

    if(size <= 0) return NULL;
    if(!global_base){
        block = request_space(NULL, size);
        if(!block){
            return NULL;
        }
        global_base = block;
    } else {
        block_meta *last = global_base;
        block = find_free_block(&last,size);
        if(!block){
            block = request_space(last,size);
            if(!block) return NULL;
        } else {
            block->free = 0;
            block->magic = 0x77777777;
        }
    }
    return (block + 1);
}
void *realloc(void *ptr, size_t size){
    if(!ptr) return malloc(size);

    block_meta *block_ptr = get_block_ptr(ptr);
    if(block_ptr->size >= size) return ptr;
    void *new_ptr;
    new_ptr = malloc(size);
    if(!new_ptr) return NULL;
    memcpy(new_ptr, ptr, block_ptr->size);
    free(ptr);
    return new_ptr;
}
void *calloc(size_t nelem, size_t elsize) {
  size_t size = nelem * elsize; // TODO: check for overflow.
  void *ptr = malloc(size);
  memset(ptr, 0, size);
  return ptr;
}
block_meta *get_block_ptr(void *ptr){
    return (block_meta*)ptr - 1;
}
void free(void *ptr){
    if(!ptr) return;
    block_meta* block_ptr = get_block_ptr(ptr);
    assert(block_ptr->free == 0);
    assert(block_ptr->magic == 0x77777777 || block_ptr->magic == 0x12345678);
    block_ptr->free = 1;
    block_ptr->magic = 0x55555555;


}

int main(){
    printf("");
}









/*
    TODO: https://danluu.com/malloc-tutorial/
Personally, this sort of thing never sticks with me unless I work through some exercsies, so I'll leave a couple exercises here for anyone who's interested.

1. malloc is supposed to return a pointer “which is suitably aligned for any built-in type”. Does our malloc do that? If so, why? If not, fix the alignment. Note that “any built-in type” is basically up to 8 bytes for C because SSE/AVX types aren't built-in types.

2. Our malloc is really wasteful if we try to re-use an existing block and we don't need all of the space. Implement a function that will split up blocks so that they use the minimum amount of space necessary

3. After doing 2, if we call malloc and free lots of times with random sizes, we'll end up with a bunch of small blocks that can only be re-used when we ask for small amounts of space. Implement a mechanism to merge adjacent free blocks together so that any consecutive free blocks will get merged into a single block.

4. Find bugs in the existing code! I haven't tested this much, so I'm sure there are bugs, even if this basically kinda sorta works.

*/