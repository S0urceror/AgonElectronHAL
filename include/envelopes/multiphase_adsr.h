//
// Title:			Multi-phase ADSR Envelope support
// Author:			Steve Sims
// Created:			14/01/2024
// Last Updated:	14/01/2024

#ifndef ENVELOPE_MULTIPHASE_ADSR_H
#define ENVELOPE_MULTIPHASE_ADSR_H

#include <memory>
#include <vector>
#include <Arduino.h>

#include "./types.h"

struct VolumeSubPhase {
	uint8_t level;			// relative volume level for sub-phase
	uint16_t duration;		// number of steps
};

class MultiphaseADSREnvelope : public VolumeEnvelope {
	public:
		MultiphaseADSREnvelope(std::shared_ptr<std::vector<VolumeSubPhase>> attack, std::shared_ptr<std::vector<VolumeSubPhase>> sustain, std::shared_ptr<std::vector<VolumeSubPhase>> release);
		uint8_t		getVolume(uint8_t baseVolume, uint32_t elapsed, int32_t duration);
		bool		isReleasing(uint32_t elapsed, int32_t duration);
		bool 		isFinished(uint32_t elapsed, int32_t duration);
		uint32_t	getRelease() {
			return _releaseDuration;
		};
	private:
		uint8_t		getTargetVolume(uint8_t baseVolume, uint8_t level);
		std::shared_ptr<std::vector<VolumeSubPhase>> _attack;
		std::shared_ptr<std::vector<VolumeSubPhase>> _sustain;
		std::shared_ptr<std::vector<VolumeSubPhase>> _release;
		uint32_t	_attackDuration;
		uint32_t	_sustainDuration;
		uint32_t	_releaseDuration;
		uint8_t		_sustainSubphases;
		uint8_t		_attackLevel;			// final levels
		uint8_t		_sustainLevel;
		uint8_t		_releaseLevel;
		bool		_sustainLoops;
};


#endif // ENVELOPE_MULTIPHASE_ADSR_H
