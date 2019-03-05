#ifndef MOUSE_H
#define MOUSE_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "i8042.h"


extern int hook_id_mouse;
extern uint8_t data;
extern bool error_flag;



typedef enum {INIT, LMB_PRESSED, MOVE, RMB_PRESSED, FINAL} state_t;
//typedef enum {RB_DOWN, RB_UP, LB_DOWN, LB_UP, MOVE_UP, MOVE_DOWN, NO_MOVE, WRONG_MOVE} ev_type_t;


void (mouse_ih)();

int (mouse_subscribe_int)(uint8_t *bit_no);
int (mouse_unsubscribe_int)();

int (kbc_cmd)(uint8_t command, uint8_t *arg_or_res, bool expect_res);
int (mouse_cmd)(uint8_t command);
void (check_event)(struct packet *data_packet, struct mouse_ev *event_ptr);
int16_t (complement_to_decimal)(uint16_t binary);

#endif
