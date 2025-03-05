#include "arena.h"
#include <stdlib.h>

ARENA *arena_create(size_t capacity) {
    ARENA *arena = (ARENA *)malloc(sizeof(ARENA));
    if (!arena)
        return NULL;

    arena->buffer = (char *)malloc(capacity);
    if (!arena->buffer) {
        free(arena);
        return NULL;
    }

    arena->capacity = capacity;
    arena->offset = 0;
    return arena;
}

void arena_destroy(ARENA *arena) {
    if (arena) {
        free(arena->buffer);
        free(arena);
    }
}

void *arena_alloc(ARENA *arena, size_t size) {
    // Check if there's enough space in the arena.
    if (arena->offset + size > arena->capacity) {
        return NULL;
    }

    // Allocate memory by returning the current position and advancing the offset.
    void *ptr = arena->buffer + arena->offset;
    arena->offset += size;
    return ptr;
}

void arena_reset(ARENA *arena) {
    if (arena) {
        arena->offset = 0;
    }
}
