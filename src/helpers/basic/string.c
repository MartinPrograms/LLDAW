#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <conio.h>
#endif

#include "string.h"

#if defined(__APPLE__) || defined(__linux__)
// stupid function that's not available on macos/linux
typedef int errno_t;
errno_t strcpy_s(char *dest, size_t destsz, const char *src) {
    if (!dest || !src) return 1;
    if (strlen(src) >= destsz) return 1;
    strcpy(dest, src);
    return 0;
}
#endif

STRING ConsoleReadLine(const char* prompt, ARENA* arena) {
    STRING str;
    str.data = NULL;
    str.length = 0;

    printf("%s", prompt);

    char buffer[CONSOLE_BUFFER_SIZE];

    if (fgets(buffer, CONSOLE_BUFFER_SIZE, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        str.data = (char*)arena_alloc(arena, len + 1);
        if (str.data != NULL) {
            strcpy_s(str.data, len + 1, buffer);
            str.length = len;
        }
  }

  return str;
}

STRING ConsoleReadKey(const char *prompt, ARENA *arena) {
  STRING str;
  str.data = NULL;
  str.length = 0;

  printf("%s", prompt);

  // If windows
  #if defined(_WIN32)
fflush(stdout);
str.data = (char*)arena_alloc(arena, 2); // Allocate memory for 1 char + null terminator
if (str.data != NULL) {
    str.data[0] = (char)_getch();
    str.data[1] = '\0';
    str.length = 1;
}
  #endif

  // If macos/linux
  #if defined(__APPLE__) || defined(__linux__)
str.data = (char*)arena_alloc(arena, 2); // Allocate memory for 1 char + null terminator
if (str.data != NULL) {
    str.data[0] = (char)getchar();
    str.data[1] = '\0';
    str.length = 1;
}
  #endif

  printf("\n");

  return str;
}

STRING StringCreate(const char *data, ARENA *arena) {
    STRING str;
    str.data = NULL;
    str.length = 0;
    str.arena = arena;

    str.data = (char*)arena_alloc(arena, strlen(data) + 1);
    if (str.data != NULL) {
        strcpy_s(str.data, strlen(data) + 1, data);
        str.length = strlen(data);
    }

    return str;
}

STRING StringConcat(STRING *string, const char *data) {
    size_t new_length = string->length + strlen(data);
    char *new_data = (char*)arena_alloc(string->arena, new_length + 1);
    if (new_data != NULL) {
        strcpy_s(new_data, new_length + 1, string->data);
        strcpy_s(new_data + string->length, strlen(data) + 1, data);

        string->data = new_data;
        string->length = new_length;
    }

    return *string;
}

STRING StringConcatString(STRING *string, STRING *data) {
    size_t new_length = string->length + data->length;
    char *new_data = (char*)arena_alloc(string->arena, new_length + 1);
    if (new_data != NULL) {
        strcpy_s(new_data, new_length + 1, string->data);
        strcpy_s(new_data + string->length, data->length + 1, data->data);

        string->data = new_data;
        string->length = new_length;
    }

    return *string;
}

STRING StringFromInt(int value, ARENA *arena) {
    int len = snprintf(NULL, 0, "%d", value);
    char *result = (char *)arena_alloc(arena, len + 1);
    if (!result) {
        return (STRING){.data = NULL, .length = 0, .arena = arena};
    }
    snprintf(result, len + 1, "%d", value);
    return (STRING){.data = result, .length = len, .arena = arena};
}

