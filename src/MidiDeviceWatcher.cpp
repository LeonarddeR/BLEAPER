#include "MidiDeviceWatcher.h"

using namespace std;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Foundation;

MidiDeviceWatcher::MidiDeviceWatcher(wstring selector) : deviceSelector(selector)
{
	deviceWatcher = DeviceInformation::CreateWatcher(selector);
	addedEventToken = deviceWatcher.Added({this, &MidiDeviceWatcher::deviceWatcher_added});
	removedEventToken = deviceWatcher.Removed({this, &MidiDeviceWatcher::deviceWatcher_removed});
	updatedEventToken = deviceWatcher.Updated({this, &MidiDeviceWatcher::deviceWatcher_updated});
	enumerationCompletedEventToken = deviceWatcher.EnumerationCompleted({this, &MidiDeviceWatcher::deviceWatcher_enumerationCompleted});
}

MidiDeviceWatcher::~MidiDeviceWatcher()
{
	deviceWatcher.Added(addedEventToken);
	deviceWatcher.Removed(removedEventToken);
	deviceWatcher.Updated(updatedEventToken);
	deviceWatcher.EnumerationCompleted(enumerationCompleted);
}

void MidiDeviceWatcher::start()
{
	if (deviceWatcher.Status() != DeviceWatcherStatus::Started)
	{
		deviceWatcher.Start();
	}
}

void MidiDeviceWatcher::stop()
{
	if (deviceWatcher.Status() != DeviceWatcherStatus::Stopped)
	{
		deviceWatcher.Stop();
	}
}
