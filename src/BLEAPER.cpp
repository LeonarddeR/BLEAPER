#include <Windows.h>
#include <Windowsx.h>
#include <string>
#include <cstring>
#include <sstream>
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

hstring hstringFromCharString(const char *str)
{
	auto idLength = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	wstring wstr(idLength, L'\0');
	auto charsConverted = MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr.data(), idLength);
	return hstring{wstr};
}

const char *hstringToCharString(hstring const &hstr)
{
	auto idLength = WideCharToMultiByte(CP_UTF8, 0, hstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	string str(idLength, '\0');
	auto charsConverted = WideCharToMultiByte(CP_UTF8, 0, hstr.c_str(), -1, str.data(), idLength, nullptr, nullptr);
	return str.c_str();
}

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
		new DeviceDialog();
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
			// try connecting;
			hstring currentId = midiDeviceService->getCurrentDeviceFromConfig();
			if (!currentId.empty())
			{
				midiDeviceService->connectDevice(currentId, false);
			}
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

DeviceDialog::DeviceDialog()
{
	deviceInformationCollection = midiDeviceService->getDeviceInformationCollection();
	this->dialogHwnd = CreateDialog(pluginHInstance, MAKEINTRESOURCE(ID_DEVICES_DLG), mainHwnd, DeviceDialog::dialogProc);
	SetWindowLongPtr(this->dialogHwnd, GWLP_USERDATA, (LONG_PTR)this);
	comboHwnd = GetDlgItem(dialogHwnd, ID_MIDI_IN_DEVICE);
	WDL_UTF8_HookComboBox(comboHwnd);
	ComboBox_AddString(this->comboHwnd, "None");
	int initialComboSel = 0;
	for (UINT i = 0; i < deviceInformationCollection.Size(); i++)
	{
		auto const &devInfo = deviceInformationCollection.GetAt(i);
		ComboBox_AddString(this->comboHwnd, hstringToCharString(devInfo.Name()));
		if (initialComboSel != 0 && devInfo.Id() == midiDeviceService->getCurrentDeviceFromConfig())
		{
			initialComboSel = i + 1;
		}
	}
	ComboBox_SetCurSel(this->comboHwnd, initialComboSel);
	ShowWindow(this->dialogHwnd, SW_SHOWNORMAL);
}

DeviceDialog::~DeviceDialog()
{
	DestroyWindow(this->dialogHwnd);
}

void DeviceDialog::onOk()
{
	int curComboSel = ComboBox_GetCurSel(this->comboHwnd);
	hstring newDevice;
	if (curComboSel > 0)
	{
		newDevice = deviceInformationCollection.GetAt(curComboSel - 1).Id();
	}
	midiDeviceService->connectDevice(newDevice, true);
}

INT_PTR CALLBACK DeviceDialog::dialogProc(HWND dialogHwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DeviceDialog *dialog = (DeviceDialog *)GetWindowLongPtr(dialogHwnd, GWLP_USERDATA);
	switch (msg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			dialog->onOk();
			delete dialog;
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			delete dialog;
			return TRUE;
		}
		break;
	case WM_CLOSE:
		delete dialog;
		return TRUE;
	}
	return FALSE;
}
