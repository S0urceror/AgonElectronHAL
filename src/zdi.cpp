/*
 * Title:           Electron - HAL
 *                  a playful alternative to Quark
 *                  quarks and electrons combined are matter.
 * Author:          Mario Smit (S0urceror)
*/

// ZDI library

#include "hal.h"
#include "zdi.h"

byte debug_flags = 0x00;
bool zdi_mode_flag = false;
uint8_t upper_address = 0x00;
char szLine[80];
uint8_t charcnt = 0;

// low-level bit stream
///////////////////////
void zdi_start ()
{
    digitalWrite (ZDI_TDI,HIGH);
    pinMode (ZDI_TDI, OUTPUT);

    digitalWrite (ZDI_TCK,HIGH);
    usleep (ZDI_WAIT_MICRO);
    // start signal
    digitalWrite (ZDI_TDI,LOW);
    usleep (ZDI_WAIT_MICRO);
}
void zdi_write_bit (bool bit)
{
    pinMode (ZDI_TDI, OUTPUT);
    usleep (ZDI_WAIT_MICRO);

    digitalWrite (ZDI_TCK,LOW);
    usleep (ZDI_WAIT_MICRO);
    digitalWrite (ZDI_TDI,bit);
    usleep (ZDI_WAIT_MICRO);
    digitalWrite (ZDI_TCK,HIGH);
    usleep (ZDI_WAIT_MICRO);
}
bool zdi_read_bit ()
{
    bool bit;

    pinMode (ZDI_TDI, INPUT);
    usleep (ZDI_WAIT_MICRO);

    digitalWrite (ZDI_TCK,LOW);
    usleep (ZDI_WAIT_MICRO);
    bit = digitalRead (ZDI_TDI);
    usleep (ZDI_WAIT_MICRO);
    digitalWrite (ZDI_TCK,HIGH);
    usleep (ZDI_WAIT_MICRO);

    return bit;
}
void zdi_register (byte regnr,bool read)
{
    // write 7-bits, msb to lsb order
    for (int i=6;i>=0;i--)
    {
        zdi_write_bit (regnr & (0b1<<i));
    }
    // write read/write bit
    zdi_write_bit (read);
}
void zdi_separator (bool done_or_continue)
{
    zdi_write_bit (done_or_continue);
}
void zdi_write (byte value)
{
    // write 8-bits, msb to lsb order
    for (int i=7;i>=0;i--)
    {
        zdi_write_bit (value & (0b1<<i));
    }
}
byte zdi_read ()
{
    byte value = 0;
    // read 8-bits, msb to lsb order
    for (int i=7;i>=0;i--)
    {
        value = value << 1;
        value = value | zdi_read_bit ();
    }
    return value;
}

// medium-level register read and writes
///////////////////////

byte zdi_read_register (byte regnr)
{
    byte value;

    zdi_start ();
    zdi_register (regnr,ZDI_READ);
    zdi_separator (ZDI_CMD_CONTINUE);
    value  = zdi_read ();
    zdi_separator (ZDI_CMD_DONE);

    return value;
}
void zdi_write_register (byte regnr, byte value)
{
    zdi_start ();
    zdi_register (regnr,ZDI_WRITE);
    zdi_separator (ZDI_CMD_CONTINUE);
    zdi_write(value);
    zdi_separator (ZDI_CMD_DONE);
}

void zdi_read_registers (byte startregnr, byte count, byte* values)
{
    byte* ptr = values;

    zdi_start ();
    zdi_register (startregnr,ZDI_READ);
    while (count-- > 0)
    {
        zdi_separator (ZDI_CMD_CONTINUE);
        *(ptr++)  = zdi_read ();
    }
    zdi_separator (ZDI_CMD_DONE);
}

void zdi_write_registers (byte startregnr, byte count, byte* values)
{
    byte* ptr = values;

    zdi_start ();
    zdi_register (startregnr,ZDI_WRITE);
    while (count-- > 0)
    {
        zdi_separator (ZDI_CMD_CONTINUE);
        zdi_write (*(ptr++));
    }
    zdi_separator (ZDI_CMD_DONE);
}

// high-level debugging, register and memory read functions
///////////////////////
uint16_t zdi_get_productid ()
{
    byte pids[2];
    zdi_read_registers (ZDI_ID_L,2,pids);
    return (pids[1]<<8)+pids[0];
}
uint8_t zdi_get_revision ()
{
    return zdi_read_register (ZDI_ID_REV);
}
uint8_t zdi_get_cpu_status ()
{
    return zdi_read_register (ZDI_STAT);
}
uint8_t zdi_get_bus_status ()
{
    return zdi_read_register (ZDI_BUS_STAT);
}
uint32_t zdi_read_cpu (rw_control_t rw)
{
    zdi_write_register (ZDI_RW_CTL,rw);
    byte values[3];
    zdi_read_registers (ZDI_RD_L,3,values);
    return (values[2]<<16)+(values[1]<<8)+values[0]; // U+H+L
}
void zdi_write_cpu (rw_control_t rw,uint32_t value)
{
    byte values[3];
    values[0] = value & 0xff;       // L
    values[1] = (value>>8) & 0xff;  // H
    values[2] = (value>>16) & 0xff; // U
    zdi_write_registers (ZDI_WR_DATA_L,3,values);
    zdi_write_register (ZDI_RW_CTL,rw | 0b10000000); // set high bit to indicate write
}
void zdi_read_memory (uint32_t address,uint32_t count, byte* memory)
{
    byte* ptr = memory;
    // set start address
    zdi_write_cpu (REG_PC,address);
    // commence read from auto-increment read memory byte register
    zdi_start ();
    zdi_register (ZDI_RD_MEM,ZDI_READ);
    // one by one
    for (uint32_t i=0;i<count;i++)
    {
        zdi_separator (ZDI_CMD_CONTINUE);
        *(ptr++) = zdi_read ();
    }
    // done
    zdi_separator (ZDI_CMD_DONE);
}
void zdi_write_memory (uint32_t address,uint32_t count, byte* memory)
{
    byte* ptr = memory;

    zdi_write_cpu (REG_PC,address);
    // commence write from auto-increment read memory byte register
    zdi_start ();
    zdi_register (ZDI_WR_MEM,ZDI_WRITE);
    for (uint32_t i=0;i<count;i++)
    {
        zdi_separator (ZDI_CMD_CONTINUE);
        zdi_write (*(ptr++));
    }
    // done
    zdi_separator (ZDI_CMD_DONE);
}
void zdi_debug_break ()
{
    debug_flags |= 0b10000000;
    zdi_write_register (ZDI_BRK_CTL, debug_flags);
}
void zdi_debug_continue ()
{
    debug_flags &= 0b01111110;
    zdi_write_register (ZDI_BRK_CTL, debug_flags);
}
void zdi_debug_step ()
{
    // set single-step bit
    debug_flags |= 0b00000001;
    zdi_write_register (ZDI_BRK_CTL, debug_flags);
}
bool zdi_debug_breakpoint_reached ()
{
    byte status = zdi_get_cpu_status ();
    return (status & 0b10000000); // ZDI mode means breakpoint
}
void zdi_debug_breakpoint_enable (uint8_t index,uint32_t address)
{
    // set bp address
    byte bp[3] = {  (byte)(address & 0xff),
                    (byte)((address>>8) & 0xff),
                    (byte)((address>>16) & 0xff)
                 };
    zdi_write_registers (ZDI_ADDR0_L+4*index,3,bp);
    // enable bp
    debug_flags|=(0b00001000 << index);
    zdi_write_register (ZDI_BRK_CTL, debug_flags);
}
void zdi_debug_breakpoint_disable (uint8_t index)
{
    byte mask = (0b00001000 << index);
    debug_flags &= (~mask);
    zdi_write_register (ZDI_BRK_CTL, debug_flags);
}
uint8_t zdi_available_break_point()
{
    uint8_t i;
    byte mask = 0b00001000;
    for (i=0;i<4;i++)
    {
        if (!(debug_flags & (mask << i)))
            break;
    }
    if (i==4)
        return 0xff;

    return i;
}
void zdi_reset ()
{
    zdi_write_register (ZDI_MASTER_CTL, 0b10000000);
}
bool zdi_mode ()
{
    return zdi_mode_flag;
}
void zdi_enter ()
{
    // clear line buffer
    memset (szLine,0,sizeof(szLine));
    charcnt=0;
    upper_address = 0x00;
    debug_flags = 0x00;

    zdi_mode_flag = true;
    digitalWrite (ZDI_TCK,HIGH);
    pinMode (ZDI_TCK, OUTPUT);

    // get cpu identification
    hal_printf ("\r\nZDI mode EZ80: %X.%X\r\n#",zdi_get_productid (),zdi_get_revision());
}
void zdi_exit ()
{
    // remove all breakpoints
    for (uint8_t i=0;i<4;i++)
        zdi_debug_breakpoint_disable (i);

    // back to running mode if we are at breakpoint
    if (zdi_debug_breakpoint_reached())
        zdi_debug_continue ();

    zdi_mode_flag = false;
    hal_printf ("\r\n*");
}

// ZDI interface
///////////////////////
void zdi_debug_status (debug_state_t state)
{
    byte mem[8];
    // read register contents
    uint32_t pc = zdi_read_cpu (REG_PC);
    uint32_t af = zdi_read_cpu (REG_AF);
    uint8_t f = (af>>8) &0xff;
    uint8_t mbase = (af>>16) &0xff;
    uint32_t bc = zdi_read_cpu (REG_BC);
    uint32_t de = zdi_read_cpu (REG_DE);
    uint32_t hl = zdi_read_cpu (REG_HL);
    uint32_t ix = zdi_read_cpu (REG_IX);
    uint32_t iy = zdi_read_cpu (REG_IY);
    uint32_t sp = zdi_read_cpu (REG_SP);
    uint8_t status = zdi_get_cpu_status ();
    hal_printf ("\r\nA=%02X BC=%06X DE=%06X HL=%06X IX=%06X IY=%06X SP=%06X", af & 0xff,bc,de,hl,ix,iy,sp);
    hal_printf ("\r\nFlags=%c%c%c%c%c%c Status=%c%c%c%c PC=%06X MBASE=%02X",
                                                    f&0b10000000?'P':'N',
                                                    f&0b01000000?'Z':'.',
                                                    f&0b00010000?'H':'.',
                                                    f&0b00000100?'P':'V',
                                                    f&0b00000010?'A':'S',
                                                    f&0b00000001?'C':'.',
                                                    status&0b10000000?'Z':'.',
                                                    status&0b00010000?'A':'.',
                                                    status&0b00001000?'M':'.',
                                                    status&0b00001000?'I':'.',
                                                    pc,
                                                    mbase);        

    // show register contents
    if (status & 0b10000000) // stopped at a breakpoint
    {
        // disassembly at PC?               
        zdi_read_memory (pc,8,mem);
        hal_printf ("\r\n%06X %02X %02X %02X %02X %02X %02X %02X %02X",pc,mem[0],mem[1],mem[2],mem[3],mem[4],mem[5],mem[6],mem[7]);
        zdi_write_cpu (REG_PC,pc);
    }
    // show state
    const char* prompt;
    switch (state)
    {
        case RUN: prompt = "running";break;
        case BREAK: prompt = "break";break;
        case STEP: prompt = "step";break;
    }
    hal_printf ("\r\n(%s)\r\n#",prompt);                                                                            
}

void zdi_bin_to_intel_hex (byte* memory,uint32_t start,uint16_t size)
{
    uint8_t checksum=0;
    for (int i=0;i<size;i++)
    {
        if ((i%16)==0)
        {
            uint8_t new_upper_address = (start>>16);
            if (new_upper_address!=upper_address)
            {
                upper_address = new_upper_address;
                checksum = (~(0x02+0x04+upper_address));
                hal_printf (":0200000400%02X%02X\r\n",upper_address,checksum+1);
                checksum = 0;
            }
            uint16_t pc = i+(start&0xffff);
            hal_printf (":%02X%04X00",0x10,pc);
            checksum += 0x10 + ((pc>>8)&0xff) + (pc&0xff);
        }
        checksum+=memory[i];
        hal_printf ("%02X",memory[i]);
        if ((i%16)==15)
        {
            checksum = (~checksum);
            hal_printf ("%02X\r\n",checksum+1);
            checksum = 0;
        }
    }
    hal_printf (":00000001FF\r\n#");
}
bool cpu_was_at_breakpoint = false;
void zdi_intel_hex_to_bin(char* szLine, uint8_t charcnt)
{
    char szLen[3],szAddress[5],szType[3],szCheckSum[3],szByte[3];
    szByte[2]='\0';
    strncpy (szLen,szLine+1,2);szLen[2]='\0';
    strncpy (szAddress,szLine+3,4);szAddress[4]='\0';
    strncpy (szType,szLine+7,2);szType[2]='\0';
    strncpy (szCheckSum,szLine+charcnt-2,2);szCheckSum[2]='\0';
    uint8_t len = strtoul (szLen,NULL,16);
    uint32_t address = (uint16_t) strtoul (szAddress,NULL,16);
    uint8_t type = strtoul (szType,NULL,16);
    uint8_t checksum = strtoul (szCheckSum,NULL,16);
    uint8_t check = (uint8_t)len + 
                    (uint8_t)((address>>8)&0xff) + 
                    (uint8_t)(address&0xff) + 
                    (uint8_t)type;
    if (type==0)
    {
        // get pc
        uint32_t pc = zdi_read_cpu (REG_PC);

        // calculate long address
        address = (upper_address<<16)+address;
        
        // allocate memory
        byte* memory = (byte*) malloc (len);
        
        // parse data
        for (uint8_t i=0;i<len;i++)
        {
            strncpy (szByte,szLine+9+i*2,2);
            byte b = strtoul (szByte,NULL,16);
            memory[i] = b;
            check = check + b;
        }

        // only write to memory when no checksum error
        check = (~check);
        check++;
        if (check != checksum)
            hal_printf (" !");
        else
            // write to ez80 and free memory
            zdi_write_memory (address,len,memory);

        free (memory);

        // restore pc
        zdi_write_cpu (REG_PC, pc);
    }
    if (type==1)
    {
        // end of file
        if (!cpu_was_at_breakpoint)
            zdi_debug_continue ();

    }
    if (type==4 || type==2)
    {
        // start of file
        cpu_was_at_breakpoint = zdi_debug_breakpoint_reached();
        if (!cpu_was_at_breakpoint)
            zdi_debug_break ();

        char szUpper[5];
        strncpy (szUpper,szLine+9,4);szUpper[4]='\0';
        upper_address = strtoul (szUpper,NULL,16)&0xff;
    }
}
void zdi_process_line ()
{
    switch (szLine[0])
    {
        case 'h':
            hal_printf ("\r\nh               - this help message");
            hal_printf ("\r\nb               - break the program");
            hal_printf ("\r\nb address       - set breakpoint at hex address");
            hal_printf ("\r\nd nr            - unset breakpoint");
            hal_printf ("\r\nc               - continue the program");
            hal_printf ("\r\ns               - step by step");
            hal_printf ("\r\nr               - show registers and status");
            hal_printf ("\r\nj address       - jump to address");
            hal_printf ("\r\nx address size  - examine memory from address");
            hal_printf ("\r\n:0123456789ABCD - data in Intel Hex format");
            hal_printf ("\r\n#");
            break;
        case 'b':
            if (charcnt==1)
            {
                // issue BREAK next instruction
                zdi_debug_break ();
                zdi_debug_status (BREAK);
            }
            else
            {
                // set breakpoint to address
                uint32_t address = strtoul (szLine+1,NULL,16);
                uint8_t bp = zdi_available_break_point();
                if (bp!=255)
                {
                    zdi_debug_breakpoint_enable (bp,address);
                    hal_printf ("\r\n(bp%d set to 0x%06X)\r\n#",bp+1,address);
                }
                else
                    hal_printf ("\r\n(error, not more than 4 hw breakpoints)\r\n#");
            }
            break;
        case 'd':
            if (charcnt>1)
            {
                // delete breakpoint
                uint8_t bp = strtoul (szLine+1,NULL,10);
                bp--;
                if (bp<4)
                {
                    zdi_debug_breakpoint_disable (bp);
                    hal_printf ("\r\n(bp%d disabled)\r\n#",bp+1);
                }
                else
                    hal_printf ("\r\n(error, not more than 4 hw breakpoints)\r\n#");
            }
            else
                hal_printf ("\r\n(error, specify breakpoint)\r\n#");
            break;
        case 'j': // jump to address
            if (charcnt>2)
            {
                u32_t address=strtoul (szLine+2,NULL,16);
                bool already_breaked = false;
                if (zdi_debug_breakpoint_reached())
                    already_breaked = true;
                else
                    zdi_debug_break ();
                zdi_write_cpu (REG_PC,address);
                // continue if
                if (!already_breaked)
                {
                    hal_printf ("\r\n(jumping to 0x%06X)\r\n#",address);
                    zdi_debug_continue();
                }
                else
                {
                    hal_printf ("\r\n(PC set to 0x%06X)\r\n#",address);
                }
            }
            else
            {
                hal_printf ("\r\n(error, no jump address)\r\n#");
            }
            break;
        case 'x': // examine, memory dump
            if (charcnt>2)
            {
                hal_printf ("\r\n");
                upper_address = 0x00;

                // break cpu
                bool already_breaked = false;
                if (zdi_debug_breakpoint_reached())
                    already_breaked = true;
                else
                    zdi_debug_break ();
                
                // get pc
                uint32_t pc = zdi_read_cpu (REG_PC);

                // read memory
                char* pStart = szLine+2;
                char* pSize;
                u32_t address=strtoul (pStart,&pSize,16);
                u16_t size=16;
                if (*pSize != '\0')
                    size = strtoul (pSize,NULL,16);

                byte* mem = (byte*) malloc (size);
                zdi_read_memory (address,size,mem);
                zdi_bin_to_intel_hex (mem,address,size);
                free (mem);

                // restore pc
                zdi_write_cpu (REG_PC, pc);

                // continue if
                if (!already_breaked)
                    zdi_debug_continue();
            }
            else
                hal_printf ("\r\n(error, no start address)\r\n#");
            break;
        case 'c':
            if (zdi_debug_breakpoint_reached()) // breakpoint?
            {
                zdi_debug_continue();
                hal_printf ("\r\n(continue)\r\n#");
            }
            else
                hal_printf ("\r\n#");
            break;
        case 'r':
            if (zdi_debug_breakpoint_reached())
                zdi_debug_status (BREAK);
            else
                zdi_debug_status (RUN);
            break;
        case 's':
        case '\0':
            if (zdi_debug_breakpoint_reached()) // breakpoint?
            {
                zdi_debug_step ();
                zdi_debug_status (STEP);
            }
            else    
                hal_printf ("\r\n#");
            break;
        case ':':
            if (charcnt>=1+2+4+2+2)
            {
                zdi_intel_hex_to_bin (szLine,charcnt);
                hal_printf ("\r\n#");
            }
            else
                hal_printf ("\r\n(wrong Intel Hex format)\r\n#");
            break;
        default:
            hal_printf ("\r\n(unknown command)\r\n#");
            break;
    }
}
void zdi_process_cmd (uint8_t key)
{
    switch (key)
    {
        case 0x08: // backspace
            hal_printf ("%c %c",key,key);
            szLine[--charcnt]='\0';
            break;
        case 'q':
        case 0x1b: // escape
            zdi_exit();
            break;
        case '\r': // carriage return
            // process line
            zdi_process_line ();
            memset (szLine,0,sizeof(szLine));
            charcnt=0;
            break;
        case '\n': // newline
            // absorb
            break;
        default:
            // echo characters and add to line buffer
            hal_printf ("%c",key);
            if (charcnt<sizeof (szLine))
                szLine[charcnt++]=key;
    }
}

