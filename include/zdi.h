#ifndef __ZDI_H_
#define __ZDI_H_

#define ZDI_TCK 26
#define ZDI_TDI 27
#define ZDI_READ 1
#define ZDI_WRITE 0
#define ZDI_WAIT_MICRO 50
#define ZDI_CMD_CONTINUE 0
#define ZDI_CMD_DONE 1

typedef enum 
{
    REG_AF,
    REG_BC,
    REG_DE,
    REG_HL,
    REG_IX,
    REG_IY,
    REG_SP,
    REG_PC,
    SET_ADL,
    RESET_ADL,
    EXX,
    MEM
} rw_control_t;

typedef enum
{
    BREAK,
    STEP,
    RUN
} debug_state_t;

// low-level bit stream
void zdi_start ();
void zdi_write_bit (bool bit);
bool zdi_read_bit ();
void zdi_register (byte regnr,bool read);
void zdi_separator (bool done_or_continue);
void zdi_write (byte value);
byte zdi_read ();

// medium-level register read and writes
byte zdi_read_register (byte regnr);
void zdi_write_register (byte regnr, byte value);
void zdi_read_registers (byte startregnr, byte count, byte* values);
void zdi_write_registers (byte startregnr, byte count, byte* values);

// high-level debugging, register and memory read functions

void do_zdi();
bool zdi_mode ();
void zdi_process_cmd (uint8_t key);
void zdi_enter ();
void zdi_exit ();

#endif