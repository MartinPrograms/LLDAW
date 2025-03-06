#pragma once

#if defined(__APPLE__) || defined(__linux__)
    #include <termios.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
#endif

#include "./basic/string.h" // Unused yes, but it's included to make it available in other files
#include "./basic/arena.h" // This however, is used. As the Init function creates an arena

ARENA* default_arena;

static inline void Init(size_t capacity) {
    // If it's macOS/linux we need to change the console mode to allow reading a single key
    #if defined(__APPLE__) || defined(__linux__)
        struct termios tattr;

        // Get the current terminal attributes.
        if (tcgetattr(STDIN_FILENO, &tattr) < 0) {
            perror("tcgetattr");
            exit(EXIT_FAILURE);
        }

        // Disable canonical mode and echo.
        tattr.c_lflag &= ~(ICANON);

        // Set the new attributes immediately.
        if (tcsetattr(STDIN_FILENO, TCSANOW, &tattr) < 0) {
            perror("tcsetattr");
            exit(EXIT_FAILURE);
        }
    #endif

    default_arena = arena_create(capacity);
}

static inline void Destroy() {
    // First destroy the default arena
    arena_destroy(default_arena);

    // Now if it's macOS/linux we need to reset the console mode
    #if defined(__APPLE__) || defined(__linux__)
        struct termios tattr;

        // Get the current terminal attributes.
        if (tcgetattr(STDIN_FILENO, &tattr) < 0) {
            perror("tcgetattr");
            exit(EXIT_FAILURE);
        }

        // Enable canonical mode and echo.
        tattr.c_lflag |= ICANON;

        // Set the new attributes immediately.
        if (tcsetattr(STDIN_FILENO, TCSANOW, &tattr) < 0) {
            perror("tcsetattr");
            exit(EXIT_FAILURE);
        }
    #endif
}