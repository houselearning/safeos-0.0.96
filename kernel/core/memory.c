#include "memory.h"
#include "memory_layout.h"

typedef struct block {
    size_t size;
    struct block *next;
    int free;
} block_t;

static block_t *free_list = NULL;
static size_t heap_alloc_count = 0;

void memory_init(void) {
    /* Initialize heap as one large free block */
    free_list = (block_t *)HEAP_START;
    free_list->size = HEAP_SIZE - sizeof(block_t);
    free_list->next = NULL;
    free_list->free = 1;
    heap_alloc_count = 0;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    /* Align size to 4 bytes */
    size = (size + 3) & ~3;

    /* Check for overflow - if requested size would exceed heap bounds, fail */
    if (size > HEAP_SIZE - sizeof(block_t)) {
        return NULL;  /* Requested allocation too large for heap */
    }

    block_t *current = free_list;
    block_t *prev = NULL;

    while (current) {
        if (current->free && current->size >= size + sizeof(block_t)) {
            /* Verify we're still within heap bounds before allocation */
            uintptr_t alloc_end = (uintptr_t)current + sizeof(block_t) + size;
            if (alloc_end > HEAP_END) {
                return NULL;  /* This block would exceed heap boundary */
            }

            /* Split the block */
            block_t *new_block = (block_t *)((char *)current + sizeof(block_t) + size);
            new_block->size = current->size - size - sizeof(block_t);
            new_block->next = current->next;
            new_block->free = 1;

            current->size = size;
            current->next = new_block;
            current->free = 0;
            heap_alloc_count++;

            return (void *)((char *)current + sizeof(block_t));
        }
        prev = current;
        current = current->next;
    }

    return NULL; /* No suitable block found; heap exhausted */
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
