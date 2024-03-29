//
// Title:			Agon Video BIOS - Audio class
// Author:			Dean Belfield
// Contributors:	Steve Sims (enhancements for more sophisticated audio support)
//                  S0urceror (PlatformIO compatibility)
// Created:			05/09/2022
// Last Updated:	08/10/2023
//
// Modinfo:

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <fabgl.h>

#include "audio_channel.h"
#include "audio_sample.h"
#include "types.h"

// audio channels and their associated tasks
std::unordered_map<uint8_t, std::shared_ptr<AudioChannel>> audioChannels;
std::vector<TaskHandle_t, psram_allocator<TaskHandle_t>> audioHandlers;

std::unordered_map<uint16_t, std::shared_ptr<AudioSample>> samples;	// Storage for the sample data

fabgl::SoundGenerator *soundGenerator;				// audio handling sub-system
std::mutex soundGeneratorMutex;

// Audio channel driver task
//
void audioDriver(void * parameters) {
	uint8_t channelNum = *(uint8_t *)parameters;
	auto channel = make_shared_psram<AudioChannel>(channelNum);

	audioChannels[channelNum] = channel;
	while (true) {
		channel->loop();
		vTaskDelay(1);
	}
}

void initAudioChannel(uint8_t channel) {
	xTaskCreatePinnedToCore(audioDriver,  "audioDriver",
		4096,						// This stack size can be checked & adjusted by reading the Stack Highwater
		&channel,					// Parameters
		PLAY_SOUND_PRIORITY,		// Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		&audioHandlers[channel],	// Task handle
		ARDUINO_RUNNING_CORE
	);
}

void audioTaskAbortDelay(uint8_t channel) {
	if (audioHandlers[channel]) {
		xTaskAbortDelay(audioHandlers[channel]);
	}
}

void audioTaskKill(uint8_t channel) {
	if (audioHandlers[channel]) {
		vTaskDelete(audioHandlers[channel]);
		audioChannels[channel]->detachSoundGenerator();
		audioHandlers[channel] = nullptr;
		audioChannels.erase(channel);
		//debug_log("audioTaskKill: channel %d killed\n\r", channel);
	} else {
		//debug_log("audioTaskKill: channel %d not found\n\r", channel);
	}
}

// Change the sample rate
//
void setSampleRate(uint16_t sampleRate) {
	// make a new sound generator and re-attach all our active channels
	if (sampleRate == 65535) {
		sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;
	}
	auto lock = std::unique_lock<std::mutex>(soundGeneratorMutex);
	// detatch the old sound generator
	if (soundGenerator) {
		soundGenerator->play(false);
		for (const auto &channelPair : audioChannels) {
			if (!channelPair.second) {
				//debug_log("duff channel pair for channel %d, skipping\n\r", channelPair.first);
				audioChannels.erase(channelPair.first);
				continue;
			}
			channelPair.second->detachSoundGenerator();
		}
		delete soundGenerator;
	}
	// delete the old sound generator
	soundGenerator = new fabgl::SoundGenerator(sampleRate);
	for (const auto &channelPair : audioChannels) {
		channelPair.second->attachSoundGenerator();
	}
	soundGenerator->play(true);
}

// Initialise the sound driver
//
void initAudio() {
	// make new sound generator
	setSampleRate(AUDIO_DEFAULT_SAMPLE_RATE);
	audioHandlers.reserve(MAX_AUDIO_CHANNELS);
	//debug_log("initAudio: we have reserved %d channels\n\r", audioHandlers.capacity());
	for (uint8_t i = 0; i < AUDIO_CHANNELS; i++) {
		initAudioChannel(i);
	}
}

// Channel enabled?
//
bool channelEnabled(uint8_t channel) {
	return channel < MAX_AUDIO_CHANNELS && audioChannels.find(channel) != audioChannels.end();
}

// Play a note
//
uint8_t playNote(uint8_t channel, uint8_t volume, uint16_t frequency, uint16_t duration) {
	if (channelEnabled(channel)) {
		return audioChannels[channel]->playNote(volume, frequency, duration);
	}
	return 1;
}

// Get channel status
//
uint8_t getChannelStatus(uint8_t channel) {
	if (channelEnabled(channel)) {
		return audioChannels[channel]->getStatus();
	}
	return -1;
}

// Set channel volume
//
uint8_t setVolume(uint8_t channel, uint8_t volume) {
	if (channel == 255) {
		if (volume == 255) {
			return soundGenerator->volume();
		}
		soundGenerator->setVolume(volume < 128 ? volume : 127);
		return soundGenerator->volume();
	} else if (channelEnabled(channel)) {
		return audioChannels[channel]->setVolume(volume);
	}
	return 255;
}

// Set channel frequency
//
uint8_t setFrequency(uint8_t channel, uint16_t frequency) {
	if (channelEnabled(channel)) {
		return audioChannels[channel]->setFrequency(frequency);
	}
	return 0;
}

// Set channel waveform
//
uint8_t setWaveform(uint8_t channel, int8_t waveformType, uint16_t sampleId) {
	if (channelEnabled(channel)) {
		auto channelRef = audioChannels[channel];
		return channelRef->setWaveform(waveformType, channelRef, sampleId);
	}
	return 0;
}

// Seek to a position on a channel
//
uint8_t seekTo(uint8_t channel, uint32_t position) {
	if (channelEnabled(channel)) {
		return audioChannels[channel]->seekTo(position);
	}
	return 0;
}

// Set channel duration
//
uint8_t setDuration(uint8_t channel, uint16_t duration) {
	if (channelEnabled(channel)) {
		return audioChannels[channel]->setDuration(duration);
	}
	return 0;
}

// Set channel sample rate
//
uint8_t setSampleRate(uint8_t channel, uint16_t sampleRate) {
	if (channel == 255) {
		// set underlying sample rate
		setSampleRate(sampleRate);
		return 0;
	}
	if (channelEnabled(channel)) {
		return audioChannels[channel]->setSampleRate(sampleRate);
	}
	return 0;
}

// Enable a channel
//
uint8_t enableChannel(uint8_t channel) {
	if (channelEnabled(channel)) {
		// channel already enabled
		return 1;
	}
	if (channel >= 0 && channel < MAX_AUDIO_CHANNELS && audioChannels[channel] == nullptr) {
		// channel not enabled, so enable it
		initAudioChannel(channel);
		return 1;
	}
	return 0;
}

// Disable a channel
//
uint8_t disableChannel(uint8_t channel) {
	if (channelEnabled(channel)) {
		audioTaskKill(channel);
		return 1;
	}
	return 0;
}

// Clear a sample
//
uint8_t clearSample(uint16_t sampleId) {
	//debug_log("clearSample: sample %d\n\r", sampleId);
	if (samples.find(sampleId) == samples.end()) {
		//debug_log("clearSample: sample %d not found\n\r", sampleId);
		return 1;
	}
	samples[sampleId] = nullptr;
	//debug_log("reset sample\n\r");
	return 0;
}

// Reset samples
//
void resetSamples() {
	//debug_log("resetSamples\n\r");
	samples.clear();
}


