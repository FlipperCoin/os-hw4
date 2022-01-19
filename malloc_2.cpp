#include <unistd.h>
#include <string.h>

struct MallocMetadata {
    size_t size;
    bool is_free;
};

void* begin = sbrk(0);

MallocMetadata* _get_block(void* p) {
    return (MallocMetadata*)((char*)p-sizeof(MallocMetadata));
}

void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

void* smalloc(size_t size) {
    if (size == 0 || size > 100000000) {
        return NULL;
    }

    void* end = sbrk(0);
    void* ptr = begin;
    while (ptr != NULL && ptr != end) {
        MallocMetadata* meta = (MallocMetadata*)ptr;
        if (meta->is_free && meta->size >= size) {
            meta->is_free = false;
            return (char*)ptr+sizeof(MallocMetadata);
        }

        ptr = (char*)ptr + (sizeof(MallocMetadata) + meta->size);
    }

    if ((void*)-1 == sbrk(sizeof(MallocMetadata) + size)) {
        return NULL;
    }

    ((MallocMetadata*)end)->size = size;
    ((MallocMetadata*)end)->is_free = false;
    // *(MallocMetadata*)end = {.size = size, .is_free = false};
    return (char*)end + sizeof(MallocMetadata);
}

void* scalloc(size_t num, size_t size) {
    if (size == 0 || num == 0) 
    {
        return NULL;
    }
    
    void* ptr = smalloc(num * size);
    if (ptr == NULL) return NULL;

    memset(ptr, 0, num * size);

    return ptr;
}

void sfree(void* p) {
    if(p == NULL) return;
    
    MallocMetadata* block = _get_block(p);
    block->is_free = true;
    return;
}

void* srealloc(void* oldp, size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    if(oldp == NULL)
    {
        return smalloc(size);
    }

    MallocMetadata* block = _get_block(oldp);

    if (block->size < size) {
        void* newp = smalloc(size);
        if (newp == NULL) return NULL;

        memcpy(newp, oldp, size);

        sfree(oldp);
        return newp;
    }

    return oldp;
}

size_t _num_free_blocks() {
    size_t count = 0;
    
    void* end = sbrk(0);
    for (void* p = begin; p != end; p = ((char*)p + sizeof(MallocMetadata) + ((MallocMetadata*)p)->size)) {
        if (((MallocMetadata*)p)->is_free) count++;
    }

    return count;
}

size_t _num_free_bytes() {
    size_t count = 0;
    
    void* end = sbrk(0);
    for (void* p = begin; p != end; p = ((char*)p + sizeof(MallocMetadata) + ((MallocMetadata*)p)->size)) {
        if (((MallocMetadata*)p)->is_free) count += ((MallocMetadata*)p)->size;
    }

    return count;
}

size_t _num_allocated_blocks() {
    size_t count = 0;
    
    void* end = sbrk(0);
    for (void* p = begin; p != end; p = ((char*)p + sizeof(MallocMetadata) + ((MallocMetadata*)p)->size)) {
        count++;
    }

    return count;
}

size_t _num_allocated_bytes() {
    size_t count = 0;
    
    void* end = sbrk(0);
    for (void* p = begin; p != end; p = ((char*)p + sizeof(MallocMetadata) + ((MallocMetadata*)p)->size)) {
        count += ((MallocMetadata*)p)->size;
    }

    return count;
}


size_t _num_meta_data_bytes() {
    size_t count = 0;
    
    void* end = sbrk(0);
    for (void* p = begin; p != end; p = ((char*)p + sizeof(MallocMetadata) + ((MallocMetadata*)p)->size)) {
        count += sizeof(MallocMetadata);
    }

    return count;
}

size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}