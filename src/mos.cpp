
/*
 * Title:           Electron - HAL
 *                  a playful alternative to Quark
 *                  quarks and electrons combined are matter.
 * Author:          Mario Smit (S0urceror)
*/

// MOS interoperability layer (not complete)

#include "mos.h"

uint8_t col,row;

void mos_init ()
{
    col=1;
    row=1;

    // send ESC to EZ80
    ez80_serial.write(27);
}

// Send a packet of data to the MOS
//
void mos_send_packet(byte code, byte len, byte data[]) {
    // wait for CTS
    while (!getCTSStatus());

    ez80_serial.write(code + 0x80);
    ez80_serial.write(len);
    for(int i = 0; i < len; i++) {
        ez80_serial.write(data[i]);
    }
}

void mos_send_general_poll ()
{
    byte uv = ez80_serial.read();
    byte packet[] = {
        uv
    };
    mos_send_packet(PACKET_GP, sizeof packet, packet);
}

void mos_send_vdp_mode ()
{
    byte packet[] = {
        0x80,	 						// Width in pixels (L)
        0x02,       					// Width in pixels (H)
        0xe0,							// Height in pixels (L)
        0x01,       					// Height in pixels (H)
        80 ,	                        // Width in characters (byte)
        60 ,	                        // Height in characters (byte)
        64,       						// Colour depth
    };
    mos_send_packet(PACKET_MODE, sizeof packet, packet);
}
void mos_send_cursor_pos ()
{
    byte packet[] = {
        (byte) (col),
        (byte) (row),
    };
    //hal_printf ("x:%d,y:%d",x,y);
    mos_send_packet(PACKET_CURSOR, sizeof packet, packet);	
}

void mos_col_left ()
{
    if (col>1)
        col--;
}
void mos_col_right ()
{
    col++;
}
void mos_set_column (uint8_t c)
{
    col = c;
}