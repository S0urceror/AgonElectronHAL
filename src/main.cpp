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
#include "globals.h"
#include "hal.h"
#include "zdi.h"
#pragma message ("Building a Electron OS compatible version of Electron HAL")
#include "eos.h"
#include "tms9918.h"
#include "ay_3_8910.h"
#include "audio_driver.h"

fabgl::PS2Controller        ps2;
fabgl::VGABaseController*	display = nullptr;
fabgl::Terminal*            terminal;
TMS9918                     vdp;
AY_3_8910                   psg;
bool                        ignore_escape = false;
bool                        display_mode_direct=false;
TaskHandle_t                mainTaskHandle;

void boot_screen()
{
    // initialize terminal
    terminal->write("\e[44;37m"); // background: blue, foreground: white
    terminal->write("\e[2J");     // clear screen
    terminal->write("\e[1;1H");   // move cursor to 1,1
    
    hal_printf("Electron - HAL - version %d.%d.%d\r\n",HAL_major,HAL_minor,HAL_revision);
}

void set_display_direct ()
{
    if (display_mode_direct)
        return;
    display_mode_direct = true;
    // destroy old display instance
    if (display)
    {
        terminal->deactivate();
        // destroy old display
        display->end();
        delete display;
        display = nullptr;
        // destroy old terminal
        //terminal->end();    
        // remove terminal from HAL
        hal_set_terminal (nullptr);
        delete terminal;
    }
    // create new display instance
    display = new fabgl::VGADirectController ();

    // tell vdp to use the new display
    vdp.set_display((fabgl::VGADirectController*) display);

    // setup new display   
    display->begin();
    display->setResolution(QVGA_320x240_60Hz);

    //hal_hostpc_printf ("display set to DIRECT\r\n");
}

void set_display_normal (bool force)
{
    if (!force && !display_mode_direct)
        return;
        
    display_mode_direct = false;

    if (display)
    {
        display->end();
        delete display;
        display = nullptr;
        // tell vdp to not use the display
        vdp.set_display((fabgl::VGADirectController*) nullptr);
    }
    // create new display instance
    display = new fabgl::VGA16Controller ();
    display->begin();
    display->setResolution(VGA_640x480_60Hz);
    
    // setup terminal
    terminal = new fabgl::Terminal ();
    terminal->begin(display);
    terminal->enableCursor(true);

    // let hal know what our terminal is
    hal_set_terminal (terminal);

    // hello world
    boot_screen ();
    //hal_hostpc_printf ("display set to NORMAL\r\n");
}


void do_serial_hostpc ()
{
    byte ch;
    while (true)
    {
        // characters in the buffer?
        if((ch=hal_hostpc_serial_read())>0) 
        {
            if (!zdi_mode() && ch==CTRL_Z) 
            {
                // CTRL-Z?
                zdi_enter();
            }
            else if (ch==CTRL_Y)
            {
                // CTRL-Y
                vdp.toggle_enable ();
            }
            else if (ch==CTRL_X)
            {
                // CTRL-X
                //vdp.cycle_screen2_debug ();
                if (display_mode_direct)
                    set_display_normal ();
                else
                    set_display_direct ();
            }
            else
            {
                if (zdi_mode())
                    // handle keys on the ESP32
                    zdi_process_cmd (ch);
                else
                {
                    ez80_serial.write(ch);
                }
            }
        } 
        else
            break;
    }
}

void do_keys_ps2 ()
{
    fabgl::Keyboard *kb = ps2.keyboard();
    fabgl::VirtualKeyItem item;
    
    if(kb->getNextVirtualKey(&item, 0)) 
    {
        if (item.ASCII!=0)
        {
            if (item.down)
                ez80_serial.write(item.ASCII);
            if (item.ASCII==0x20) // space
            {
                // also send virtual keycode (for SNSMAT)
                ez80_serial.write(0x80);
                ez80_serial.write(fabgl::VK_SPACE);
                ez80_serial.write(item.down?1:0);
            }
        }
        else 
        {
            // send virtual keycode (for SNSMAT)
            ez80_serial.write(0x80);
            ez80_serial.write(item.vk);
            ez80_serial.write(item.down?1:0);
        }
    }
}

void setup()
{
    mainTaskHandle = xTaskGetCurrentTaskHandle();

    // Disable the watchdog timers
    disableCore0WDT(); delay(200);								
	disableCore1WDT(); delay(200);

    // setup connection from ESP to EZ80
    hal_ez80_serial_init();
    
    // setup keyboard/PS2
    ps2.begin(PS2Preset::KeyboardPort0, KbdMode::CreateVirtualKeysQueue);

    // setup VGA display
    set_display_normal (true);

    // setup audio driver
    init_audio_driver ();
    psg.init ();

    // setup serial to hostpc
    hal_hostpc_serial_init ();

    // boot screen and wait for ElectronOS on EZ80
    boot_screen();
    if (!eos_wakeup ())
    {
        hal_printf ("Electron - OS - not running\r\n");
        while (1)
            do_serial_hostpc(); // wait for ZDI mode hotkey to reprogram ElectronOS
    }
}

void process_character (byte c)
{
    //hal_hostpc_printf ("%02X",c);
                    
    if (ignore_escape)
    {   
        // absorb 2 byte and 3 byte (0x78,0x79) escape codes
        if (!(c==0x78 || c==0x79))
            ignore_escape=false;
        return;
    }
    if (c>=0x20 && c<=0x7f) 
    {
        // normal ASCII?
        hal_printf ("%c",c);
    }
    else
    {
        // special character or ElectronOS escape code
        switch (c)
        {
            case '\r':
            case '\n':
            case 0x07: // BELL
                hal_printf ("%c",c);
                break;
            case 0x08: // backspace
                hal_printf ("\b \b");
                break;
            case 0x09: // TAB
                hal_printf ("%c",c);
                break;
            case 0x0c: // formfeed
                terminal->clear ();
                break;
            case CTRL_W:
                // 8 bits ASCII code transferred
                c = ez80_serial.read();
                hal_printf ("%c",c);
                break;
            case CTRL_Z:
                break;
            case ESC:
                ignore_escape=true;
                break;
            default:
                hal_printf ("ERR:0x%02X;",c);
                break;
        }
    }
}

void process_cmd (uint8_t cmd)
{
    // strip high bit
    uint8_t subcmd = (cmd & 0b01100000) >> 5;
    uint8_t port,value;
    uint8_t normal_out_req[2],repeatable_io_req[3],fillable_io_req[4];
    int repeat_length,i;

    switch (subcmd)
    {
        case 0b00: // IO requests take 3 bytes
            switch (cmd&0b00000111)
            {
                case 0b000:
                    // OUTput, no repeat, no fill
                    // single OUTput
                    if (ez80_serial.readBytes (normal_out_req,2)==2)
                    {
                        port = normal_out_req[0];
                        value = normal_out_req[1];
                        
                        if (port==0x98 || port==0x99)
                            vdp.write (port,value);
                        if (port==0xa0 || port==0xa1)
                            psg.write (port,value);
                        //hal_printf ("OUT (%02X), %02X\r\n",port,value);
                    }
                    break;
                case 0b010:
                    // OUTput, LDIRVM, multiple times, different values
                    if (ez80_serial.readBytes (repeatable_io_req,3)==3)
                    {
                        port = repeatable_io_req[0];
                        repeat_length = (repeatable_io_req[1]<<8) + repeatable_io_req[2];
                        //hal_printf ("%02X,%02X,%02X - OUT repeat %d times\r\n",repeatable_io_req[0],repeatable_io_req[1],repeatable_io_req[2],repeat_length);
                        for (i=0;i<repeat_length;i++)
                        {
                            if (ez80_serial.readBytes (&value,1)==1)
                            {
                                //hal_printf ("OUT (%02X), %02X\r\n",port,value);
                                if (port==0x98 || port==0x99)
                                    vdp.write (port,value);
                                if (port==0xa0 || port==0xa1)
                                    psg.write (port,value);
                            }
                        }
                        //hal_printf ("OUT (%02Xh), xx - %d repeated\r\n",port,i);
                    }
                    break;
                case 0b100:
                    // OUTput, FILL, multiple times, same value
                    if (ez80_serial.readBytes (fillable_io_req,4)==4)
                    {
                        port = fillable_io_req[0];
                        repeat_length = (fillable_io_req[1]<<8) + fillable_io_req[2];
                        value = fillable_io_req[3];
                        for (i=0;i<repeat_length;i++)
                        {
                            if (port==0x98 || port==0x99)
                                vdp.write (port,value);
                            if (port==0xa0 || port==0xa1)
                                psg.write (port,value);
                        }
                        //hal_printf ("OUT (%02Xh), %02Xh - %d filled\r\n",port,value,i);
                    }
                    break;
                case 0b001:
                    // INput, single value
                    if (ez80_serial.readBytes (&port,1)==1)
                    {
                        if (port==0x98 || port==0x99)
                            value = vdp.read (port);
                        if (port==0xa2)
                            value = psg.read (port);

                        ez80_serial.write(value);
                        //hal_printf ("IN A,(%02X) => %02X\r\n",port,value);
                    }
                    break;
                case 0b011:
                    // INput, LDIRMV, multiple times, multiple value
                    if (ez80_serial.readBytes (repeatable_io_req,3)==3)
                    {
                        port = repeatable_io_req[0];
                        repeat_length = (repeatable_io_req[1]<<8) + repeatable_io_req[2];
                        //hal_printf ("%02Xh,%02Xh,%02Xh - IN repeat %d times\r\n",repeatable_io_req[0],repeatable_io_req[1],repeatable_io_req[2],repeat_length);
                        for (i=0;i<repeat_length;i++)
                        {
                            //hal_printf ("IN (%02Xh) => %02Xh\r\n",port,value);
                            if (port==0x98 || port==0x99)
                                value = vdp.read (port);
                            if (port==0xa2)
                                value = psg.read (port);

                            ez80_serial.write(value);
                        }
                        //hal_printf ("IN (%02Xh), xx - %d repeated\r\n",port,i);
                    }
                    break;
            }
            break;
        default:
            hal_printf ("Unsupported in/out to: 0x%02X\r\n",cmd);
            break;
    }
}

void do_serial_ez80 ()
{
    int read_character;
    while (true)
    {
        // check if ez80 has send us something
        read_character = ez80_serial.read();
        if (read_character!=-1)
        {
            // read character
            uint8_t ch = (uint8_t) read_character;
            if (ch & 0x80 /*0b10000000*/)
                process_cmd (ch);
            else
                process_character (ch);
        }
        else
            break;
    }
}

void loop()
{
    do_keys_ps2();
    do_serial_hostpc();
    do_serial_ez80();
}
