#ifndef I8042_H
#define I8042_H

#define BIT(n)            (0x01<<(n))

#define KBD_IRQ           1
#define MOUSE_IRQ         12

#define STAT_REG          0x64
#define KBC_CMD_REG       0x64
#define OUT_BUF           0x60

// STATUS REGISTER BITS
#define ST_PARITY         BIT(7)
#define ST_TIMEOUT        BIT(6)
#define ST_AUX            BIT(5)
#define ST_IBF            BIT(1)
#define ST_OBF            BIT(0)

// KBC COMMANDS
#define READ_CMD_BYTE     0x20
#define WRITE_CMD_BYTE    0x60
#define ENABLE_KBD_INT    0xAE
#define WRITE_BYTE_MOUSE  0xD4

// MOUSE COMMANDS
#define MOUSE_RESET       0xFF
#define DISABLE_DT_RPT    0xF5
#define ENABLE_DT_RPT     0xF4
#define SET_REMOTE_MODE   0xF0
#define READ_DATA         0xEB
#define SET_STREAM_MODE   0xEA

// ACKNOELEDGMENT BYTES
#define ACK               0xFA
#define NACK              0xFE
#define ERROR             0xFC


#define DELAY             20000

// COMMAND BYTE
#define INT2              BIT(1)
#define INT               BIT(0)

// MOUSE MASKS
#define Y_OVF             BIT(7)
#define X_OVF             BIT(6)
#define Y_SIGNAL          BIT(5)
#define X_SIGNAL          BIT(4)
#define MMB               BIT(2)
#define RMB               BIT(1)
#define LMB               BIT(0)

#endif
