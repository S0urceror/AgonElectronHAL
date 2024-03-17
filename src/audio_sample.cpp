//
// Title:			Agon Video BIOS - Audio class
// Author:			Dean Belfield
// Contributors:	Steve Sims (enhancements for more sophisticated audio support)
//                  S0urceror (PlatformIO compatibility)
// Created:			05/09/2022
// Last Updated:	08/10/2023
//
// Modinfo:

#include "audio_sample.h"

AudioSample::~AudioSample() {
	// iterate over channels
	// for (auto &channelPair : this->channels) {
	// 	auto channel = channelPair.second.lock();
	// 	if (channel) {
	// 		// Remove sample from channel
	//		debug_log("AudioSample: removing sample from channel %d\n\r", channel->channel());
	// 		// TODO change so only removes if channel is definitely set to this sample
	// 		channel->setWaveform(AUDIO_WAVE_DEFAULT, nullptr);
	// 	}
	// }
}

int8_t AudioSample::getSample(uint32_t & index, uint32_t & blockIndex) {
	// get the next sample
	if (blockIndex >= blocks.size()) {
		// we've reached the end of the sample, and haven't looped, so return 0 (silence)
		return 0;
	}

	auto block = blocks[blockIndex];
	int8_t sample = block->getBuffer()[index++];

	if (index >= block->size()) {
		// block reached end, move to next block
		index = 0;
		blockIndex++;
	}

	if (format == AUDIO_FORMAT_8BIT_UNSIGNED) {
		sample = sample - 128;
	}

	return sample;
}

void AudioSample::seekTo(uint32_t position, uint32_t & index, uint32_t & blockIndex, int32_t & repeatCount) {
	// NB repeatCount calculation here can result in zero, or a negative number,
	// or a number that's beyond the end of the sample, which is fine
	// it just means that the sample will never loop
	if (repeatLength < 0) {
		// repeat to end of sample
		repeatCount = getSize() - position;
	} else if (repeatLength > 0) {
		auto repeatEnd = repeatStart + repeatLength;
		repeatCount = repeatEnd - position;
	} else {
		repeatCount = 0;
	}

	blockIndex = 0;
	index = position;
	while (blockIndex < blocks.size() && index >= blocks[blockIndex]->size()) {
		index -= blocks[blockIndex]->size();
		blockIndex++;
	}
}

uint32_t AudioSample::getSize() {
	uint32_t samples = 0;
	for (auto block : blocks) {
		samples += block->size();
	}
	return samples;
}
