#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "custom.h"
#include "ui_renderer.h"

Font* fonts = NULL;
int fontCount = 0;
Clay_Arena arena = {0};

void (*UIRender)(void) = NULL;
bool CustomScrollCapture = false;

void HandleClayErrors(Clay_ErrorData errorData) {
    // See the Clay_ErrorData struct for more information
    printf("%s", errorData.errorText.chars);
}

Clay_RenderCommandArray DrawUI(float sizeX, float sizeY, float mouseX, float mouseY, bool clickM1, float scrollDeltaX, float scrollDeltaY, float deltaTime) {
    Clay_SetLayoutDimensions((Clay_Dimensions) {sizeX, sizeY});
    Clay_SetPointerState((Clay_Vector2) {mouseX, mouseY}, clickM1);
    if (!CustomScrollCapture) {
        Clay_UpdateScrollContainers(true, (Clay_Vector2) {scrollDeltaX, scrollDeltaY}, deltaTime);
    }
    CustomScrollCapture = false;
    Clay_BeginLayout();

    UIRender();

    Clay_RenderCommandArray layout = Clay_EndLayout();
    return layout;
}

#include "raylib.h"
#include "raymath.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "ui_settings.h"
#include "components.h"

static inline Clay_Dimensions Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    // Measure string size for Font
    Clay_Dimensions textSize = { 0 };

    float maxTextWidth = 0.0f;
    float lineTextWidth = 0;

    float textHeight = config->fontSize;
    // ignore userData, we don't need it for raylib
    // but still have to use it or the compiler will complain
    (void)userData;
    Font fontToUse = fonts[config->fontId];
    // Font failed to load, likely the fonts are in the wrong place relative to the execution dir.
    // RayLib ships with a default font, so we can continue with that built in one.
    if (!fontToUse.glyphs) {
        fontToUse = GetFontDefault();
    }

    float scaleFactor = (float)config->fontSize/(float)fontToUse.baseSize;

    for (int i = 0; i < text.length; ++i)
    {
        if (text.chars[i] == '\n') {
            maxTextWidth = fmax(maxTextWidth, lineTextWidth);
            lineTextWidth = 0;
            continue;
        }
        int index = text.chars[i] - 32;
        if (fontToUse.glyphs[index].advanceX != 0) lineTextWidth += fontToUse.glyphs[index].advanceX;
        else lineTextWidth += (fontToUse.recs[index].width + fontToUse.glyphs[index].offsetX);
    }

    maxTextWidth = fmax(maxTextWidth, lineTextWidth);

    textSize.width = maxTextWidth * scaleFactor;
    textSize.height = textHeight;

    return textSize;
}

void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands)
{
    for (int j = 0; j < renderCommands.length; j++)
    {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        Clay_BoundingBox boundingBox = renderCommand->boundingBox;
        switch (renderCommand->commandType)
        {
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                // Raylib uses standard C strings so isn't compatible with cheap slices, we need to clone the string to append null terminator
                Clay_TextRenderData *textData = &renderCommand->renderData.text;
                char *cloned = (char *)malloc(textData->stringContents.length + 1);
                memcpy(cloned, textData->stringContents.chars, textData->stringContents.length);
                cloned[textData->stringContents.length] = '\0';
                Font fontToUse = fonts[textData->fontId];
                DrawTextEx(fontToUse, cloned, (Vector2){boundingBox.x, boundingBox.y}, (float)textData->fontSize, (float)textData->letterSpacing, CLAY_COLOR_TO_RAYLIB_COLOR(textData->textColor));
                free(cloned);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Texture2D imageTexture = *(Texture2D *)renderCommand->renderData.image.imageData;
                Clay_Color tintColor = renderCommand->renderData.image.backgroundColor;
                if (tintColor.r == 0 && tintColor.g == 0 && tintColor.b == 0 && tintColor.a == 0) {
                    tintColor = (Clay_Color) { 255, 255, 255, 255 };
                }
                DrawTextureEx(
                        imageTexture,
                        (Vector2){boundingBox.x, boundingBox.y},
                        0,
                        boundingBox.width / (float)imageTexture.width,
                        CLAY_COLOR_TO_RAYLIB_COLOR(tintColor));
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                BeginScissorMode((int)roundf(boundingBox.x), (int)roundf(boundingBox.y), (int)roundf(boundingBox.width), (int)roundf(boundingBox.height));
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                EndScissorMode();
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleRenderData *config = &renderCommand->renderData.rectangle;
                if (config->cornerRadius.topLeft > 0) {
                    float radius = (config->cornerRadius.topLeft * 2) / (float)((boundingBox.width > boundingBox.height) ? boundingBox.height : boundingBox.width);
                    DrawRectangleRounded((Rectangle) { boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height }, radius, 8, CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor));
                } else {
                    DrawRectangle(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor));
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderRenderData *config = &renderCommand->renderData.border;
                // Left border
                if (config->width.left > 0) {
                    DrawRectangle((int)roundf(boundingBox.x), (int)roundf(boundingBox.y + config->cornerRadius.topLeft), (int)config->width.left, (int)roundf(boundingBox.height - config->cornerRadius.topLeft - config->cornerRadius.bottomLeft), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Right border
                if (config->width.right > 0) {
                    DrawRectangle((int)roundf(boundingBox.x + boundingBox.width - config->width.right), (int)roundf(boundingBox.y + config->cornerRadius.topRight), (int)config->width.right, (int)roundf(boundingBox.height - config->cornerRadius.topRight - config->cornerRadius.bottomRight), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Top border
                if (config->width.top > 0) {
                    DrawRectangle((int)roundf(boundingBox.x + config->cornerRadius.topLeft), (int)roundf(boundingBox.y), (int)roundf(boundingBox.width - config->cornerRadius.topLeft - config->cornerRadius.topRight), (int)config->width.top, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Bottom border
                if (config->width.bottom > 0) {
                    DrawRectangle((int)roundf(boundingBox.x + config->cornerRadius.bottomLeft), (int)roundf(boundingBox.y + boundingBox.height - config->width.bottom), (int)roundf(boundingBox.width - config->cornerRadius.bottomLeft - config->cornerRadius.bottomRight), (int)config->width.bottom, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.topLeft > 0) {
                    DrawRing((Vector2) { roundf(boundingBox.x + config->cornerRadius.topLeft), roundf(boundingBox.y + config->cornerRadius.topLeft) }, roundf(config->cornerRadius.topLeft - config->width.top), config->cornerRadius.topLeft, 180, 270, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.topRight > 0) {
                    DrawRing((Vector2) { roundf(boundingBox.x + boundingBox.width - config->cornerRadius.topRight), roundf(boundingBox.y + config->cornerRadius.topRight) }, roundf(config->cornerRadius.topRight - config->width.top), config->cornerRadius.topRight, 270, 360, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.bottomLeft > 0) {
                    DrawRing((Vector2) { roundf(boundingBox.x + config->cornerRadius.bottomLeft), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft) }, roundf(config->cornerRadius.bottomLeft - config->width.top), config->cornerRadius.bottomLeft, 90, 180, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.bottomRight > 0) {
                    DrawRing((Vector2) { roundf(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight) }, roundf(config->cornerRadius.bottomRight - config->width.bottom), config->cornerRadius.bottomRight, 0.1, 90, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                Clay_CustomRenderData *config = &renderCommand->renderData.custom;
                CustomElement *element = (CustomElement *)config->customData;
                RenderCustomElement(element, boundingBox.width, boundingBox.height, boundingBox.x, boundingBox.y);
                break;
            }
            default: {
                printf("Error: unhandled render command.");
                exit(1);
            }
        }
    }
}

void RaylibRenderUI(Clay_RenderCommandArray renderCommands) {
    Clay_Raylib_Render(renderCommands);
    Reset();
}


void InitUI(float sizeX, float sizeY) {
    uint64_t totalMemorySize = 1024 * 1024 * 64; // 64MB
    arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
    Clay_SetMaxElementCount(16000 * 4);

    Clay_Initialize(arena, (Clay_Dimensions){sizeX, sizeY}, (Clay_ErrorHandler){HandleClayErrors, .userData = NULL});

    // MacOS high DPI fix
    int smallFont = STANDARD_FONT_SIZE;
    int largeFont = LARGE_FONT_SIZE;
    #if defined(__APPLE__)
    smallFont*=2;
    largeFont*=2;
    #endif
    Font small = LoadFontEx("resources/Roboto-Regular.ttf", smallFont, nullptr, 250); // standard font size is 16, but we want to double it, so we have a bit more quality
    Font large = LoadFontEx("resources/Roboto-Regular.ttf", largeFont, nullptr, 250); // standard font size is 16, but we want to double it, so we have a bit more quality

    Clay_SetMeasureTextFunction(Raylib_MeasureText, &small);

    fonts = malloc(sizeof(Font) * 2);
    fonts[0] = small;
    fonts[1] = large;
}

void SetUIRenderFunction(void (*renderFunction)(void)) {
    UIRender = renderFunction;
}
