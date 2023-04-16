#ifndef __MOS_H_
#define __MOS_H_

#include "hal.h"

// MOS compatibility layer
void mos_init ();
void mos_send_packet(byte code, byte len, byte data[]);
void mos_send_general_poll ();
void mos_send_vdp_mode ();
void mos_send_cursor_pos ();
void mos_col_left ();
void mos_col_right ();
void mos_set_column (uint8_t col);

#endif