#include "state.h"

// Define play, pause, and stop function callbacks
void (*PlayCallback)(void) = nullptr;
void (*PauseCallback)(void) = nullptr;
void (*StopCallback)(void) = nullptr;
