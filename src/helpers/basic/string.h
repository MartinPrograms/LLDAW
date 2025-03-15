#ifndef STRING_H
#define STRING_H
#pragma once

#define CONSOLE_BUFFER_SIZE 1024
#include "arena.h"

typedef struct {
    char* data;
    size_t length;
    ARENA* arena; // optional
} STRING;

/// Reads a line from the console.
STRING ConsoleReadLine(const char* prompt, ARENA* arena);

/// Reads a single key from the console.
STRING ConsoleReadKey(const char* prompt, ARENA* arena);

STRING StringCreate(const char* data, ARENA* arena);
STRING StringConcat(STRING* string, const char* data);
STRING StringConcatString(STRING* string, STRING* data);
STRING StringFromInt(int value, ARENA* arena);

#endif // STRING_H