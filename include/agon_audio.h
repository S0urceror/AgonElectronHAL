//
// Title:			Agon Video BIOS - Audio class
// Author:			Dean Belfield
// Contributors:	Steve Sims (enhancements for more sophisticated audio support)
// Created:			05/09/2022
// Last Updated:	04/08/2023
//
// Modinfo:

#ifndef AGON_AUDIO_H
#define AGON_AUDIO_H

#include "audio_channel.h"

extern std::unordered_map<uint8_t, std::shared_ptr<AudioChannel>> audioChannels;

void audioDriver(void * parameters);
void initAudioChannel(uint8_t channel);
void audioTaskAbortDelay(uint8_t channel);
void audioTaskKill(uint8_t channel);
void setSampleRate(uint16_t sampleRate);
void initAudio();
bool channelEnabled(uint8_t channel);
uint8_t playNote(uint8_t channel, uint8_t volume, uint16_t frequency, uint16_t duration);
uint8_t getChannelStatus(uint8_t channel);
uint8_t setVolume(uint8_t channel, uint8_t volume);
uint8_t setFrequency(uint8_t channel, uint16_t frequency);
uint8_t setWaveform(uint8_t channel, int8_t waveformType, uint16_t sampleId);
uint8_t seekTo(uint8_t channel, uint32_t position);
uint8_t setDuration(uint8_t channel, uint16_t duration);
uint8_t setSampleRate(uint8_t channel, uint16_t sampleRate);
uint8_t enableChannel(uint8_t channel);
uint8_t disableChannel(uint8_t channel);
uint8_t clearSample(uint16_t sampleId);
void resetSamples();

#endif // AGON_AUDIO_H
