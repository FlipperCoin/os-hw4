#include <unistd.h>
#include <string.h>

#define BINS_SIZE 128

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
    MallocMetadata* next_heap;
    MallocMetadata* prev_heap;
};

MallocMetadata* bins[BINS_SIZE];

MallocMetadata* begin = NULL;
MallocMetadata* end = NULL;

MallocMetadata* heap_begin = NULL;
MallocMetadata* heap_end = NULL;

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

MallocMetadata* _remove_free(MallocMetadata* ptr) {
    MallocMetadata **bin_head = &bins[ptr->size / 1024];
    if (ptr->next != NULL) ptr->next->prev = ptr->prev;
    if (ptr == *bin_head) 
    {
        *bin_head = ptr->next;
    }
    else 
    {
        ptr->prev->next = ptr->next;
    } 
    
    ptr->is_free = false;
    return ptr;
}

MallocMetadata* _combine(MallocMetadata* block) {
    MallocMetadata* prev_heap = block->prev_heap;
    MallocMetadata* next_heap = block->next_heap;
    
    if(prev_heap != NULL && prev_heap->is_free && next_heap != NULL && next_heap->is_free) {
        _remove_free(next_heap);
        _remove_free(prev_heap);
        prev_heap->size = prev_heap->size + block->size + next_heap->size;
        block = prev_heap;
    }
    else if (prev_heap != NULL && prev_heap->is_free) {
        _remove_free(prev_heap);
        prev_heap->size = prev_heap->size + block->size;
        block = prev_heap;
    }
    else if (next_heap != NULL && next_heap->is_free) {
        _remove_free(next_heap);
        block->size = block->size + next_heap->size;
    }

    return block;
}

MallocMetadata* _combine_realloc(MallocMetadata* block, size_t wanted_size) {
    MallocMetadata* prev_heap = block->prev_heap;
    MallocMetadata* next_heap = block->next_heap;
    
    if (prev_heap != NULL && prev_heap->is_free && prev_heap->size + block->size >= wanted_size) {
        _remove_free(prev_heap);
        prev_heap->size = prev_heap->size + block->size;
        block = prev_heap;
    }
    else if(next_heap == NULL)
    {
        if (prev_heap != NULL && prev_heap->is_free) {
            if ((void*)-1 == 
                sbrk(wanted_size + sizeof(MallocMetadata) - block->size - prev_heap->size - (2*sizeof(MallocMetadata))))
            {
                return NULL;
            }
            _remove_free(prev_heap);
            prev_heap->size = prev_heap->size + block->size;
            block = prev_heap;
        }
        else
        {
            if ((void*)-1 == (MallocMetadata*)sbrk(wanted_size - block->size))
            {
                return NULL;
            }
            block->size = wanted_size;
        }
    }
    else if (next_heap != NULL && next_heap->is_free && next_heap->size + block->size >= wanted_size) {
        _remove_free(next_heap);
        block->size = block->size + next_heap->size;
    }
    else if(prev_heap != NULL && prev_heap->is_free && next_heap != NULL && next_heap->is_free 
                && prev_heap->size + next_heap->size + block->size >= wanted_size) {
        _remove_free(next_heap);
        _remove_free(prev_heap);
        prev_heap->size = prev_heap->size + block->size + next_heap->size;
        block = prev_heap;
    }

    return block;
}


MallocMetadata* _divide_block(MallocMetadata* block, size_t size) {
    size_t orig_size = block->size;
    block->size = size;
    MallocMetadata* new_free = (MallocMetadata*)((char*)block + size + sizeof(MallocMetadata));
    new_free->size = orig_size - size - sizeof(MallocMetadata);
    if(block->next_heap) 
        block->next_heap->prev_heap = new_free;
    new_free->next_heap = block->next_heap;
    block->next_heap = new_free;    
    new_free->prev_heap = block;
    return new_free;
}

void _insert_free(MallocMetadata* block) {
    block->next = NULL;
    block->prev = NULL;
    block->is_free = true;

    _combine(block);

    size_t bin = block->size / 1024;
    if (bin > 127) {
        //TODO: Challenge 4
    }
    MallocMetadata* ptr = bins[bin];

    if(ptr == NULL)
    {
        bins[bin] = block;
        return;
    }
    
    if (block->size < ptr->size) {
        ptr->prev = block;
        block->next = ptr;
        bins[bin] = block;
        return;
    }
    while(ptr->next != NULL)
    {
        if (block->size < ptr->next->size) 
        {
            ptr->next->prev = block;
            block->next = ptr->next;
            block->prev = ptr;
            ptr->next = block;
            return;
        }
        ptr = ptr->next;
    }
    ptr->next = block;
    block->prev = ptr;
    return;
}

void _remove_allocated(MallocMetadata* block) {
    if (block == end) {
        end = block->prev;
    }
    if (block->next != NULL) block->next->prev = block->prev;

    if (block == begin) 
    {
        begin = block->next;
    }
    else 
    {
        block->prev->next = block->next;
    } 
    
    block->is_free = true;
    return;
}

void _insert_allocated(MallocMetadata* block) {
    block->prev = NULL;
    block->next = NULL;

    if (begin == NULL) {
        begin = block;
    }

    if (end != NULL) {
        end->next = block;
        block->prev = end;
    }

    end = block;
    block->is_free = false;
}

void* smalloc(size_t size) {
    if (size == 0 || size > 100000000) {
        return NULL;
    }

    size_t bin = size / 1024;
    if (bin > 127) {
        //TODO: Challenge 4
    }
    
    MallocMetadata* ptr = bins[bin];
    while (ptr != NULL) {
        if (ptr->size >= size) {
            ptr = _remove_free(ptr);
            if (ptr->size > (size + sizeof(MallocMetadata)) && (ptr->size - (size + sizeof(MallocMetadata))) >= 128) {
                MallocMetadata* new_free = _divide_block(ptr, size);
                _insert_free(new_free);
            }
            _insert_allocated(ptr);
            return (char*)ptr+sizeof(MallocMetadata);
        }

        ptr = ptr->next;
    }

    MallocMetadata* new_end;

    if (heap_end != NULL && heap_end->is_free) {
        new_end = (MallocMetadata*)sbrk(sizeof(MallocMetadata) + size - heap_end->size);
        if ((MallocMetadata*)-1 == new_end) {
            return NULL;
        }
        
        heap_end->size = size;
        _remove_free(heap_end);
        _insert_allocated(heap_end);
        return (char*)heap_end + sizeof(MallocMetadata);
    }

    new_end = (MallocMetadata*)sbrk(sizeof(MallocMetadata) + size);
    if ((MallocMetadata*)-1 == new_end) {
        return NULL;
    }

    new_end->size = size;
    new_end->next_heap = NULL;
    new_end->prev_heap = heap_end;
    if(heap_end != NULL)
    {
        heap_end->next_heap = new_end;
    }
    if(heap_begin == NULL)
        heap_begin = new_end;
    heap_end = new_end;
    _insert_allocated(new_end);
    return (char*)new_end + sizeof(MallocMetadata);
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
    _remove_allocated(block);
    _insert_free(block);
    return;
}

void* srealloc(void* oldp, size_t size) {
    if (size == 0 || size > 100000000) {
        return NULL;
    }
    
    if(oldp == NULL)
    {
        return smalloc(size);
    }

    MallocMetadata* block = _get_block(oldp);
    if (block->size < size) 
    {
        size_t orig_size = block->size;
        MallocMetadata* new_block = _combine_realloc(block,size);
        if (new_block == NULL) return NULL;
        if(new_block->size > orig_size)
        {
            _remove_allocated(block);
            
            void* newp = (char*)new_block + sizeof(MallocMetadata);

            memmove(newp, oldp, orig_size);

            if (new_block->size > (size + sizeof(MallocMetadata)) && new_block->size - (size + sizeof(MallocMetadata)) >= 128)
            {
                MallocMetadata* new_free = _divide_block(new_block, size);
                _insert_free(new_free);
            }
            
            _insert_allocated(new_block);

            return newp;
        }
        else
        {
            void* newp = smalloc(size);
            if (newp == NULL) return NULL;

            _remove_allocated(block);
            _insert_free(new_block);
            return (void*)((char*)new_block + sizeof(MallocMetadata));
        }
    }
    else if (block->size > (size + sizeof(MallocMetadata)) && block->size - (size + sizeof(MallocMetadata)) >= 128)
    {
        MallocMetadata* new_free = _divide_block(block, size);
        _insert_free(new_free);
    }

    return oldp;
}

size_t _num_free_blocks() {
    size_t count = 0;
    
    for (int i = 0; i < BINS_SIZE; i++)
    {
        for (MallocMetadata* p = bins[i]; p != NULL; p = p->next) 
        {
            count++;
        }
    }
    return count;
}

size_t _num_free_bytes() {
    size_t count = 0;
    
    for (int i = 0; i < BINS_SIZE; i++)
    {
        for (MallocMetadata* p = bins[i]; p != NULL; p = p->next) 
        {
            count += p->size;
        }
    }

    return count;
}

size_t _num_allocated_blocks() {
    size_t count = 0;
    
    for (MallocMetadata* p = begin; p != NULL; p = p->next) 
    {
        count++;
    }
    count += _num_free_blocks();
    
    return count;
}

size_t _num_allocated_bytes() {
    size_t count = 0;
    
    for (MallocMetadata* p = begin; p != NULL; p = p->next) {
        count += p->size;
    }
    count += _num_free_bytes();

    return count;
}


size_t _num_meta_data_bytes() {
    size_t count = 0;
    
    for (MallocMetadata* p = begin; p != NULL; p = p->next) {
        count += sizeof(MallocMetadata);
    }
    count += (_num_free_blocks() * sizeof(MallocMetadata));

    return count;
}

size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}