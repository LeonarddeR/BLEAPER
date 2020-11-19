#include <string>
#include <winrt/Windows.Foundation.Collections.h>
#include "MidiDeviceService.h"

using namespace std;
using namespace winrt;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Midi;
using namespace winrt::Windows::Foundation;

MidiDeviceService::MidiDeviceService(hstring selector) : deviceSelector(selector)
{
	deviceWatcher = DeviceInformation::CreateWatcher(selector);
	addedEventToken = deviceWatcher.Added({ this, &MidiDeviceService::deviceWatcher_added });
	removedEventToken = deviceWatcher.Removed({ this, &MidiDeviceService::deviceWatcher_removed });
	updatedEventToken = deviceWatcher.Updated({ this, &MidiDeviceService::deviceWatcher_updated });
	enumerationCompletedEventToken = deviceWatcher.EnumerationCompleted({ this, &MidiDeviceService::deviceWatcher_enumerationCompleted });
}

MidiDeviceService::~MidiDeviceService()
{
	deviceWatcher.Added(addedEventToken);
	deviceWatcher.Removed(removedEventToken);
	deviceWatcher.Updated(updatedEventToken);
	deviceWatcher.EnumerationCompleted(enumerationCompletedEventToken);
}

void MidiDeviceService::startWatching()
{
	if (deviceWatcher.Status() != DeviceWatcherStatus::Started)
	{
		deviceWatcher.Start();
	}
}

void MidiDeviceService::stopWatching()
{
	if (deviceWatcher.Status() != DeviceWatcherStatus::Stopped)
	{
		deviceWatcher.Stop();
	}
}

hstring MidiDeviceService::getCurrentDeviceFromConfig()
{
	auto deviceId = GetExtState(CONFIG_SECTION, "midiInDevice");
	return to_hstring(deviceId);
}

void MidiDeviceService::setCurrentDeviceInConfig(hstring const& deviceId)
{
	SetExtState(CONFIG_SECTION, "midiInDevice", to_string(deviceId).c_str(), true);
}

fire_and_forget MidiDeviceService::connectDevice(hstring deviceId, bool updateConfig = true)
{
	if (currentDevice)
	{
		if (messageReceivedEventToken)
		{
			currentDevice.MessageReceived(messageReceivedEventToken);
			messageReceivedEventToken.value = 0;
		}
		currentDevice.Close();
		currentDevice = nullptr;
	}
	if (!deviceId.empty())
	{
		auto deviceOp = MidiInPort::FromIdAsync(deviceId);
		co_await resume_after(1s);
		ShowMessageBox(s.str(), "eor", 0);
		try
		{
			currentDevice = co_await deviceOp;
		}
		catch (hresult_canceled const&)
		{
			currentDevice = nullptr;
		}
		catch (hresult_illegal_method_call const&)
		{
			currentDevice = nullptr;
		}
		if (!currentDevice)
		{
			ShowMessageBox("Couldn't connect to device.", "Connection Error", 0);
			co_return;
		}
		messageReceivedEventToken = currentDevice.MessageReceived({ this, &MidiDeviceService::midiInPort_messageReceived });
	}
	if (updateConfig)
	{
		configuredDeviceId = deviceId;
		setCurrentDeviceInConfig(configuredDeviceId);
	}
}

void MidiDeviceService::deviceWatcher_added(DeviceWatcher const& sender, DeviceInformation const& args)
{
	if (enumerationCompleted)
	{
		updateDevices();
	}
	if (configuredDeviceId == args.Id())
	{
		connectDevice(configuredDeviceId, false);
	}
}

void MidiDeviceService::deviceWatcher_removed(DeviceWatcher const& sender, DeviceInformationUpdate const& args)
{
	if (enumerationCompleted)
	{
		updateDevices();
		if (configuredDeviceId == args.Id())
		{
			// Disconnect
			connectDevice(L"", false);
		}
	}
}

void MidiDeviceService::deviceWatcher_updated(DeviceWatcher const& sender, DeviceInformationUpdate const& args)
{
	if (enumerationCompleted)
	{
		updateDevices();
	}
}

void MidiDeviceService::deviceWatcher_enumerationCompleted(DeviceWatcher const& sender, IInspectable args)
{
	enumerationCompleted = true;
	updateDevices();
}

void MidiDeviceService::updateDevices()
{
	deviceInformationCollection = DeviceInformation::FindAllAsync(deviceSelector).get();
}

void MidiDeviceService::midiInPort_messageReceived(MidiInPort const& source, MidiMessageReceivedEventArgs const& args)
{
	auto data = args.Message().RawData().data();
	StuffMIDIMessage(0, data[0], data[1], data[2]);
}