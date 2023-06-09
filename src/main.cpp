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
#include "eos.h"
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
                #ifdef HW_FLOW_CONTROL_TEST
                // BAUDRATE and HW FLOW_CONTROL TEST
                if (ch=='+') 
                {
                    int amount = 65536;
                    hal_printf ("Sending %d bytes (R=receive, E=error): \r\n",amount);
                    long int t1 = millis();
                    for (int cnt=0;cnt<amount;cnt++)
                        ez80_serial.write ((byte)cnt);
                    long int t2 = millis();
                    hal_printf ("This took: %ld milliseconds\r\n",t2-t1);
                    hal_printf ("Send speed: %ld bits/sec\r\n",((amount*8*1000) / (t2-t1)));
                }
                else if  (ch=='-')
                {
                    int amount = 0;
                    bool firsttime = true;
                    hal_printf ("Receiving byte stream\r\n");
                    ez80_serial.write ('\0');
                    ez80_serial.write ('\0');
                    long int t1 = millis();
                    byte b = 0;
                    while (!ez80_serial.available());
                    while (true)
                    {
                        ch = ez80_serial.read();
                        if (ch==0)
                        {
                            b=0;
                            if (firsttime)
                                firsttime = false;
                            else
                                break;
                        }
                        else
                            firsttime = true;
                        if (ch!=b)
                            hal_printf ("E");
                        b++;
                        amount++;

                    }
                    long int t2 = millis();
                    hal_printf ("Received %d bytes\r\n",amount);
                    hal_printf ("This took: %ld milliseconds\r\n",t2-t1);
                    hal_printf ("Receive speed: %ld bits/sec\r\n",((amount*8*1000) / (t2-t1)));
                }
                else
                #endif
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
                if (item.down)
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

    hal_printf("Electron - HAL - version %d.%d.%d\r\n",HAL_major,HAL_minor,HAL_revision);
    #ifdef MOS_COMPATIBILITY
    hal_printf("\r\nLimited MOS RC4 support\r\n\n");
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

    #ifdef MOS_COMPATIBILITY
    mos_init ();
    #else
    if (!eos_wakeup ())
    {
        hal_printf ("Electron - OS - not running\r\n");
    }
    #endif

    while (1)
    {
        do_keys_ps2();
        do_keys_hostpc();
                
        // ez80 has send us something
        if (ez80_serial.available() > 0)
        {
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
                    case 0x07: // BELL
                        hal_printf ("%c",c);
                        break;
                    case 0x08: // backspace
                    case 0x7f:
                        #ifdef MOS_COMPATIBILITY
                        mos_col_left ();
                        #endif
                        hal_printf ("\b \b");
                        break;
                    case 0x09: // TAB
                        hal_printf ("%c",c);
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
                    #else
                    case CTRL_Z:
                    case ESC:
                        // absorb, for now
                        break;
                    #endif
                    default:
                        hal_printf ("ERR:0x%02X;",c);
                        break;
                }
            }
        }
    }
}
