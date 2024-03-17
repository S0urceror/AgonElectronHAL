#ifndef AUDIO_CHANNEL_H
#define AUDIO_CHANNEL_H

#include <memory>
#include <atomic>
#include <unordered_map>
#include <fabgl.h>

#include "types.h"
#include "envelopes/types.h"

extern fabgl::SoundGenerator *soundGenerator;	// audio handling sub-system
extern void audioTaskAbortDelay(uint8_t channel);

#define AUDIO_CHANNELS			6		// Default number of audio channels
#define AUDIO_DEFAULT_SAMPLE_RATE	16384	// Default sample rate
#define MAX_AUDIO_CHANNELS		32		// Maximum number of audio channels
#define PLAY_SOUND_PRIORITY		3		// Sound driver task priority with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest

// Audio command definitions
//
#define AUDIO_CMD_PLAY			0		// Play a sound
#define AUDIO_CMD_STATUS		1		// Get the status of a channel
#define AUDIO_CMD_VOLUME		2		// Set the volume of a channel
#define AUDIO_CMD_FREQUENCY		3		// Set the frequency of a channel
#define AUDIO_CMD_WAVEFORM		4		// Set the waveform type for a channel
#define AUDIO_CMD_SAMPLE		5		// Sample management
#define AUDIO_CMD_ENV_VOLUME	6		// Define/set a volume envelope
#define AUDIO_CMD_ENV_FREQUENCY	7		// Define/set a frequency envelope
#define AUDIO_CMD_ENABLE		8		// Enables a channel
#define AUDIO_CMD_DISABLE		9		// Disables (destroys) a channel
#define AUDIO_CMD_RESET			10		// Reset audio channel
#define AUDIO_CMD_SEEK			11		// Seek to a position in a sample
#define AUDIO_CMD_DURATION		12		// Set the duration of a channel
#define AUDIO_CMD_SAMPLERATE	13		// Set the samplerate for channel or underlying audio system
#define AUDIO_CMD_SET_PARAM		14		// Set a waveform parameter

#define AUDIO_SAMPLE_LOAD		0		// Send a sample to the VDP
#define AUDIO_SAMPLE_CLEAR		1		// Clear/delete a sample
#define AUDIO_SAMPLE_FROM_BUFFER				2	// Load a sample from a buffer
#define AUDIO_SAMPLE_SET_FREQUENCY				3	// Set the base frequency of a sample
#define AUDIO_SAMPLE_BUFFER_SET_FREQUENCY		4	// Set the base frequency of a sample (using buffer ID)
#define AUDIO_SAMPLE_SET_REPEAT_START			5	// Set the repeat start point of a sample
#define AUDIO_SAMPLE_BUFFER_SET_REPEAT_START	6	// Set the repeat start point of a sample (using buffer ID)
#define AUDIO_SAMPLE_SET_REPEAT_LENGTH			7	// Set the repeat length of a sample
#define AUDIO_SAMPLE_BUFFER_SET_REPEAT_LENGTH	8	// Set the repeat length of a sample (using buffer ID)
#define AUDIO_SAMPLE_DEBUG_INFO 0x10	// Get debug info about a sample

#define AUDIO_DEFAULT_FREQUENCY	523		// Default sample frequency (C5, or C above middle C)

#define AUDIO_FORMAT_8BIT_SIGNED	0	// 8-bit signed sample
#define AUDIO_FORMAT_8BIT_UNSIGNED	1	// 8-bit unsigned sample
#define AUDIO_FORMAT_DATA_MASK		7	// data bit mask for format
#define AUDIO_FORMAT_WITH_RATE		8	// OR this with the format to indicate a sample rate follows
#define AUDIO_FORMAT_TUNEABLE		16	// OR this with the format to indicate sample can be tuned (frequency adjustable)

#define AUDIO_ENVELOPE_NONE			0		// No envelope
#define AUDIO_ENVELOPE_ADSR			1		// Simple ADSR volume envelope
#define AUDIO_ENVELOPE_MULTIPHASE_ADSR		2		// Multi-phase ADSR envelope

#define AUDIO_FREQUENCY_ENVELOPE_STEPPED	1		// Stepped frequency envelope

#define AUDIO_FREQUENCY_REPEATS		0x01	// Repeat/loop the frequency envelope
#define AUDIO_FREQUENCY_CUMULATIVE	0x02	// Reset frequency envelope when looping
#define AUDIO_FREQUENCY_RESTRICT	0x04	// Restrict frequency envelope to the range 0-65535

#define AUDIO_PARAM_DUTY_CYCLE		0		// Square wave duty cycle
#define AUDIO_PARAM_VOLUME			2		// Volume
#define AUDIO_PARAM_FREQUENCY		3		// Frequency
#define AUDIO_PARAM_16BIT			0x80	// 16-bit value
#define AUDIO_PARAM_MASK			0x0F	// Parameter mask

#define AUDIO_STATUS_ACTIVE		0x01	// Has an active waveform
#define AUDIO_STATUS_PLAYING	0x02	// Playing a note (not in release phase)
#define AUDIO_STATUS_INDEFINITE	0x04	// Indefinite duration sound playing
#define AUDIO_STATUS_HAS_VOLUME_ENVELOPE	0x08	// Channel has a volume envelope set
#define AUDIO_STATUS_HAS_FREQUENCY_ENVELOPE	0x10	// Channel has a frequency envelope set

#define BUFFERED_SAMPLE_BASEID	0xFB00	// Base ID for buffered samples

#define AUDIO_WAVE_DEFAULT			0		// Default waveform (Square wave)
#define AUDIO_WAVE_SQUARE			0		// Square wave
#define AUDIO_WAVE_TRIANGLE			1		// Triangle wave
#define AUDIO_WAVE_SAWTOOTH			2		// Sawtooth wave
#define AUDIO_WAVE_SINE				3		// Sine wave
#define AUDIO_WAVE_NOISE			4		// Noise (simple, no frequency support)
#define AUDIO_WAVE_VICNOISE			5		// VIC-style noise (supports frequency)
#define AUDIO_WAVE_SAMPLE			8		// Sample playback, explicit buffer ID sent in following 2 bytes
#define AUDIO_WAVE_AY_3_8910_NOISE 	6
// negative values for waveforms indicate a sample number

enum AudioState : uint8_t {	// Audio channel state
	Idle = 0,				// currently idle/silent
	Pending,				// note will be played next loop call
	Playing,				// playing (passive)
	PlayLoop,				// active playing loop (used when an envelope is active)
	Release,				// in "release" phase
	Abort					// aborting a note
};

// The audio channel class
//
class AudioChannel {
	public:
		AudioChannel(uint8_t channel);
		~AudioChannel();
		uint8_t		playNote(uint8_t volume, uint16_t frequency, int32_t duration);
		uint8_t		getStatus();
		uint8_t		setWaveform(int8_t waveformType, std::shared_ptr<AudioChannel> channelRef, uint16_t sampleId = 0);
		uint8_t		setVolume(uint8_t volume);
		uint8_t		setFrequency(uint16_t frequency);
		uint8_t		setDuration(int32_t duration);
		uint8_t		setVolumeEnvelope(std::unique_ptr<VolumeEnvelope> envelope);
		uint8_t		setFrequencyEnvelope(std::unique_ptr<FrequencyEnvelope> envelope);
		uint8_t		setSampleRate(uint16_t sampleRate);
		uint8_t		setDutyCycle(uint8_t dutyCycle);
		uint8_t		setParameter(uint8_t parameter, uint16_t value);
		WaveformGenerator * getWaveform() { return this->_waveform.get(); }
		void		attachSoundGenerator();
		void		detachSoundGenerator();
		uint8_t		seekTo(uint32_t position);
		void		loop();
		uint8_t		channel() { return _channel; }
	private:
		std::shared_ptr<WaveformGenerator>	getSampleWaveform(uint16_t sampleId, std::shared_ptr<AudioChannel> channelRef);
		void		waitForAbort();
		uint8_t		getVolume(uint32_t elapsed);
		uint16_t	getFrequency(uint32_t elapsed);
		bool		isReleasing(uint32_t elapsed);
		bool		isFinished(uint32_t elapsed);
		uint8_t		_channel;
		uint8_t		_volume;
		uint16_t	_frequency;
		int32_t		_duration;
		uint32_t	_startTime;
		uint8_t		_waveformType;
		std::atomic<AudioState>				_state;
		std::shared_ptr<WaveformGenerator>	_waveform = nullptr;
		std::unique_ptr<VolumeEnvelope>		_volumeEnvelope;
		std::unique_ptr<FrequencyEnvelope>	_frequencyEnvelope;
};

#endif // AUDIO_CHANNEL_H
