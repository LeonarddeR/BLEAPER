# BLEAPER
BLE-MIDI support for REAPER

As an owner of a Bluetooth MIDI supporting device having difficulties attaching the device wired, I decided to write a plugin for REAPER that allows Bluetooth Midi input to work on Windows 10 using the new MIDI API found in there.

In short, after downloading the 32 or 64 bit version of the plugin and extracting it in %appdata%\reaper\userPlugins, you can use the plugin by selecting BLEAPER Midi Input from the Extensions menu and choosing the proper input. The input is routed to the Virtual Midi Keyboard, so setting that as an input on one of your midi tracks should get you going.