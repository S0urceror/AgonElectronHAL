#ifndef __PPI_8255_H
#define __PPI_8255_H

#include <stdint.h>
#include <vector>
#include <fabutils.h>

class PPI8255
{
    protected:
        uint8_t portA,portB,portC,control;
        uint8_t current_row;
        
    public:
        PPI8255 ();
        virtual void write (uint8_t port, uint8_t value);
        virtual uint8_t read (uint8_t port)=0;
        virtual uint8_t record_keypress (uint8_t ascii,uint8_t modifier,uint8_t vk,uint8_t down)=0;
        virtual uint8_t get_row_bits (uint8_t row)=0;
        virtual uint8_t get_next_row ()=0;
};

class SG1000_PPI8255 : public PPI8255
{
    private:
        uint16_t rows[8];
    public:
        SG1000_PPI8255 ();
        uint8_t read (uint8_t port);
        uint8_t record_keypress (uint8_t ascii,uint8_t modifier,uint8_t vk,uint8_t down);
        uint8_t get_row_bits (uint8_t row);
        uint8_t get_next_row ();
};

typedef struct
{
    uint8_t vk;
    uint8_t modifier;
} ascii_to_vk_modifier;

typedef struct  
{
    uint8_t msx_matrix_row;
    uint8_t msx_matrix_mask;
} vk_to_msx_matrix;

class MSX_PPI8255 : public PPI8255
{
    private:
        uint8_t rows[11];
        std::vector <uint8_t> vk_key_up;
    public:
        MSX_PPI8255 ();
        uint8_t read (uint8_t port);
        uint8_t record_keypress (uint8_t ascii,uint8_t modifier,uint8_t vk,uint8_t down);
        uint8_t get_row_bits (uint8_t row);
        uint8_t get_next_row ();
};

#endif // __PPI_8255_H