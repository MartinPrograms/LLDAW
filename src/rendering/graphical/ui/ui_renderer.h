#pragma once
#ifndef UI_RENDERER_H
#define UI_RENDERER_H
#include <clay.h>
#include <raylib.h>

extern Font* fonts;
extern int fontCount;

extern Clay_Arena arena;

// Function pointer to a user set function that calls all the calls to clay
extern void (*UIRender)(void);
extern bool CustomScrollCapture;

Clay_RenderCommandArray DrawUI(float sizeX, float sizeY, float mouseX, float mouseY, bool clickM1, float scrollDeltaX, float scrollDeltaY, float deltaTime); // This function draws the main ui
void RaylibRenderUI(Clay_RenderCommandArray renderCommands); // This function renders the ui
void InitUI(float sizeX, float sizeY); // This function initializes the ui
void SetUIRenderFunction(void (*renderFunction)(void)); // This function sets the render function

#endif // UI_RENDERER_H