#include "arena.h"

#include <stdio.h>
#include <stdlib.h>

ARENA* default_arena = NULL;
ARENA* frame_arena = NULL;
ARENA* buffer_arena = NULL;

ARENA *arena_create(size_t capacity) {
    ARENA *arena = (ARENA *)malloc(sizeof(ARENA));
    if (!arena)
        return NULL;

    arena->buffer = malloc(capacity);
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
        if (arena->buffer) {
            free(arena->buffer);
        }
        free(arena);
    }
}

void *arena_alloc(ARENA *arena, size_t size) {
    // Check if there's enough space in the arena.
    if (arena->offset + size > arena->capacity) {
        printf("[WARNING] Arena is full, cannot allocate %zu bytes.\n", size);
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

float percentage_arena_used(ARENA *arena) {
    return (float)arena->offset / (float)arena->capacity;
}
