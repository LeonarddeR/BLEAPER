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

using namespace std;
using namespace winrt;
using namespace winrt::Windows::Devices::Midi;

MidiDeviceService *midiDeviceService = nullptr;

// We support 16 devices and the 'None' device
int midiInCommands[17];

map<int, hstring> midiInDeviceCommandToDeviceIdMap;

void menuhook(const char *menuidstr, HMENU hMenu, int flag)
{
	if (strcmp(menuidstr, "Main extensions"))
	{
		return;
	}
	if (flag == 0)
	{
		int iPos = GetMenuItemCount(hMenu);
		auto hSubMenu = CreatePopupMenu();
		InsertMenu(hMenu, iPos, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)hSubMenu, "BLEAPER MIDI Input");
		InsertMenu(hSubMenu, 0, MF_BYPOSITION | MF_STRING, midiInCommands[0], "None");
	}
	else if (flag == 1)
	{
		ShowConsoleMsg(menuidstr);
		midiInDeviceCommandToDeviceIdMap.clear();
		auto devCol = midiDeviceService->getDeviceInformationCollection();
		for (unsigned int i = 0; i < devCol.Size(); i++)
		{
			auto devInfo = devCol.GetAt(i);
			midiInDeviceCommandToDeviceIdMap[midiInCommands[i + 1]] = devInfo.Id();
		}
	}
}

bool handleCommand(int command, int flag)
{
	const auto it = midiInDeviceCommandToDeviceIdMap.find(command);
	if (it != midiInDeviceCommandToDeviceIdMap.end())
	{
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
			midiDeviceService = new MidiDeviceService(MidiInPort::GetDeviceSelector());
			midiDeviceService->startWatching();
			rec->Register("hookcommand", (void *)handleCommand);
			for (unsigned int i = 0; i <= 17; i++)
			{
				stringstream s;
				s << "BLEAPER_MIDI_IN" << i;
				midiInCommands[i] = rec->Register("command_id", (void *)s.str().c_str());
			}
			rec->Register("hookcustommenu", menuhook);
			AddExtensionsMainMenu();
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
