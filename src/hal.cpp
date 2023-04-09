/*
 * Title:           Electron - HAL
 *                  a playful alternative to Quark
 *                  quarks and electrons combined are matter.
 * Author:          Mario Smit (S0urceror)
*/

// Hardware Abstraction Layer

#include "hal.h"

HardwareSerial host_serial(0);
fabgl::Terminal* fabgl_terminal;

// Set the RTS line value
//
void setRTSStatus(bool value) {
	digitalWrite(UART_RTS, value ? LOW : HIGH);		// Asserts when LOW
}

bool getCTSStatus ()
{
    // TODO: full hw handshake support
    return true;
    return digitalRead (UART_CTS)&0b1;
}

void hal_hostpc_serial (fabgl::Terminal* term)
{   
    fabgl_terminal = term;
	host_serial.begin(115200, SERIAL_8N1, 3, 1);
}
// print both on the Terminal and the connecting Host via serial (if any)
void hal_printf (const char* format, ...) {
	va_list ap;
   	va_start(ap, format);
   	int size = vsnprintf(nullptr, 0, format, ap) + 1;
   	if (size > 0) {
    	va_end(ap);
     	va_start(ap, format);
     	char buf[size + 1];
     	vsnprintf(buf, size, format, ap);
     	host_serial.print(buf);
        fabgl_terminal->write (buf);
   	}
   	va_end(ap);
}
char hal_hostpc_serial_read ()
{
	if (host_serial.available()) {
		return host_serial.read();
	}
	return 0;
}