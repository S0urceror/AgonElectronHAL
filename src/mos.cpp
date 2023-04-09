
#include "mos.h"

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
    int x,y;
    termctrl.getCursorPos (&x,&y);
    byte packet[] = {
        (byte) (x-2),
        (byte) (y-1),
    };
    //hal_printf ("x:%d,y:%d",x,y);
    mos_send_packet(PACKET_CURSOR, sizeof packet, packet);	
}