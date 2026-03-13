#include "memory.h"

#define HEAP_START 0x400000  // 4MB
#define HEAP_SIZE  0x100000  // 1MB heap

typedef struct block {
    size_t size;
    struct block *next;
    int free;
} block_t;

static block_t *free_list = NULL;

void memory_init(void) {
    free_list = (block_t *)HEAP_START;
    free_list->size = HEAP_SIZE - sizeof(block_t);
    free_list->next = NULL;
    free_list->free = 1;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    // Align size to 4 bytes
    size = (size + 3) & ~3;

    block_t *current = free_list;
    block_t *prev = NULL;

    while (current) {
        if (current->free && current->size >= size + sizeof(block_t)) {
            // Split the block
            block_t *new_block = (block_t *)((char *)current + sizeof(block_t) + size);
            new_block->size = current->size - size - sizeof(block_t);
            new_block->next = current->next;
            new_block->free = 1;

            current->size = size;
            current->next = new_block;
            current->free = 0;

            return (void *)((char *)current + sizeof(block_t));
        }
        prev = current;
        current = current->next;
    }

    return NULL; // No suitable block found
}

void kfree(void *ptr) {
    if (!ptr) return;

    block_t *block = (block_t *)((char *)ptr - sizeof(block_t));
    block->free = 1;

    // Coalesce with next block if free
    if (block->next && block->next->free) {
        block->size += sizeof(block_t) + block->next->size;
        block->next = block->next->next;
    }

    // Coalesce with previous block if free (need to traverse list)
    block_t *current = free_list;
    block_t *prev = NULL;
    while (current && current != block) {
        prev = current;
        current = current->next;
    }
    if (prev && prev->free) {
        prev->size += sizeof(block_t) + block->size;
        prev->next = block->next;
    }
}
