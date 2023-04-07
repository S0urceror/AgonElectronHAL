/*
 * Title:           Electron - HAL
 *                  a playful alternative to Quark
 *                  quarks and electrons combined are matter.
 * Author:          Mario Smit (S0urceror)
 * Credits:         Dean Belfield
 *                  Koenraad Van Nieuwenhove (Cocoacrumbs)
 * Created:         01/03/2023
 */

#include "fabgl.h"
#include "hal.h"
#include "zdi.h"

#define SERIALKB 1

fabgl::PS2Controller    PS2Controller;
fabgl::VGA16Controller  DisplayController;
fabgl::Terminal         Terminal;
bool                    ZDIMode=false;

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

// Send a packet of data to the MOS
//
void send_packet(byte code, byte len, byte data[]) {
    // wait for CTS
    while (!getCTSStatus());

    ez80_serial.write(code + 0x80);
    ez80_serial.write(len);
    for(int i = 0; i < len; i++) {
        ez80_serial.write(data[i]);
    }
}

// Handle the keyboard
void do_keyboard()
{
    byte ch;
	if((ch=hal_hostpc_serial_read())>0) 
    {
		byte packet[] = {
			ch,
			0,
		};
        if (!zdi_mode() && ch==0x1a) 
        {
            zdi_enter();
        }
        else
        {
            if (zdi_mode())
                zdi_process_cmd (ch);
            else
		        send_packet(PACKET_KEYCODE, sizeof packet, packet);
        }
	} 
    else
    {
        fabgl::Keyboard *kb = PS2Controller.keyboard();
        fabgl::VirtualKeyItem item;
        byte 		keycode = 0;						// Last pressed key code
        byte 		modifiers = 0;						// Last pressed key modifiers
        
        if(kb->getNextVirtualKey(&item, 0)) 
        {
            // check ZDI mode hotkeys
            if (!zdi_mode() && item.ASCII==0x1a && item.down)
            {
                zdi_enter ();
            }
            else
            {
                if (!zdi_mode())
                {
                    modifiers = 
                        item.CTRL		<< 0 |
                        item.SHIFT		<< 1 |
                        item.LALT		<< 2 |
                        item.RALT		<< 3 |
                        item.CAPSLOCK	<< 4 |
                        item.NUMLOCK	<< 5 |
                        item.SCROLLLOCK << 6 |
                        item.GUI		<< 7
                    ;
                    byte packet[] = {
                        item.ASCII,
                        modifiers,
                        item.vk,
                        item.down,
                    };
                    // key is echoed back by EZ80
                    send_packet(PACKET_KEYCODE, sizeof packet, packet);
                }
                else
                {
                    if (item.down)
                        zdi_process_cmd (item.ASCII);
                }
            }
        }
    }
}

void boot_screen()
{
    // initialize terminal
    Terminal.write("\e[44;37m"); // background: blue, foreground: white
    Terminal.write("\e[2J");     // clear screen
    Terminal.write("\e[1;1H");   // move cursor to 1,1

    hal_printf("Electron - HAL - version 0.0.1\r\n");
    hal_printf("a playful alternative to Quark\r\n\n");

    // send ESC to EZ80
    ez80_serial.write(27);
}

void setup()
{
    // Disable the watchdog timers
    disableCore0WDT(); delay(200);								
	disableCore1WDT(); delay(200);

    // setup connection from ESP to EZ80
    ez80_serial.end();
    ez80_serial.setRxBufferSize(1024);
    ez80_serial.begin(UART_BR, SERIAL_8N1, UART_RX, UART_TX);
    ez80_serial.setHwFlowCtrlMode(HW_FLOWCTRL_RTS, 64);			// Can be called whenever
	ez80_serial.setPins(UART_NA, UART_NA, UART_CTS, UART_RTS);	// Must be called after begin
    pinMode(UART_RTS, OUTPUT);
    pinMode(UART_CTS, INPUT);	
    setRTSStatus(true);

    // setup keyboard/PS2
    PS2Controller.begin(PS2Preset::KeyboardPort0, KbdMode::CreateVirtualKeysQueue);

    // setup VGA display
    DisplayController.begin();
    DisplayController.setResolution(VGA_640x480_60Hz);
    
    // setup terminal
    Terminal.begin(&DisplayController);
    Terminal.enableCursor(true);

    // setup serial to hostpc and link to terminal
    hal_hostpc_serial (&Terminal);
}

void loop()
{
    boot_screen();

    while (1)
    {
        do_keyboard();
        
        if (ez80_serial.available() > 0)
        {
            if(ez80_serial.available() > UART_RX_THRESH) {
               setRTSStatus(false);		
            }
            byte c = ez80_serial.read();
            if (c>=0x20 && c<0x80) {
                hal_printf ("%c",c);
            }
            else
            {
                switch (c)
                {
                    case '\r':
                    case '\n':
                        hal_printf ("%c",c);
                        break;
                    case 0x17:
                        if ((c=ez80_serial.read())==0)
                        {
                          if ((c=ez80_serial.read())==0x86)
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
                            send_packet(PACKET_MODE, sizeof packet, packet);
                            break;
                          }
                        }
                    default:
                        hal_printf ("0x%02X;",c);
                        break;
                }
            }
        }
        else 
        {
          setRTSStatus(true);
        }

    }
}
