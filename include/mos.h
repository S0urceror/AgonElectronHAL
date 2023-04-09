#ifndef __MOS_H_
#define __MOS_H_

#include "hal.h"
extern fabgl::TerminalController termctrl;

// MOS compatibility layer
void mos_send_packet(byte code, byte len, byte data[]);
void mos_send_vdp_mode ();
void mos_send_cursor_pos ();

#endif