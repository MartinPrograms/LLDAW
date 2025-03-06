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
    STRING result;
    result.data = (char*)arena_alloc(arena, strlen(data) + 1);
    if (result.data != NULL) {
        strcpy_s(result.data, strlen(data) + 1, data);
        result.length = strlen(data);
    }
    return result;
}

STRING StringConcat(STRING *string, const char *data) {
    STRING result;
    result.data = (char*)malloc(string->length + strlen(data) + 1);
    if (result.data != NULL) {
        strcpy(result.data, string->data);
        strcat(result.data, data);
        result.length = string->length + strlen(data);
    }
    return result;
}
