#ifndef I8042_H
#define I8042_H

#define BIT(n)        (0x01<<(n))

#define KBD_IRQ        1

#define STAT_REG       0x64
#define KBC_CMD_REG    0x64
#define OUT_BUF        0x60

#define ST_PARITY         BIT(7)
#define ST_TIMEOUT        BIT(6)
#define ST_AUX            BIT(5)
#define ST_IBF            BIT(1)
#define ST_OBF            BIT(0)

#define ESC_MAKE          0x01
#define ESC_BREAK         0x81

#define READ_CMD_BYTE     0x20
#define WRITE_CMD_BYTE    0x60
#define ENABLE_KBD_INT    0xAE


#define DELAY             20000
#define WAIT_KBC          20000

#define INT               BIT(0)

#endif
