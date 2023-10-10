#include "fabgl.h"
#include "tms9918.h"
#include "hal.h"
#include "globals.h"

TMS9918::TMS9918 ()
{
    status_register = 0;
    mode = 1;
    read_address = 0;
    write_address = 0;
    mem16kb = true;
    port99_write = false;
    data_register = 0;
    screen_enable = true;
    interrupt_enable = true;
    sprite_size_16 = false;
    sprite_magnify = false;
    nametable = 0;
    colortable = 0;
    patterntable = 0;
    sprite_attribute_table = 0;
    sprite_pattern_table = 0;
    textmode_colors = 0;
    display = nullptr;

    memset (memory,0,sizeof(memory));
}
void TMS9918::init_mode ()
{
    switch (mode) 
    {
        case 0b000 :
            //hal_printf ("Graphics I Mode\r\n");
            break;
        case 0b100 :
            //hal_printf ("Graphics II Mode\r\n");
            set_display_direct ();
            break;
        case 0b010 :
            //hal_printf ("Multicolor Mode\r\n");
            break;
        case 0b001 :
            //hal_printf ("Text Mode\r\n");
            break;
        default:
            break;
    }
}
void TMS9918::write_register (uint8_t reg, uint8_t value)
{
    uint8_t newmode;
    uint8_t oldmode;

    // store raw info
    registers[reg] = value;
    // extract info
    switch (reg)
    {
        case 0:
            newmode = data_register & 0b00001110;
            newmode = newmode << 1;
            //oldmode = mode;
            // clear all upper mode bits
            mode = mode & 0b00000011;
            // set new upper mode bits
            mode = mode | newmode;
            // have we selected a new mode?
            //if (oldmode!=mode) 
            //    init_mode ();
            break;
        case 1:
            // handle the mode changes
            //////////////////////////
            newmode = data_register & 0b00011000;
            newmode = newmode >> 3;
            newmode = ((newmode & 1) << 1) + ((newmode & 2) >> 1); // rotate bits
            oldmode = mode;
            // clear two lower mode bits
            mode = mode & 0b00011100;
            // set new lower mode bits
            mode = mode | newmode;
            // have we selected a new mode?
            if (oldmode!=mode) 
                init_mode ();
            // handle the other bits
            ////////////////////////
            mem16kb =           (data_register & 0b10000000) > 0;
            screen_enable =     (data_register & 0b01000000) > 0;
            interrupt_enable =  (data_register & 0b00100000) > 0;
            sprite_size_16 =    (data_register & 0b00000010) > 0;
            sprite_magnify =    (data_register & 0b00000001) > 0;
            
            //if (mem16kb)            hal_printf("16kB VRAM\r\n");
            //if (screen_enable)      hal_printf("screen enabled\r\n");
            //if (interrupt_enable)   hal_printf("VDP interrupts enabled\r\n");
            //if (sprite_size_16)     hal_printf("16x16 sprites\r\n");
            //else                    hal_printf("8x8 sprites\r\n");
            //if (sprite_magnify)     hal_printf("magnified sprites\r\n");
            break;
        case 2:
            nametable = data_register;
            nametable = nametable << 10;
            nametable = nametable & 0x3fff;
            //hal_printf("nametable: %04X\r\n",nametable);
            break;
        case 3:
            colortable = data_register;
            colortable = colortable << 6;
            colortable = colortable & 0x3fff;
            //hal_printf("colortable: %04X\r\n",colortable);
            break;
        case 4:
            patterntable = data_register;
            patterntable = patterntable << 11;
            patterntable = patterntable & 0x3fff;
            //hal_printf("patterntable: %04X\r\n",patterntable);
            break;
        case 5:
            sprite_attribute_table = data_register;
            sprite_attribute_table = sprite_attribute_table << 7;
            sprite_attribute_table = sprite_attribute_table & 0x3fff;
            //hal_printf("sprite_attribute_table: %04X\r\n",sprite_attribute_table);
            break;
        case 6:
            sprite_pattern_table = data_register;
            sprite_pattern_table = sprite_pattern_table << 11;
            sprite_pattern_table = sprite_pattern_table & 0x3fff;
            //hal_printf("sprite_pattern_table: %04X\r\n",sprite_pattern_table);
            break;
        case 7:
            textmode_colors = data_register;
            //hal_printf("textmode colors: %02X\r\n",textmode_colors);
            break;
        default:
            break;
    }              
}
void TMS9918::write (uint8_t port, uint8_t value)
{
    if (port==0x98) 
    {
        //hal_printf ("writing %02X to %04X\r\n",value, write_address);
        memory[write_address]=value;
        write_address = (write_address + 1) & 0x3fff;
    }
    if (port==0x99) 
    {
        if (port99_write == false)
        {
            port99_write = true;
            data_register = value;
        } 
        else 
        {
            // reset port99_write
            port99_write = false;
            // write register
            if ((value & 0b11000000) == 0b10000000)
                write_register (value & 0b00000111,data_register);
            // vram write address
            if ((value & 0b11000000) == 0b01000000)
            {
                uint16_t temp = value & 0b00111111;
                temp = temp << 8;
                write_address = temp + data_register;
            }
            // vram read address
            if ((value & 0b11000000) == 0b00000000)
            {
                uint16_t temp = value & 0b00111111;
                temp = temp << 8;
                read_address = temp + data_register;
            }
        }
    }
}
uint8_t TMS9918::read (uint8_t port)
{
    uint8_t val=0;
    if (port==0x98) 
    {
        val = memory[read_address];
        read_address = (read_address + 1) & 0x3fff;
        //hal_printf ("read %02X from %04X\r\n",val, read_address);
    }
    if (port==0x99) 
    {
        val = status_register;
    }
    return val;
}

void TMS9918::set_display (fabgl::VGADirectController* dsply)
{
    display = dsply;
}

void TMS9918::draw_screen0 (uint8_t* dest,int scanline)
{
}
void TMS9918::draw_screen1 (uint8_t* dest,int scanline)
{
}
void TMS9918::draw_screen2 (uint8_t* dest,int scanline)
{
    uint16_t nametable_idx;
    uint8_t  pattern;
    uint16_t pattern_idx;
    uint8_t  pixel_pattern;
    uint16_t color_idx;
    uint8_t  color_byte;
    uint8_t  fgcolor;
    uint8_t  bgcolor;
    uint16_t posX,posY;
    uint8_t  character;
    uint8_t  pixel;

    uint16_t sprite_attributes;
    uint8_t sprite_Y;
    uint8_t sprite_X;
    uint8_t sprite_pattern_nr;
    uint8_t sprite_atts;
    uint8_t sprite_color_idx;
    fabgl::RGB222 sprite_color;
    uint8_t sprite_pixels_size;
    uint16_t sprite_pattern_address;

    // background color that shines through color 0
    bgcolor = display->createRawPixel(colors[textmode_colors & 0x0f]); // lower nibble;
    memset(dest, bgcolor, screen_width);

    if (!screen_enable)
        return;

    // we're drawing 320x192
    if (scanline<screen_border_vert || scanline >= screen_height - screen_border_vert)
        return;
    posY = scanline - screen_border_vert;

    // in graphics II only
    uint16_t _patterntable = 0x0000;
    uint16_t _colortable = 0x0000;
    if (patterntable & 0b0010000000000000) {
        _patterntable = 0x2000;
    }
    _patterntable += 0x800 * uint16_t (posY/64);
    if (colortable & 0b0010000000000000) {
        _colortable = 0x2000;
    }
    _colortable += 0x800 * uint16_t (posY/64);

    // draw 32 chars * 8 pixels for the posY specified
    //
    for (character=0;character<32;character++)
    {
        // get pattern and color idx
        nametable_idx = ((posY / 8) * 32) + character;
        pattern = memory [nametable + nametable_idx];
        pattern_idx = pattern * 8 + (posY % 8);
        color_idx = pattern * 8 + (posY % 8);
        // get pixel pattern
        pixel_pattern = memory[_patterntable + pattern_idx];
        // get colors
        color_byte = memory[_colortable + color_idx];
        // get RGB222 values of MSX palet
        fgcolor = display->createRawPixel(colors[color_byte >> 4]);  // upper nibble
        bgcolor = display->createRawPixel(colors[color_byte & 0x0f]);// lower nibble
        // calculate position of character on the posY
        posX = screen_border_horz + character * 8; // shift 32 pixels to the right to make 256 pixels center on our 320 width
        for (pixel=0;pixel<8;pixel++)
        {
            // check every bit in the pixel_pattern
            if (pixel_pattern & (1<<(7-pixel)))
            {
                VGA_PIXELINROW(dest, posX+pixel) = fgcolor;
            }
            else
            {
                if ((color_byte & 0x0f) != 0)
                    // when not transparent have a pixel with the bgcolor
                    VGA_PIXELINROW(dest, posX+pixel) = bgcolor;
            }
        }
    }

    // sprites
    for (int sprite=0;sprite<32;sprite++)
    {
        // get sprite attributes
        sprite_attributes = sprite_attribute_table + sprite * 4;
        sprite_Y =          memory[sprite_attributes+0];
        sprite_X =          memory[sprite_attributes+1];
        sprite_pattern_nr = memory[sprite_attributes+2];
        sprite_atts =       memory[sprite_attributes+3];

        // do we need to continue with lower priority sprites?
        if (sprite_Y == 208)
            break; // last sprite in the series
        
        // sprite is actually 1 pixel lower
        sprite_Y++;

        // get derived values
        sprite_color_idx =  sprite_atts & 0xf;
        sprite_color =      colors[sprite_color_idx];
        fgcolor = display->createRawPixel(sprite_color); 
        sprite_pixels_size= sprite_size_16?16:8;

        // resize sprite when magnified
        if (sprite_magnify)
            sprite_pixels_size = sprite_pixels_size * 2;
        
        // is sprite visible on this scanline?
        if (posY < sprite_Y || 
            posY >= sprite_Y+sprite_pixels_size)
            continue; // not visible

        // is sprite transparent colour?
        if (sprite_color_idx == 0)
            continue; // not visible
        
        // draw sprite pixels
        //
        // two segments of 8 pixels when we have a 16x16 sprite, or one segment of  pixels when 8x8
        for (uint8_t hor_sprite_segment = 0; hor_sprite_segment<(sprite_size_16?2:1); hor_sprite_segment++)
        {
            // get start address of pattern in memory
            sprite_pattern_address = sprite_pattern_table + (sprite_pattern_nr * 8) + (posY - sprite_Y) + (16 * hor_sprite_segment);
            // get pixel pattern
            pixel_pattern = memory[sprite_pattern_address];
            // calculate position of sprite on the posY
            posX = screen_border_horz + sprite_X + hor_sprite_segment*(sprite_pixels_size/2) + 1; 
            for (pixel=0;pixel<8;pixel++)
            {
                if (posX+pixel < screen_width-screen_border_horz)
                {
                    // check every bit in the pixel_pattern
                    if (pixel_pattern & (1<<(7-pixel)))
                        VGA_PIXELINROW(dest, posX+pixel) = fgcolor;
                }
            }
        }
    }
}
void TMS9918::draw_screen3 (uint8_t* dest,int scanline)
{
}

void TMS9918::draw_screen (uint8_t* dest,int scanline)
{
    // not set up yet
    if (display==nullptr)
        return;

    switch (mode) 
    {
        case 0b001 :
            draw_screen0 (dest,scanline);       // Text Mode
            break;
        case 0b000 :
            draw_screen1 (dest,scanline);       // Graphics I Mode
            break;
        case 0b100 :
            draw_screen2 (dest,scanline);       // Graphics II Mode
            break;
        case 0b010 :
            draw_screen3 (dest,scanline);       // Multicolor Mode
            break;
    }
}