#include "ay_3_8910.h"
#include "envelopes/ay_3_8910_envelope.h"

extern AY_3_8910_Volume ay_3_8910_volume;
// switch (env_shape)
// {
//     case 0b0000:
//     case 0b0001:
//     case 0b0010:
//     case 0b0011:
//     case 0b1001:
//         attack = 0;
//         decay = (MASTER_FREQUENCY / (16*env_period)) * 1000;
//         sustain = 0;
//         release = 0;
//         // waveform: ＼＿＿＿＿＿＿＿
//         break;
//     case 4:
//     case 5:
//     case 6:
//     case 7:
//     case 15:
//         // waveform: ／＿＿＿＿＿＿＿
//         attack = (MASTER_FREQUENCY / (16*env_period)) * 1000;
//         decay = 0;
//         sustain = 0;
//         release = 0;
//         break;
//     case 8:
//         // waveform: ＼＼＼＼＼＼＼＼
//         attack = 0;
//         decay = (MASTER_FREQUENCY / (16*env_period)) * 1000;
//         sustain = 0;
//         release = 0;
//         break;
//     case 10:
//         // waveform: ＼／＼／＼／＼／
//         break;
//     case 11:
//         // waveform: ＼￣￣￣￣￣￣￣
//         attack = 0;
//         decay = (MASTER_FREQUENCY / (16*env_period)) * 1000;
//         sustain = 0;
//         release = 0;
//         break;
//     case 12:
//         // waveform: ／／／／／／／／
//         attack = (MASTER_FREQUENCY / (16*env_period)) * 1000;
//         decay = 0;
//         sustain = 0;
//         release = 0;
//         break;
//     case 13:
//         // waveform: ／￣￣￣￣￣￣￣
//         attack = (MASTER_FREQUENCY / (16*env_period)) * 1000;
//         decay = 0;
//         sustain = 255;
//         release = 0;
//         end_after_period = false;
//         break;
//     case 14:
//         // waveform: ／＼／＼／＼／＼
//         break;
// }

AY_3_8910_VolumeEnvelope::AY_3_8910_VolumeEnvelope(uint8_t envelope,uint16_t period)
{
	_continue = envelope & 0b1000;
    _attack = envelope & 0b0100;
    _alternate = envelope & 0b0010;
    _hold = envelope & 0b0001;
	_period = 256.0 * float(period) / float (MASTER_FREQUENCY) * 1000.0;
	//debug_log("audioDriver: AY_3_8910_VolumeEnvelope: continue=%d, attack=%d, alternate=%d, hold=%d, period=%d\n\r", _continue,_attack,_alternate,_hold,_period);
}

uint8_t AY_3_8910_VolumeEnvelope::getVolume(uint8_t baseVolume, uint32_t elapsed, int32_t duration) {
	uint8_t volume=0;
	float factor;
	uint8_t period_cnt=0;
	// returns volume for the given elapsed time
	// baseVolume is the level the attack phase should reach
	// volume for fab-gl is 0-127 but accepts higher values, so we're not clamping
	// a duration of -1 means we're playing forever
	if (elapsed<_period)
	{
		factor = ((float) elapsed)/((float) _period);
		if (_attack)
			volume = 15*factor;				// waveform: /
		else
			volume = 15-15*factor;	// waveform: ＼

		_hold_volume = volume;
	}
	else
	{
		// signal releasing
		if (_continue)
		{
			// continue emitting sound, never finished
			if (_hold)
			{
				volume = _hold_volume;				// waveform: _______ or ￣￣￣￣￣￣￣
				if (_alternate)
					volume = 15-volume; // flip
			}
			else
			{
				// calculations
				period_cnt = elapsed / _period;
				elapsed = elapsed % _period;
				factor = ((float) elapsed)/((float) _period);

				// normal waveform
				if (_attack)
					volume = 15*factor;				// waveform: /
				else
					volume = 15-15*factor;	// waveform: ＼

				// alternate
				if (period_cnt%2 == 1 && _alternate)
					volume = 15-volume;
			}
		}
		else
			// stop emitting sound, signal finished
			volume = 0;
	}
	return ay_3_8910_volume.getAgonVolume(volume);
}

bool AY_3_8910_VolumeEnvelope::isReleasing(uint32_t elapsed, int32_t duration) {
	if (elapsed<_period)
		return false;
	else
		return true;
}

bool AY_3_8910_VolumeEnvelope::isFinished(uint32_t elapsed, int32_t duration) {
	if (elapsed<_period || _continue)
		return false;
	else
		return true;
}