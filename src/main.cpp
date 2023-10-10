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
#pragma message ("Building a Electron OS compatible version of Electron HAL")
#include "eos.h"
#include "tms9918.h"
#include "ay_3_8910.h"
#include "audio_driver.h"

fabgl::PS2Controller        ps2;
fabgl::VGADirectController  display_direct;
fabgl::VGA16Controller      display_normal;
fabgl::VGABaseController*	display = nullptr;
fabgl::Terminal             terminal;
TMS9918                     vdp;
AY_3_8910                   psg;
bool                        ignore_escape = false;

constexpr int        scanlinesPerCallback = 2;  // screen height should be divisible by this value
static TaskHandle_t  mainTaskHandle;

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
                ez80_serial.write(ch);
            }
        }
	} 
}

void IRAM_ATTR drawScanline(void * arg, uint8_t * dest, int scanLine)
{
    auto width  = ((fabgl::VGADirectController*)display)->getScreenWidth();
    auto height = ((fabgl::VGADirectController*)display)->getScreenHeight();

    // draws "scanlinesPerCallback" scanlines every time drawScanline() is called
    for (int i = 0; i < scanlinesPerCallback; ++i) 
    {
        vdp.draw_screen (dest,scanLine);
        
        // go to next scanline
        ++scanLine;
        dest += width;
    }

    // give signal to main loop to continue
    //
    if (scanLine == height) 
    {
        // signal end of screen
        vTaskNotifyGiveFromISR(mainTaskHandle, NULL);
    }
}

void set_display_direct ()
{
    // destroy old terminal
    terminal.end();

    // destroy old display instance
    if (display)
    {
        display->end();
        display = nullptr;
    }
    // create new display instance
    display = display_direct.instance ();

    // tell vdp to use the new display
    vdp.set_display((fabgl::VGADirectController*) display);

    // setup new display
    display->begin();
    ((fabgl::VGADirectController*)display)->setScanlinesPerCallBack(scanlinesPerCallback);
    ((fabgl::VGADirectController*)display)->setDrawScanlineCallback(drawScanline);        
    display->setResolution(QVGA_320x240_60Hz);

    // setup terminal
    terminal.begin(display);
    //terminal.enableCursor(true);
}

void set_display_normal ()
{
    if (display)
        display->end();

    display = display_normal.instance ();
    display->begin();
    display->setResolution(VGA_640x480_60Hz);

    // setup terminal
    terminal.begin(display);
    terminal.enableCursor(true);
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
                // send virtual keycode
                ez80_serial.write(0x80);
                ez80_serial.write(fabgl::VK_SPACE);
                ez80_serial.write(item.down?1:0);
            }
        }
        else 
        {
            // send virtual keycode
            ez80_serial.write(0x80);
            ez80_serial.write(item.vk);
            ez80_serial.write(item.down?1:0);
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
    set_display_normal ();

    // setup audio driver
    init_audio_driver ();
    psg.init ();

    // setup serial to hostpc
    hal_hostpc_serial_init ();

    // let hal know what our terminal is
    hal_set_terminal (&terminal);
}

void process_character (byte c)
{
    if (ignore_escape)
    {   
        if (!(c==0x78 || c==0x79))
            ignore_escape=false;
        return;
    }
    if (c>=0x20 && c<0x7f) 
    {
        // normal ASCII?
        hal_printf ("%c",c);
    }
    else
    {
        // special character or MOS escape code
        switch (c)
        {
            case '\r':
            case '\n':
            case 0x07: // BELL
                hal_printf ("%c",c);
                break;
            case 0x08: // backspace
            case 0x7f:
                hal_printf ("\b \b");
                break;
            case 0x09: // TAB
                hal_printf ("%c",c);
                break;
            case 0x0c: // formfeed
                terminal.clear ();
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

void process_cmd (byte ch)
{
    // commands have the following structure:
    // byte 0 - 0x80 or 0x81, denoting OUTput or INput respectively
    // byte 1 - (nn), denoting port number
    // byte 2 - in case of an OUTput this contains the new value, 
    //          in case of INput we have to return this value
    byte port,value;
    // strip high bit
    uint8_t cmd = ch & 0x7f;
    switch (cmd)
    {
        case 0x00: 
            // OUTput
            while (ez80_serial.available()==0);
            port = ez80_serial.read();
            while (ez80_serial.available()==0);
            value = ez80_serial.read();

            //hal_printf ("OUT (%02X), %02X\r\n",port,value);

            if (port==0x98 || port==0x99)
                vdp.write (port,value);
            if (port==0xa0 || port==0xa1)
                psg.write (port,value);

            break;
        case 0x01:
            // INput
            while (ez80_serial.available()==0);
            port = ez80_serial.read();

            if (port==0x98 || port==0x99)
                value = vdp.read (port);
            if (port==0xa2)
                value = psg.read (port);

            ez80_serial.write(value);

            //hal_printf ("IN A,(%02X) => %02X\r\n",port,value);
            break;
        default:
            hal_printf ("Unsupported in/out to: 0x%02X\r\n",cmd);
            break;
    }
}

void loop()
{
    boot_screen();

    if (!eos_wakeup ())
    {
        hal_printf ("Electron - OS - not running\r\n");
    }

    while (1)
    {
        do_keys_ps2();
        do_keys_hostpc();
                
        // ez80 has send us something
        if (ez80_serial.available() > 0)
        {
            // read character
            byte ch = ez80_serial.read();

            if (ch & 0x80 /*0b10000000*/)
                process_cmd (ch);
            else
                process_character (ch);

        }
    }
}
