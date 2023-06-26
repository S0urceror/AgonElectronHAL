#ifndef __HAL_H_
#define __HAL_H_

#include "Arduino.h"
#include "HardwareSerial.h"
#include "fabgl.h"

#define ez80_serial Serial2

#define UART_BR         1152000
#define UART_NA         -1
#define UART_TX         2
#define UART_RX         34
#define UART_RTS        13 // The ESP32 RTS pin
#define UART_CTS        14 // The ESP32 CTS pin
#define UART_RX_THRESH	64 // Point at which RTS is toggled

#define GPIO_ITRP       17 // VSync Interrupt Pin - for reference only

#ifdef MOS_COMPATIBILITY
const byte PACKET_KEYCODE	= 0x01;	// Keyboard data
const byte PACKET_SCRCHAR   = 0x03;
const byte PACKET_MODE      = 0x06;
const byte PACKET_CURSOR    = 0x02;
const byte PACKET_GP        = 0x00; // general poll data
#endif

extern HardwareSerial hal_serial;
extern fabgl::Terminal* fabgl_terminal;

void hal_printf (const char* format, ...);
void hal_hostpc_serial_init (fabgl::Terminal*);
char hal_hostpc_serial_read ();
void hal_ez80_serial_init ();

#define	HAL_major		0
#define	HAL_minor		1
#define	HAL_revision 	0


#endif