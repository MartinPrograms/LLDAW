#ifndef BASE_UI_H
#define BASE_UI_H

#include <clay.h>
#include "../../../helpers/include.h"
#include "ui_settings.h"

void RenderMainUI();
void CreateButton(const char *text, void (*callback)());

Clay_String GetString(const char *string);

#endif // BASE_UI_H