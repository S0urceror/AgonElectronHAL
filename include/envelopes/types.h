//
// Title:			Audio Envelope support
// Author:			Steve Sims
// Created:			14/01/2024
// Last Updated:	14/01/2024

#ifndef ENVELOPE_TYPES_H
#define ENVELOPE_TYPES_H

#include <memory>

inline int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class VolumeEnvelope {
	public:
		virtual uint8_t getVolume(uint8_t baseVolume, uint32_t elapsed, int32_t duration) = 0;
		virtual bool isReleasing(uint32_t elapsed, int32_t duration) = 0;
		virtual bool isFinished(uint32_t elapsed, int32_t duration) = 0;
		virtual uint32_t getRelease() = 0;
};

class FrequencyEnvelope {
	public:
		virtual uint16_t getFrequency(uint16_t baseFrequency, uint32_t elapsed, int32_t duration) = 0;
		virtual bool isFinished(uint32_t elapsed, int32_t duration) = 0;
};

#endif // ENVELOPE_TYPES_H
