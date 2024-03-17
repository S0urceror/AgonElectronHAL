#ifndef ENHANCED_SAMPLES_GENERATOR_H
#define ENHANCED_SAMPLES_GENERATOR_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <fabgl.h>

#include "audio_sample.h"
#include "types.h"

// Enhanced samples generator
//
class EnhancedSamplesGenerator : public WaveformGenerator {
	public:
		EnhancedSamplesGenerator(std::shared_ptr<AudioSample> sample);

		void setFrequency(int value);
		void setSampleRate(int value);
		int getSample();

		int getDuration(uint16_t frequency);

		void seekTo(uint32_t position);
	private:
		std::shared_ptr<AudioSample> _sample;

		uint32_t	index;				// Current index inside the current sample block
		uint32_t	blockIndex;			// Current index into the sample data blocks
		int32_t		repeatCount;		// Sample count when repeating
		// TODO consider whether repeatStart and repeatLength may need to be here
		// which would allow for per-channel repeat settings

		int			frequency;
		int			previousSample;
		int			currentSample;
		double		samplesPerGet;
		double		fractionalSampleOffset;

		double calculateSamplerate(uint16_t frequency);
		int8_t getNextSample();
};


#endif // ENHANCED_SAMPLES_GENERATOR_H
