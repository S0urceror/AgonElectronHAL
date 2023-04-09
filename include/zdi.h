#ifndef __ZDI_H_
#define __ZDI_H_

#define ZDI_TCK 26
#define ZDI_TDI 27
#define ZDI_READ 1
#define ZDI_WRITE 0
#define ZDI_CMD_CONTINUE 0
#define ZDI_CMD_DONE 1

// nr of microseconds to wait for next bit
// documentation says 4Mhz ZCLK speed is possible, so 0.25 usecs
// we stay a bit on the safe side with 10 usecs
#define ZDI_WAIT_MICRO 10

// ZDI write registers
#define ZDI_ADDR0_L     0x00
#define ZDI_ADDR0_H     0x01
#define ZDI_ADDR0_U     0x02
#define ZDI_ADDR1_L     0x04
#define ZDI_ADDR1_H     0x05
#define ZDI_ADDR1_U     0x06
#define ZDI_ADDR2_L     0x08
#define ZDI_ADDR2_H     0x09
#define ZDI_ADDR2_U     0x0A
#define ZDI_ADDR3_L     0x0c
#define ZDI_ADDR3_H     0x0d
#define ZDI_ADDR3_U     0x0e
#define ZDI_BRK_CTL     0x10
#define ZDI_MASTER_CTL  0x11
#define ZDI_WR_DATA_L   0x13
#define ZDI_WR_DATA_H   0x14
#define ZDI_WR_DATA_U   0x15
#define ZDI_RW_CTL      0x16
#define ZDI_BUS_CTL     0x17
#define ZDI_IS4         0x21
#define ZDI_IS3         0x22
#define ZDI_IS2         0x23
#define ZDI_IS1         0x24
#define ZDI_IS0         0x25
#define ZDI_WR_MEM      0x30

// ZDI read registers
#define ZDI_ID_L        0x00
#define ZDI_ID_H        0x01
#define ZDI_ID_REV      0x02
#define ZDI_STAT        0x03
#define ZDI_RD_L        0x10
#define ZDI_RD_H        0x11
#define ZDI_RD_U        0x12
#define ZDI_BUS_STAT    0x17
#define ZDI_RD_MEM      0x20

// CPU read/write values
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
bool zdi_mode ();
void zdi_enter ();
void zdi_exit ();
uint8_t zdi_available_break_point();

// zdi interface functions
void zdi_debug_status (debug_state_t state);
void zdi_intel_hex (byte* memory,uint32_t start,uint16_t size);
void zdi_process_line ();
void zdi_process_cmd (uint8_t key);


#endif