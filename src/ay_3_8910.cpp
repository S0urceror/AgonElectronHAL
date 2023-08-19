#include "ay_3_8910.h"

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
void AY_3_8910::write (uint8_t port, uint8_t value)
{
    bool updateA,updateB,updateC = false;

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
                break;
            // Voice, I/O port control
            case 0x07:
                mixer = value;
                updateA = true;
                updateB = true;
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
                break;
            default:
                break;
        }
    }

    // update sound
    if (updateA && toneA > 0) 
    {
        uint32_t freq = 1789772 / 16;
        freq = freq / toneA;
        uint8_t amp = amplA;
        if (mixer & 0b00000001 > 0)
            amp = 0; // muted
        //self.audio_channels.start_tone(0,0,amp*16,freq as i16,500); // 0.5 second
        updateA = false;
    }
    if (updateB && toneB > 0) 
    {
        uint32_t freq = 1789772 / 16;
        freq = freq / toneB;
        uint8_t amp = amplB;
        if (mixer & 0b00000010 > 0)
            amp = 0; // muted
        //self.audio_channels.start_tone(0,0,amp*16,freq as i16,500); // 0.5 second
        updateB = false;
    }
    if (updateC && toneC > 0) 
    {
        uint32_t freq = 1789772 / 16;
        freq = freq / toneC;
        uint8_t amp = amplC;
        if (mixer & 0b00000100 > 0)
            amp = 0; // muted
        //self.audio_channels.start_tone(0,0,amp*16,freq as i16,500); // 0.5 second
        updateC = false;
    }
}
uint8_t AY_3_8910::read (uint8_t port)
{
    uint8_t val=0;

    if (port==0xa2)
        val = 0x3f; // no things pressed on the joystick (yet), all signals 1-High

    return val;
}