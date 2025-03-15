#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <clay.h>
#include "../../audio/generator.h"

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
void AdsrVisualizer(AdsrEnvelope envelope);
void TrackerMidiNote(int midi_note, int velocity, int* adjusted_midi_note, int* adjusted_midi_velocity, bool active, int generator_index, int cell);

extern int ButtonCount;
void CreateButton(const char *text, void* userData, void (*callback)(void*));

void CreateDropdown(int id, const char** items, int itemCount, int selectedIndex, int* outIndex);
void CreateContextMenu(const char** items, int itemCount, void (*callback)(int));

void Reset();

Clay_String GetString(const char *string);

#endif