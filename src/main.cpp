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
#include "mos.h"

#define SERIALKB 1

fabgl::PS2Controller    PS2Controller;
fabgl::VGA16Controller  DisplayController;
fabgl::Terminal         terminal;
bool                    ZDIMode=false;

void do_keys_hostpc ()
{
    byte ch;
    // characters in the buffer?
    if((ch=hal_hostpc_serial_read())>0) 
    {
		byte packet[] = {
			ch,
			0,
		};
        if (!zdi_mode() && ch==0x1a) 
        {
            // CTRL-Z?
            zdi_enter();
        }
        else
        {
            if (zdi_mode())
                // handle keys on the ESP32
                zdi_process_cmd (ch);
            else
                // send to MOS on the EZ80
		        mos_send_packet(PACKET_KEYCODE, sizeof packet, packet);
        }
	} 
}

void do_keys_ps2 ()
{
    fabgl::Keyboard *kb = PS2Controller.keyboard();
    fabgl::VirtualKeyItem item;
    byte keycode = 0;						// Last pressed key code
    byte modifiers = 0;						// Last pressed key modifiers
    
    if(kb->getNextVirtualKey(&item, 0)) 
    {
        // CTRL-Z?
        if (!zdi_mode() && item.ASCII==0x1a && item.down)
        {
            zdi_enter ();
        }
        else
        {
            // normal key pressed
            if (zdi_mode())
            {
                // handle keys on the ESP32
                if (item.down)
                    zdi_process_cmd (item.ASCII);
            }
            else
            {
                // send to MOS on the EZ80
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
                mos_send_packet(PACKET_KEYCODE, sizeof packet, packet);
            }
        }
    }
}

void boot_screen()
{
    // initialize terminal
    terminal.write("\e[44;37m"); // background: blue, foreground: white
    terminal.write("\e[2J");     // clear screen
    terminal.write("\e[1;1H");   // move cursor to 1,1

    hal_printf("Electron - HAL - version 0.0.1\r\n");
    hal_printf("a playful alternative to Quark\r\n\n");

    // stop MOS boot wait by sending ESC key
    mos_init ();
}

void setup()
{
    // Disable the watchdog timers
    disableCore0WDT(); delay(200);								
	disableCore1WDT(); delay(200);

    // setup connection from ESP to EZ80
    hal_ez80_serial_init();
    
    // setup keyboard/PS2
    PS2Controller.begin(PS2Preset::KeyboardPort0, KbdMode::CreateVirtualKeysQueue);

    // setup VGA display
    DisplayController.begin();
    DisplayController.setResolution(VGA_640x480_60Hz);
    
    // setup terminal
    terminal.begin(&DisplayController);
    terminal.enableCursor(true);

    // setup serial to hostpc and link to terminal
    hal_hostpc_serial_init (&terminal);
}

void loop()
{
    boot_screen();

    while (1)
    {
        do_keys_ps2();
        do_keys_hostpc();
                
        // ez80 has send us something
        if (ez80_serial.available() > 0)
        {
            // larger then buffer?
            if(ez80_serial.available() > UART_RX_THRESH) {
                // please stop sending
                setRTSStatus(false);		
            }
            // read character
            byte c = ez80_serial.read();
            if (c>=0x20 && c<0x80) 
            {
                // normal ASCII?
                hal_printf ("%c",c);
                mos_col_right();
            }
            else
            {
                // special character or MOS escape code
                switch (c)
                {
                    case '\r':
                    case '\n':
                        mos_set_column (1);
                    case 0x09: // ??
                        hal_printf ("%c",c);
                        break;
                    case 0x08: // backspace
                        mos_col_left ();
                        hal_printf ("%c %c",c,c);
                        break;
                    case 0x0c: // formfeed
                        terminal.clear ();
                        mos_set_column (1);
                        break;
                    case 23: // MOS escape code
                        if ((c=ez80_serial.read())==0)
                        {
                            c=((byte) ez80_serial.read());
                            switch (c)
                            {
                                case 0x80: // general poll
                                    mos_send_general_poll ();
                                    break;
                                case 0x86: // VDP_MODE
                                    mos_send_vdp_mode ();
                                    break;
                                case 0x82: // VDP_CURSOR
                                    mos_send_cursor_pos ();
                                    break;
                                default:
                                    hal_printf ("VDP:0x%02X;",c);
                                    break;
                            }
                        }
                        else
                            hal_printf ("MOS:0x%02X;",c);
                        break;
                    default:
                        hal_printf ("ERR:0x%02X;",c);
                        break;
                }
            }
        }
        else 
        {
            // yes we can receive more
            setRTSStatus(true);
        }
    }
}
