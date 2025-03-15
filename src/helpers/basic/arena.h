#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

/// A simple arena allocator.
typedef struct ARENA {
     /// The buffer used for allocation.
    void *buffer;
    /// The capacity of the buffer.
    size_t capacity;
    /// The current offset in the buffer.
    size_t offset;
} ARENA;

/// Creates a new arena with the given capacity.
ARENA *arena_create(size_t capacity);

/// Destroys the given arena.
void arena_destroy(ARENA *arena);

/// Allocates a block of memory of the given size from the arena.
void *arena_alloc(ARENA *arena, size_t size);

/// Resets the arena by setting the offset to 0.
void arena_reset(ARENA *arena);

/// [0,1] percentage of the arena used.
float percentage_arena_used(ARENA *arena);

#endif // ARENA_H
