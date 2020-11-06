#pragma once

#include <windows.h>
#include "reaper/reaper_plugin.h"
#include "reaper/reaper_plugin_functions.h"

#define REAPERAPI_MINIMAL
#define REAPERAPI_WANT_ShowConsoleMsg
#define REAPERAPI_WANT_StuffMIDIMessage
#define REAPERAPI_WANT_AddExtensionsMainMenu

void menuhook(const char *menuidstr, HMENU hMenu, int flag);

