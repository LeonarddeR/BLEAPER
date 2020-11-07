#include <Windows.h>
#include <string>
#include <cstring>
#include <sstream>
#include <map>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Midi.h>
#include <winrt/Windows.Foundation.Collections.h>

#define REAPERAPI_IMPLEMENT
#include "BLEAPER.h"
#include "MidiDeviceService.h"
#include "WDL/win32_utf8.h"
#include "resource.h"

using namespace std;
using namespace winrt;
using namespace winrt::Windows::Devices::Midi;

HINSTANCE pluginHInstance;
HWND mainHwnd;
MidiDeviceService *midiDeviceService = nullptr;

gaccel_register_t devicesDialogGaccel{{0, 0, 0}, "BLEAPER: Open MIDI devices dialog"};

void menuhook(const char *menuidstr, HMENU hMenu, int flag)
{
	if (strcmp(menuidstr, "Main extensions") || flag != 0)
	{
		return;
	}
	InsertMenu(hMenu, GetMenuItemCount(hMenu), MF_BYPOSITION | MF_STRING, devicesDialogGaccel.accel.cmd, "BLEAPER MIDI Device...");
}

bool handleCommand(int command, int flag)
{
	if (command == devicesDialogGaccel.accel.cmd)
	{
		openDeviceDialog();
		return true;
	}
	return false;
}

extern "C"
{
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
	{
		if (rec)
		{
			// Load.
			if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc || REAPERAPI_LoadAPI(rec->GetFunc) != 0)
			{
				return 0; // Incompatible.
			}
			pluginHInstance = hInstance;
			mainHwnd = rec->hwnd_main;
			midiDeviceService = new MidiDeviceService(MidiInPort::GetDeviceSelector());
			midiDeviceService->startWatching();
			rec->Register("hookcommand", (void *)handleCommand);
			devicesDialogGaccel.accel.cmd = rec->Register("command_id", (void *)"BLEAPER_OPENDEVICESDIALOG");
			rec->Register("gaccel", &devicesDialogGaccel);
			rec->Register("hookcustommenu", menuhook);
			AddExtensionsMainMenu();
			return 1;
		}
		else
		{
			midiDeviceService->stopWatching();
			delete midiDeviceService;
			return 0;
		}
	}
}

INT_PTR CALLBACK device_dialogProc(HWND dialog, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			DestroyWindow(dialog);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(dialog);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(dialog);
		return TRUE;
	}
	return FALSE;
}

void openDeviceDialog()
{
	auto dialog = CreateDialog(pluginHInstance, MAKEINTRESOURCE(ID_DEVICES_DLG), mainHwnd, device_dialogProc);
	auto combo = GetDlgItem(dialog, ID_MIDI_IN_DEVICE);
	WDL_UTF8_HookComboBox(combo);
	ShowWindow(dialog, SW_SHOWNORMAL);
}
