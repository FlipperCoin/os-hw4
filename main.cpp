#include "malloc_3.cpp"
// #include "malloc_2.cpp"
// #include "malloc_1.cpp"
#include "stdio.h"

int main() {
    // printf("\n");
    // printf("sbrk: %p\n", sbrk(0));
    // void* ptr1 = smalloc(10);
    // printf("ptr1: %p, sbrk: %p\n", ptr1, sbrk(0));
    // void* ptr2 = smalloc(15);
    // printf("ptr2: %p, sbrk: %p\n", ptr2, sbrk(0));
    // void* ptr3 = smalloc(8);
    // printf("ptr3: %p, sbrk: %p\n", ptr3, sbrk(0));

    void* ptr1 = smalloc(128*1024+2);

    void* ptr2 = scalloc(3,10);
    void* ptr3 = srealloc(NULL, 50);
    printf("ptr1: %p, size: %lu\n", ptr1, _get_block(ptr1)->size);
    printf("ptr2: %p, size: %lu\n", ptr2, _get_block(ptr2)->size);
    printf("ptr3: %p, size: %lu\n", ptr3, _get_block(ptr3)->size);

    sfree(ptr2);
    printf("ptr2: %p, size: %lu\n", ptr2, _get_block(ptr2)->size);
    void* ptr4 = smalloc(30);

    printf("ptr4: %p, size: %lu\n", ptr4, _get_block(ptr4)->size);
    sfree(ptr4);

    void* ptr5 = smalloc(60);
    printf("ptr5: %p, size: %lu\n", ptr5, _get_block(ptr5)->size);

    return 0;
}