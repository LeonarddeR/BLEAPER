#pragma once

#include <windows.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include "reaper/reaper_plugin.h"
#include "reaper/reaper_plugin_functions.h"

#define REAPERAPI_MINIMAL
#define REAPERAPI_WANT_ShowConsoleMsg
#define REAPERAPI_WANT_ShowMessageBox
#define REAPERAPI_WANT_StuffMIDIMessage
#define REAPERAPI_WANT_AddExtensionsMainMenu
#define REAPERAPI_WANT_GetExtState
#define REAPERAPI_WANT_SetExtState

void menuhook(const char* menuidstr, HMENU hMenu, int flag);
bool handleCommand(int command, int flag);
void openDeviceDialog();

class DeviceDialog;

const char CONFIG_SECTION[] = "bleaper";

class DeviceDialog
{
public:
	DeviceDialog();
	~DeviceDialog();

private:
	HWND dialogHwnd;
	HWND comboHwnd;
	winrt::Windows::Devices::Enumeration::DeviceInformationCollection deviceInformationCollection{ nullptr };

	void onOk();

	static INT_PTR CALLBACK dialogProc(HWND dialogHwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};