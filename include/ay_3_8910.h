#ifndef __AY_3_8910_H_
#define __AY_3_8910_H_

#include <stdint.h>

class AY_3_8910
{
    public:
        AY_3_8910 ();

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
};

#endif // __AY_3_8910_H_