#include <string>
#include <cstring>

#define REAPERAPI_IMPLEMENT
#include "BLEAPER.h"

using namespace std;

midi_Input* input = nullptr;

midi_Input* createMidiInput() { 
return new RTMidiIn();
}


extern "C" {

REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t* rec) {
	if (rec) {
		// Load.
		if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc || REAPERAPI_LoadAPI(rec->GetFunc) != 0) {
			return 0; // Incompatible.
		}
		input = createMidiInput();
		rec->Register("Something_horrible", (void*)input);
		return 1;
	} else {
		// Unload.
		delete input;
		input = nullptr;
		return 0;
	}
}

}
