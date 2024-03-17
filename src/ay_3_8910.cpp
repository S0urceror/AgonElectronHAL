#include "ay_3_8910.h"
#include "fabgl.h"
#include "hal.h" 
#include "agon_audio.h"
#include "audio_channel.h"
#include "envelopes/ay_3_8910_envelope.h"

#define PLAY_SOUND_PRIORITY 3

AY_3_8910_Volume ay_3_8910_volume;

AY_3_8910::AY_3_8910 ()
{
    register_select=0;
    toneA=0;
    toneB=0;
    toneC=0;
    noise=0;
    mixer=0;
    env_period=0;
    env_shape=0;
    amplA=0;
    amplB=0;
    amplC=0;
}

void AY_3_8910::init ()
{
    // tone
    setWaveform (0,AUDIO_WAVE_SQUARE,0);
    setWaveform (1,AUDIO_WAVE_SQUARE,0);
    setWaveform (2,AUDIO_WAVE_SQUARE,0);
    // noise
    setWaveform (3,AUDIO_WAVE_AY_3_8910_NOISE,0);
    setWaveform (4,AUDIO_WAVE_AY_3_8910_NOISE,0);
    setWaveform (5,AUDIO_WAVE_AY_3_8910_NOISE,0);
}

void AY_3_8910::updateChannel (uint8_t channel, uint32_t tone_freq,bool volume_envelope,uint8_t volume)
{
    // on
    setFrequency (channel,MASTER_FREQUENCY_DIV / tone_freq);
    if (!volume_envelope)
    {
        // normal volume control, switch off envelopes, if any
        if (channelEnabled(channel))
        {
            if (audioChannels[channel]->getStatus() & AUDIO_STATUS_HAS_VOLUME_ENVELOPE)
                audioChannels[channel]->setVolumeEnvelope (nullptr);
        }
        setVolume (channel,ay_3_8910_volume.getAgonVolume(volume));
    }
    else
    {
        // volume envelope
        std::unique_ptr<AY_3_8910_VolumeEnvelope> envelope = make_unique_psram<AY_3_8910_VolumeEnvelope>(env_shape,env_period);
        if (channelEnabled(channel))
        {
            audioChannels[channel]->setVolumeEnvelope (std::move (envelope));
            // base volume
            setVolume (channel,ay_3_8910_volume.getAgonVolume(15));
        }
    }
}

void AY_3_8910::updateSound (uint8_t channel, uint8_t mixer, uint8_t amp, uint32_t tone_freq, uint32_t noise_freq)
{
    bool tone = (mixer & (0b00000001<<channel))==0;
    bool noise = (mixer & (0b00001000<<channel))==0;
    bool volume_envelope = (amp & 0x10) != 0;
    uint8_t volume = amp & 0x0f;

    if (tone && tone_freq > 0)
    {
        updateChannel (channel,tone_freq,volume_envelope,volume);
    } 
    else
    {
        // off
        if (!volume_envelope)
            setVolume (channel,0);
        if (audioChannels[channel]->getStatus() & AUDIO_STATUS_HAS_VOLUME_ENVELOPE)
                audioChannels[channel]->setVolumeEnvelope (nullptr);
    }
    // noise channel + 3
    if (noise && noise_freq > 0)
    {
        updateChannel (channel+3,noise_freq,volume_envelope,volume);
    } 
    else
    {
        // off
        if (!volume_envelope)
            setVolume (channel+3,0);
        if (audioChannels[channel+3]->getStatus() & AUDIO_STATUS_HAS_VOLUME_ENVELOPE)
            audioChannels[channel+3]->setVolumeEnvelope (nullptr);
    }
}
void AY_3_8910::write (uint8_t port, uint8_t value)
{
    bool updateA=false,updateB=false,updateC = false;
    uint8_t mixer_diff;

    if (port==0xa0)
        register_select = value;
    if (port==0xa1)
    {
        switch (register_select)
        {
            // Tone A - fine
            case 0x00:
                toneA = toneA & 0b111100000000;
                toneA = toneA + value;
                updateA = true;
                break;
            // Tone A - coarse
            case 0x01:
                toneA = toneA & 0b000011111111;
                toneA = toneA + (value << 8);
                updateA = true;
                break;
            // Tone B - fine
            case 0x02:
                toneB = toneB & 0b111100000000;
                toneB = toneB + value;
                updateB = true;
                break;
            // Tone B - coarse
            case 0x03:
                toneB = toneB & 0b000011111111;
                toneB = toneB + (value << 8);
                updateB = true;
                break;
            // Tone C - fine
            case 0x04:
                toneC = toneC & 0b111100000000;
                toneC = toneC + value;
                updateC = true;
                break;
            // Tone C - coarse
            case 0x05:
                toneC = toneC & 0b000011111111;
                toneC = toneC + (value << 8);
                updateC = true;
                break;
            // Noise generator
            case 0x06:
                noise = value & 0b00011111;
                // noise frequency change applies to all channels
                updateA = true;
                updateB = true;
                updateC = true;
                break;
            // Voice, I/O port control
            case 0x07:
                mixer_diff = mixer ^ value;
                mixer = value;
                if (mixer_diff&0b00001001!=0)
                    updateA = true;
                if (mixer_diff&0b00010010!=0)
                    updateB = true;
                if (mixer_diff&0b00100100!=0)
                    updateC = true;
                break;
            // Amplitude, volume control - A
            case 0x08:
                amplA = value & 0b00011111;
                updateA = true;
                break;
            // Amplitude, volume control - B
            case 0x09:
                amplB = value & 0b00011111;
                updateB = true; 
                break;
            // Amplitude, volume control - C
            case 0x0a:
                amplC = value & 0b00011111;
                updateC = true;
                break;
            // Envelope period - fine
            case 0x0b:
                env_period = env_period & 0b1111111100000000;
                env_period = env_period + value;
                break;
            // Envelope period - coarse
            case 0x0c:
                env_period = env_period & 0b0000000011111111;
                env_period = env_period + (value << 8);
                break;
            // Envelope shape
            case 0x0d:
                env_shape = value & 0b00001111;
                if (amplA&0b10000)
                    updateA = true;
                if (amplB&0b10000)
                    updateC = true;
                if (amplC&0b10000)
                    updateC = true;
                break;
            // gpio read/write
            case 0x0e:
                gpio = value;
                break;
            // gpio control
            case 0x0f:
                gpio_control = value;
                break;
            default:
                break;
        }
    }

    // update sound
    if (updateA) 
    {
        updateSound (0,mixer,amplA,toneA,noise);
        updateA = false;
    }
    if (updateB) 
    {
        updateSound (1,mixer,amplB,toneB,noise);
        updateB = false;
    }
    if (updateC) 
    {
        updateSound (2,mixer,amplC,toneC,noise);
        updateC = false;
    }
}

uint8_t AY_3_8910::read (uint8_t port)
{
    uint8_t val=0;

    if (port==0xa2)
    {
        switch (register_select)
        {
            // gpio read/write
            case 0x0e:
                if (gpio_control & 0b01000000)
                    // joystick portB
                    val = 0x3f; // no things pressed on the joystick (yet), all signals 1-High
                else
                    // joystick portA
                    val = 0x3f; // no things pressed on the joystick (yet), all signals 1-High
                break;
            // gpio control
            case 0x0f:
                val = gpio_control;
                break;
        }
    }

    return val;
}