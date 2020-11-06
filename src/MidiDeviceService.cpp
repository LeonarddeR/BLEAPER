#include "MidiDeviceService.h"

using namespace std;
using namespace winrt;
using namespace winrt::Windows::Devices::Enumeration;
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
}

void MidiDeviceService::updateDevices()
{
	deviceInformationCollection = DeviceInformation::FindAllAsync(deviceSelector).get();
}