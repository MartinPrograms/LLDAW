#include "state.h"

#include <stddef.h>

// Define play, pause, and stop function callbacks with the format void return type and void* argument
void (*PlayCallback)(void*) = NULL;
void (*PauseCallback)(void*) = NULL;
void (*StopCallback)(void*) = NULL;