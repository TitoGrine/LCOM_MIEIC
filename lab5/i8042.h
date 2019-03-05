#ifndef I8042_H
#define I8042_H


// GENERAL MACROS

#define BIT(n)            (0x01<<(n))
#define DELAY             20000
#define STAT_REG          0x64
#define OUT_BUF           0x60


// STATUS REGISTER BITS

#define ST_PARITY         BIT(7)
#define ST_TIMEOUT        BIT(6)
#define ST_AUX            BIT(5)
#define ST_IBF            BIT(1)
#define ST_OBF            BIT(0)


// ACKNOWLEDGEMENT BYTES

#define ACK               0xFA
#define NACK              0xFE
#define ERROR             0xFC


// COMMAND BYTE

#define INT2              BIT(1)
#define INT               BIT(0)


// KEYBOARD MACROS

#define READ_CMD_BYTE     0x20
#define WRITE_CMD_BYTE    0x60
#define ENABLE_KBD_INT    0xAE
#define WRITE_BYTE_MOUSE  0xD4

#define KBD_IRQ           1
#define KBC_CMD_REG       0x64
#define ESC_BREAK         0x81


// MOUSE MACROS

#define MOUSE_RESET       0xFF
#define DISABLE_DT_RPT    0xF5
#define ENABLE_DT_RPT     0xF4
#define SET_REMOTE_MODE   0xF0
#define READ_DATA         0xEB
#define SET_STREAM_MODE   0xEA

#define Y_OVF             BIT(7)
#define X_OVF             BIT(6)
#define Y_SIGNAL          BIT(5)
#define X_SIGNAL          BIT(4)
#define MMB               BIT(2)
#define RMB               BIT(1)
#define LMB               BIT(0)

#define MOUSE_IRQ         12


// GRAPHICS MACROS

#define TEXT_MODE         0x03
#define MODE_105          0x105
#define BLACK             0x00
#define TRANSPARENT       0x00
#define VBE_MODE_INFO     0x4F01
#define VBE_MODE          0x10
#define VBE_CALL_SUCCESS  0x00
#define RED_COMPONENT     3
#define GREEN_COMPONENT   2
#define BLUE_COMPONENT    1




#endif
