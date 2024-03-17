#ifndef __AY_3_8910_H_
#define __AY_3_8910_H_

#include <stdint.h>
#include <math.h>

#define MASTER_FREQUENCY 1789772
#define MASTER_FREQUENCY_DIV (MASTER_FREQUENCY/16)
#define FABGL_MAX_AMPLITUDE 127
#define FABGL_AMPLITUDE_MULTIPLIER (128/16)

class AY_3_8910_Volume
{
    private:
        uint8_t volumeTab [16];
    public:
        
        AY_3_8910_Volume ()
        {
            // initialize volumeTab
            //
            // Calculate the volume->voltage conversion table. The AY-3-8910 has 16 levels,
            // in a logarithmic scale (3dB per step). YM2149 has 32 levels, the 16 extra
            // levels are only used for envelope volumes
            // 1/sqrt(sqrt(2)) ~= 1/(1.5dB)
            for (int i=0;i<16;i++)
                volumeTab[i] = 127.0 / pow(sqrt(2),15-i);
        }
        uint8_t getAgonVolume (uint8_t ay_3_8190_volume)
        {
            return volumeTab[ay_3_8190_volume];
        }
};

class AY_3_8910
{
    public:
        AY_3_8910 ();

        void init ();
        void write (uint8_t port, uint8_t value);
        uint8_t read (uint8_t port);

    private:
        uint8_t     register_select;
        uint16_t    toneA;
        uint16_t    toneB;
        uint16_t    toneC;
        uint8_t     noise;
        uint8_t     mixer;
        uint16_t    env_period;
        uint8_t     env_shape;
        uint8_t     amplA;
        uint8_t     amplB;
        uint8_t     amplC;
        uint8_t     gpio;
        uint8_t     gpio_control;

        void updateSound (uint8_t channel, uint8_t mixer, uint8_t amp, uint32_t tone_freq, uint32_t noise_freq);
        void updateChannel (uint8_t channel, uint32_t tone_freq,bool volume_envelope,uint8_t volume);
};

#endif // __AY_3_8910_H_