#include <Windows.h>
#include <string>
#include <cstring>
#include <winrt/Windows.Devices.Midi.h>
#include "WDL/win32_helpers.h"

#define REAPERAPI_IMPLEMENT
#include "BLEAPER.h"
#include "MidiDeviceService.h"

using namespace std;
using namespace winrt::Windows::Devices::Midi;

MidiDeviceService *midiDeviceService = nullptr;

void menuhook(const char *menuidstr, HMENU hMenu, int flag)
{
	switch (flag)
	{
	case 0:
		if (!strcmp(menuidstr, "Main extensions"))
		{
			int iPos = GetMenuItemCount(hMenu);
			auto hSubMenu = InsertSubMenu(hMenu, iPos, "BLEAPER MIDI Input");
		}
		break;
	case 1:
		break;
	}
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
			midiDeviceService = new MidiDeviceService(MidiInPort::GetDeviceSelector());
			midiDeviceService->startWatching();
			AddExtensionsMainMenu();
			rec->Register("hookcustommenu", menuhook);
			return 1;
		}
		else
		{
			rec->Register("-Ehookcustommenu", menuhook);
			midiDeviceService->stopWatching();
			delete midiDeviceService;

			return 0;
		}
	}
}
