#include <string>
#include <cstring>

#define REAPERAPI_IMPLEMENT
#include "BLEAPER.h"

using namespace std;

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
			return 1;
		}
		else
		{
			return 0;
		}
	}
}
