#include <unistd.h>
#include <string.h>

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

MallocMetadata* begin = NULL;
MallocMetadata* end = NULL;

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

    MallocMetadata* ptr = begin;
    while (ptr != NULL) {
        if (ptr->is_free && ptr->size >= size) {
            ptr->is_free = false;
            return (char*)ptr+sizeof(MallocMetadata);
        }

        ptr = ptr->next;
    }

    MallocMetadata* new_end = (MallocMetadata*)sbrk(sizeof(MallocMetadata) + size);
    if ((MallocMetadata*)-1 == new_end) {
        return NULL;
    }

    new_end->prev = NULL;
    new_end->next = NULL;
    new_end->size = size;
    new_end->is_free = false;

    if (begin == NULL) {
        begin = new_end;
    }

    if (end != NULL) {
        end->next = new_end;
        new_end->prev = end;
    }
    
    end = new_end;
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
    // if (block->prev != NULL) block->prev->next = block->next;
    // if (block->next != NULL) block->next->prev = block->prev;
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
    
    for (MallocMetadata* p = begin; p != NULL; p = p->next) {
        if (p->is_free) count++;
    }

    return count;
}

size_t _num_free_bytes() {
    size_t count = 0;
    
    for (MallocMetadata* p = begin; p != NULL; p = p->next) {
        if (p->is_free) count += p->size;
    }

    return count;
}

size_t _num_allocated_blocks() {
    size_t count = 0;
    
    for (MallocMetadata* p = begin; p != NULL; p = p->next) {
        count++;
    }

    return count;
}

size_t _num_allocated_bytes() {
    size_t count = 0;
    
    for (MallocMetadata* p = begin; p != NULL; p = p->next) {
        count += p->size;
    }

    return count;
}


size_t _num_meta_data_bytes() {
    size_t count = 0;
    
    for (MallocMetadata* p = begin; p != NULL; p = p->next) {
        count += sizeof(MallocMetadata);
    }

    return count;
}

size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}