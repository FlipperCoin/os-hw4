#include "malloc_2.cpp"
#include "stdio.h"

int main() {
    void* ptr1 = smalloc(10);
    void* ptr2 = scalloc(3,10);
    void* ptr3 = srealloc(NULL, 50);
    // printf("ptr1: %p, isFree: %d, size: %lu\n", ptr1, _get_block(ptr1)->is_free, _get_block(ptr1)->size);
    // printf("ptr2: %p, isFree: %d, size: %lu\n", ptr2, _get_block(ptr2)->is_free, _get_block(ptr2)->size);
    // printf("ptr3: %p, isFree: %d, size: %lu\n", ptr3, _get_block(ptr3)->is_free, _get_block(ptr3)->size);

    sfree(ptr2);
    // printf("ptr2: %p, isFree: %d, size: %lu\n", ptr2, _get_block(ptr2)->is_free, _get_block(ptr2)->size);
    void* ptr4 = smalloc(30);

    // printf("ptr4: %p, isFree: %d, size: %lu\n", ptr4, _get_block(ptr4)->is_free, _get_block(ptr4)->size);
    sfree(ptr4);

    void* ptr5 = smalloc(60);
    printf("ptr5: %p, isFree: %d, size: %lu\n", ptr5, _get_block(ptr5)->is_free, _get_block(ptr5)->size);

    return 0;
}