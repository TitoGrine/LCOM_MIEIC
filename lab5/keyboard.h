#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "i8042.h"


extern uint8_t data;
extern bool error_flag;


int (kbd_subscribe_int)(uint8_t *bit_no);

int (kbd_unsubscribe_int)();

int (kbc_cmd)(uint8_t command, uint8_t *arg_or_res, bool expect_res);




#ifdef LAB3
int sys_inb_cnt(port_t port, uint32_t *byte);
#else
#define sys_inb_cnt(p,q) sys_inb(p,q)
#endif



#endif
