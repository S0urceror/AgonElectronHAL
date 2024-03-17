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
#include <fabgl.h>

#include "enhanced_samples_generator.h"
#include "audio_sample.h"

EnhancedSamplesGenerator::EnhancedSamplesGenerator(std::shared_ptr<AudioSample> sample)
	: _sample(sample), repeatCount(0), index(0), blockIndex(0), frequency(0), previousSample(0), currentSample(0), samplesPerGet(1.0), fractionalSampleOffset(0.0)
{}

void EnhancedSamplesGenerator::setFrequency(int value) {
	frequency = value;
	samplesPerGet = calculateSamplerate(value);
}

void EnhancedSamplesGenerator::setSampleRate(int value) {
	WaveformGenerator::setSampleRate(value);
	samplesPerGet = calculateSamplerate(frequency);
}

int EnhancedSamplesGenerator::getSample() {
	if (duration() == 0) {
		return 0;
	}

	// if we've moved far enough along, read the next sample
	while (fractionalSampleOffset >= 1.0) {
		previousSample = currentSample;
		currentSample = getNextSample();
		fractionalSampleOffset = fractionalSampleOffset - 1.0;
	}

	 // Interpolate between the samples to reduce aliasing
	int sample = currentSample * fractionalSampleOffset + previousSample * (1.0-fractionalSampleOffset);

	fractionalSampleOffset = fractionalSampleOffset + samplesPerGet;

	// process volume
	sample = sample * volume() / 127;

	decDuration();

	return sample;
}

int EnhancedSamplesGenerator::getDuration(uint16_t frequency) {
	// TODO this will produce an incorrect duration if the sample rate for the channel has been
	// adjusted to differ from the underlying audio system sample rate
	// At this point it's not clear how to resolve this, so we'll assume it hasn't been adjusted
	return !_sample ? 0 : (_sample->getSize() * 1000 / sampleRate()) / calculateSamplerate(frequency);
}

void EnhancedSamplesGenerator::seekTo(uint32_t position) {
	_sample->seekTo(position, index, blockIndex, repeatCount);

	// prepare our fractional sample data for playback
	fractionalSampleOffset = 0.0;
	previousSample = _sample->getSample(index, blockIndex);
	currentSample = _sample->getSample(index, blockIndex);
}

double EnhancedSamplesGenerator::calculateSamplerate(uint16_t frequency) {
	auto baseFrequency = _sample->baseFrequency;
	auto frequencyAdjust = baseFrequency > 0 ? (double)frequency / (double)baseFrequency : 1.0;
	return frequencyAdjust * ((double)_sample->sampleRate / (double)(sampleRate()));
}

int8_t EnhancedSamplesGenerator::getNextSample() {
	auto sample = _sample->getSample(index, blockIndex);

	// looping magic
	repeatCount--;
	if (repeatCount == 0) {
		// we've reached the end of the repeat section, so loop back
		seekTo(_sample->repeatStart);
	}

	return sample;
}