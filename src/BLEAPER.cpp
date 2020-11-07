#include <Windows.h>
#include <string>
#include <cstring>
#include <sstream>
#include <map>
#include <winrt/Windows.Devices.Midi.h>
#include "WDL/win32_helpers.h"

#define REAPERAPI_IMPLEMENT
#include "BLEAPER.h"
#include "MidiDeviceService.h"

using namespace std;
using namespace winrt;
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

// We support 16 devices and the 'None' device
int midiInCommands[17];

map<int, hstring> midiInDeviceCommandToDeviceIdMap;

bool handleCommand(int command, int flag)
{
	const auto it = midiInDeviceCommandToDeviceIdMap.find(command);
	if (it != midiInDeviceCommandToDeviceIdMap.end())
	{
	}
	return true;
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
			rec->Register("hookcommand", (void *)handleCommand);
			rec->Register("hookcustommenu", menuhook);
			for (unsigned int i = 0; i == 17; i++)
			{
				stringstream s;
				s << "BLEAPER_MIDI_IN" << i;
				midiInCommands[i] = rec->Register("command_id", (void *)s.str().c_str());
			}
			return 1;
		}
		else
		{
			//rec->Register("-hookcustommenu", menuhook);
			//rec->Register("-hookcommand", (void *)handleCommand);
			midiDeviceService->stopWatching();
			delete midiDeviceService;

			return 0;
		}
	}
}
