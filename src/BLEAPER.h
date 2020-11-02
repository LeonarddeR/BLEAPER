#pragma once

#include "reaper/reaper_plugin.h"
#include "reaper/reaper_plugin_functions.h"

#define REAPERAPI_MINIMAL
#define REAPERAPI_WANT_ShowConsoleMsg

class RTMidiIn: public midi_Input {
	public:
	RTMidiIn();
	virtual ~RTMidiIn();
	virtual void start() override { };
	virtual void stop() override {} ;
	virtual MIDI_eventlist *GetReadBuf() { return nullptr; }
	virtual void SwapBufs(unsigned int timestamp) override {};

};

midi_Input* createMidiInput();