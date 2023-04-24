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
#ifdef MOS_COMPATIBILITY
#pragma message ("Building MOS compatible version of Electron HAL")
#include "mos.h"
#else
#pragma message ("Building a Electron OS compatible version of Electron HAL")
#endif

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
            {
                #ifdef MOS_COMPATIBILITY
                mos_send_character (ch);
                #else
                ez80_serial.write(ch);
                #endif
            }
        }
	} 
}

void do_keys_ps2 ()
{
    fabgl::Keyboard *kb = PS2Controller.keyboard();
    fabgl::VirtualKeyItem item;
    
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
                #ifdef MOS_COMPATIBILITY
                mos_send_virtual_key (item);
                #else
                ez80_serial.write(item.ASCII);
                #endif
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

    hal_printf("Electron - HAL - version 0.0.2\r\n");
    #ifdef MOS_COMPATIBILITY
    hal_printf("MOS RC4 support, not yet fully compatible\r\n\n");
    #else
    hal_printf("a playful alternative to Quark\r\n\n");
    #endif
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

    // stop MOS boot wait by sending ESC key
    #ifdef MOS_COMPATIBILITY
    mos_init ();
    #endif

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
                #ifdef MOS_COMPATIBILITY
                mos_col_right();
                #endif
            }
            else
            {
                // special character or MOS escape code
                switch (c)
                {
                    case '\r':
                    case '\n':
                        #ifdef MOS_COMPATIBILITY
                        mos_set_column (1);
                        #endif
                    case 0x09: // ??
                        hal_printf ("%c",c);
                        break;
                    case 0x08: // backspace
                        #ifdef MOS_COMPATIBILITY
                        mos_col_left ();
                        #endif
                        hal_printf ("%c %c",c,c);
                        break;
                    case 0x0c: // formfeed
                        terminal.clear ();
                        #ifdef MOS_COMPATIBILITY
                        mos_set_column (1);
                        #endif
                        break;
                    #ifdef MOS_COMPATIBILITY
                    case 0x17: // MOS escape code 23
                        mos_handle_escape_code ();
                        break;
                    #endif
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
