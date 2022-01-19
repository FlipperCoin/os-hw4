#include <unistd.h>

void* smalloc(size_t size) {
    if (size == 0 || size > 100000000) {
        return NULL;
    }
    
    void* ptr = sbrk(size);
    if ((void*)-1 == ptr) {
        return NULL;
    }

    return ptr;
}