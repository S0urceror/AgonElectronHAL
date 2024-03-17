//
// Title:			Audio ADSR Volume Envelope support
// Author:			Steve Sims
// Created:			06/08/2023
// Last Updated:	14/01/2024

#ifndef ENVELOPE_ADSR_H
#define ENVELOPE_ADSR_H

#include "./types.h"

class ADSRVolumeEnvelope : public VolumeEnvelope {
	public:
		ADSRVolumeEnvelope(uint16_t attack, uint16_t decay, uint8_t sustain, uint16_t release);
		uint8_t getVolume(uint8_t baseVolume, uint32_t elapsed, int32_t duration);
		bool isReleasing(uint32_t elapsed, int32_t duration);
		bool isFinished(uint32_t elapsed, int32_t duration);
		uint32_t getRelease() {
			return this->_release;
		}
	private:
		uint16_t _attack;
		uint16_t _decay;
		uint8_t _sustain;
		uint16_t _release;
};


#endif // ENVELOPE_ADSR_H
