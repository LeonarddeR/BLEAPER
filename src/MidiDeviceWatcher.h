#pragma once

#include <string>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Foundation.h>

class MidiDeviceWatcher
{
public:
	MidiDeviceWatcher(std::wstring selector);
	~MidiDeviceWatcher();
	void start();
	void stop();

private:
	void deviceWatcher_added(winrt::Windows::Devices::Enumeration::DeviceWatcher const &sender, winrt::Windows::Devices::Enumeration::DeviceInformation const &args);
	void deviceWatcher_removed(winrt::Windows::Devices::Enumeration::DeviceWatcher const &sender, winrt::Windows::Devices::Enumeration::DeviceInformationUpdate const &args);
	void deviceWatcher_updated(winrt::Windows::Devices::Enumeration::DeviceWatcher const &sender, winrt::Windows::Devices::Enumeration::DeviceInformationUpdate const &args);
	void deviceWatcher_enumerationCompleted(winrt::Windows::Devices::Enumeration::DeviceWatcher const &sender, winrt::Windows::Foundation::IInspectable args);

private:
	bool enumerationCompleted = false;
	winrt::Windows::Devices::Enumeration::DeviceWatcher deviceWatcher{nullptr};
	wstring deviceSelector;
	winrt::event_token addedEventToken;
	winrt::event_token removedEventToken;
	winrt::event_token updatedEventToken;
	winrt::event_token enumerationCompletedEventToken;
};