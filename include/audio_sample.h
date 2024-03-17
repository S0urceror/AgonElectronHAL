#ifndef AUDIO_SAMPLE_H
#define AUDIO_SAMPLE_H

#include <memory>
#include <unordered_map>

#include "types.h"
#include "audio_channel.h"
#include "buffer_stream.h"

struct AudioSample {
	AudioSample(std::vector<std::shared_ptr<BufferStream>> streams, uint8_t format, uint32_t sampleRate = AUDIO_DEFAULT_SAMPLE_RATE, uint16_t frequency = 0) :
		blocks(streams), format(format), sampleRate(sampleRate), baseFrequency(frequency) {}
	~AudioSample();

	int8_t getSample(uint32_t & index, uint32_t & blockIndex);
	void seekTo(uint32_t position, uint32_t & index, uint32_t & blockIndex, int32_t & repeatCount);
	uint32_t getSize();

	std::vector<std::shared_ptr<BufferStream>> blocks;
	uint8_t			format;				// Format of the sample data
	uint32_t		sampleRate;			// Sample rate of the sample
	uint16_t		baseFrequency = 0;	// Base frequency of the sample
	int32_t			repeatStart = 0;	// Start offset for repeat, in samples
	int32_t			repeatLength = -1;	// Length of the repeat section in samples, -1 means to end of sample
	// std::unordered_map<uint8_t, std::weak_ptr<AudioChannel>> channels;	// Channels playing this sample
};

#endif // AUDIO_SAMPLE_H