#include "base_ui.h"
#include <printf.h>
#include <string.h>

int buttonCount = 0;

void testing(){
    printf("Test\n");
}

void LeftPanel(){
    CLAY({
             .id = CLAY_ID("LeftPanel"),
             .layout = {
                .sizing = {CLAY_SIZING_PERCENT(0.16), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(4),
                .childGap = 4,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY
         }) {
        CLAY({
                 .id = CLAY_ID("LeftPanelText"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                       CLAY_SIZING_GROW()}, .padding = CLAY_PADDING_ALL(0)},
             }) {
            CLAY_TEXT(GetString("Left Panel"), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

void MiddlePanel(){
    CLAY({
             .id = CLAY_ID("MiddlePanel"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(4),
                .childGap = 4,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND
         }) {
        CLAY({
                 .id = CLAY_ID("MiddlePanelText"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                         CLAY_SIZING_FIT()}, .padding = CLAY_PADDING_ALL(0)},
             }) {
            CLAY_TEXT(GetString("Middle Panel"), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

void RightPanel(){
    CLAY({
             .id = CLAY_ID("RightPanel"),
             .layout = {
                .sizing = {CLAY_SIZING_PERCENT(0.15), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(4),
                .childGap = 4,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY
         }) {
        CLAY({
                 .id = CLAY_ID("RightPanelText"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                        CLAY_SIZING_FIT()}, .padding = CLAY_PADDING_ALL(0)},
             }) {
            CLAY_TEXT(GetString("Right Panel"), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

void BaseContainer(){
    CLAY({
             .id = CLAY_ID("BaseContainer"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(0),
                .childGap = 4,
                .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND
         }) {
        LeftPanel();
        MiddlePanel();
        RightPanel();
    }
}
void TopBar(){
    CLAY({
             .id = CLAY_ID("TopBar"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_FIT()},
                .padding = CLAY_PADDING_ALL(8),
                .childGap = 4,
                .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER},
                .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY
         }) {

        STRING title = StringCreate("LLDAW", default_arena);
        title = StringConcat(&title, " - ");
        title = StringConcat(&title, VERSION);

        CLAY_TEXT(GetString(title.data), CLAY_TEXT_CONFIG(
                {.textColor = COLOR_SCHEME_TEXT, .fontSize = 32, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
    }
}

void BottomBar(){
    CLAY({
             .id = CLAY_ID("BottomBar"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_PERCENT(0.15)},
                .padding = CLAY_PADDING_ALL(4),
                .childGap = 4,
                .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY
         }) {
        CreateButton("Test", testing);
    }
}
void Root(){
    CLAY({
             .id = CLAY_ID("RootContainer"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(8),
                .childGap = 8,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND
         }) {
        TopBar();
        BaseContainer();
        BottomBar();
    }
}

void RenderMainUI(void) {
    Root();

    buttonCount = 0;
}

void HandleButtonInteraction(Clay_ElementId id, Clay_PointerData data, intptr_t userData){
    // We will assume userData is the function pointer to the callback
    void (*callback)() = (void (*)())userData;

    if (data.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME){
        printf("Button %d was clicked\n", id.id);
        if ((void *) userData != NULL){
            callback();
        }else{
            printf("No callback set for button %d\n", id.id);
        }
    }
}

/// Accepts a lambda function as a callback
void CreateButton(const char* text, void (*callback)()) {
    buttonCount++;
    Clay_ElementId id = CLAY_IDI("BUTTON", buttonCount);
    CLAY({
             .id = (id), .layout = {.sizing = {CLAY_SIZING_FIT(),
                                               CLAY_SIZING_FIT()}, .padding = CLAY_PADDING_ALL(
                8)}, .backgroundColor = COLOR_SCHEME_BUTTON
         }) {
        Clay_OnHover(HandleButtonInteraction, (intptr_t) callback);
        CLAY({
                 .id = CLAY_ID_LOCAL("ButtonText"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                          CLAY_SIZING_GROW()},
                    .padding = CLAY_PADDING_ALL(0)},
             }) {
            CLAY_TEXT(GetString(text), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

Clay_String GetString(const char *string) {
    Clay_String result;
    result = (CLAY__INIT(Clay_String){.length=strlen(string), .chars=(string)});
    return result;
}
