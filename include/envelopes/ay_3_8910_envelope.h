//
// Title:			AY_3_8910_ENVELOPE_H Envelope support
// Author:			S0urceror
// Created:			06/08/2023
// Last Updated:	14/01/2024

#ifndef ENVELOPE_AY_3_8910_ENVELOPE_H
#define ENVELOPE_AY_3_8910_ENVELOPE_H

#include "./types.h"

class AY_3_8910_VolumeEnvelope : public VolumeEnvelope {
	public:
		AY_3_8910_VolumeEnvelope(uint8_t envelope,uint16_t period);
		uint8_t getVolume(uint8_t baseVolume, uint32_t elapsed, int32_t duration);
		bool isReleasing(uint32_t elapsed, int32_t duration);
		bool isFinished(uint32_t elapsed, int32_t duration);
		uint32_t getRelease() {
			return 0;
		}
	private:
		uint32_t _period; // in msecs
		uint8_t _hold_volume;

		bool _continue;
		bool _attack;
		bool _alternate;
		bool _hold;
};


#endif // ENVELOPE_AY_3_8910_ENVELOPE_H
