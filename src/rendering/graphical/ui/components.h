#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <clay.h>

typedef struct {
    const char* text;
    void* userData;
    void (*callback)(void*);
    bool useUserData;
} ButtonData;

typedef struct {
    bool isOpen;
    int selectedIndex;
    const char** items;
} DropdownState;

extern int CustomComponentIndex;
void WaveformVisualizer(const float* buffer, int bufferSize);
void SpectrumVisualizer(const float* buffer, int bufferSize);

extern int ButtonCount;
void CreateButton(const char *text, void* userData, void (*callback)(void*));

void CreateDropdown(int id, const char** items, int itemCount, int selectedIndex, int* outIndex);
void CreateContextMenu(const char** items, int itemCount, void (*callback)(int));

void Reset();

Clay_String GetString(const char *string);

#endif