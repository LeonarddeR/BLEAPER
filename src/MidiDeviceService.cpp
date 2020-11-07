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
	addedEventToken = deviceWatcher.Added({this, &MidiDeviceService::deviceWatcher_added});
	removedEventToken = deviceWatcher.Removed({this, &MidiDeviceService::deviceWatcher_removed});
	updatedEventToken = deviceWatcher.Updated({this, &MidiDeviceService::deviceWatcher_updated});
	enumerationCompletedEventToken = deviceWatcher.EnumerationCompleted({this, &MidiDeviceService::deviceWatcher_enumerationCompleted});
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
	auto idMultiByte = GetExtState(CONFIG_SECTION, "midiInDevice");
	auto idLength = MultiByteToWideChar(CP_UTF8, 0, idMultiByte, -1, nullptr, 0);
	wstring id(idLength, L'\0');
	auto charsConverted = MultiByteToWideChar(CP_UTF8, 0, idMultiByte, -1, id.data(), idLength);
	return hstring{id};
}

void MidiDeviceService::setCurrentDeviceInConfig(hstring const &deviceId)
{
	if (deviceId.empty())
	{
		SetExtState(CONFIG_SECTION, "midiInDevice", "", true);
		return;
	}
	auto idLength = WideCharToMultiByte(CP_UTF8, 0, deviceId.c_str(), -1, nullptr, 0, nullptr, nullptr);
	string idMultiByte(idLength, '\0');
	auto charsConverted = WideCharToMultiByte(CP_UTF8, 0, deviceId.c_str(), -1, idMultiByte.data(), idLength, nullptr, nullptr);
	SetExtState(CONFIG_SECTION, "midiInDevice", idMultiByte.c_str(), true);
}

void MidiDeviceService::connectDevice(hstring const &deviceId, bool updateConfig = true)
{
	if (currentDevice)
	{
		if (messageReceivedEventToken)
		{
			currentDevice.MessageReceived(messageReceivedEventToken);
		}
		currentDevice = nullptr;
	}
	if (deviceId.empty())
	{
		return;
	}
	currentDevice = MidiInPort::FromIdAsync(deviceId).get();
	setCurrentDeviceInConfig(deviceId);
	messageReceivedEventToken = currentDevice.MessageReceived({this, &MidiDeviceService::midiInPort_messageReceived});
}

void MidiDeviceService::deviceWatcher_added(DeviceWatcher const &sender, DeviceInformation const &args)
{
	if (enumerationCompleted)
	{
		updateDevices();
	}
}

void MidiDeviceService::deviceWatcher_removed(DeviceWatcher const &sender, DeviceInformationUpdate const &args)
{
	if (enumerationCompleted)
	{
		updateDevices();
	}
}

void MidiDeviceService::deviceWatcher_updated(DeviceWatcher const &sender, DeviceInformationUpdate const &args)
{
	if (enumerationCompleted)
	{
		updateDevices();
	}
}

void MidiDeviceService::deviceWatcher_enumerationCompleted(DeviceWatcher const &sender, IInspectable args)
{
	enumerationCompleted = true;
	updateDevices();
	DeviceInformation dev = deviceInformationCollection.GetAt(0);
	connectDevice(dev.Id());
}

void MidiDeviceService::updateDevices()
{
	deviceInformationCollection = DeviceInformation::FindAllAsync(deviceSelector).get();
}

void MidiDeviceService::midiInPort_messageReceived(MidiInPort const &source, MidiMessageReceivedEventArgs const &args)
{
	auto data = args.Message().RawData().data();
	StuffMIDIMessage(0, data[0], data[1], data[2]);
}