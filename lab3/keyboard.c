#include "keyboard.h"


static int hook_id_keyboard = 2;

uint32_t sys_inb_counter = 0;

uint8_t data;

bool error_flag = false;

int (kbd_subscribe_int)(uint8_t *bit_no) {

  *bit_no = hook_id_keyboard;
  sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_keyboard);

  return 0;
}

int (kbd_unsubscribe_int)() {

  sys_irqrmpolicy(&hook_id_keyboard);

  return 0;
}

void (kbc_ih)(void) {

  uint32_t status_register;

  sys_inb_cnt(STAT_REG, &status_register);

  if ( (status_register & ST_PARITY) || (status_register & ST_TIMEOUT) )
    error_flag = true;
  else 
    error_flag = false;
    
  if (status_register & ST_OBF) {
    uint32_t output;

    sys_inb_cnt(OUT_BUF, &output);

    data = (uint8_t) output;
  
  }
  
}

int (sys_inb_cnt)(port_t port, uint32_t *byte) {

  if ( sys_inb(port, byte) == 0 ) {
    sys_inb_counter++;
    return 0;
  }
  else
    return 1;
  
}

int (kbc_cmd)(uint8_t command, uint8_t *arg_or_res, bool expect_res) {

  uint32_t status;
  uint32_t kbc_command = command;

  while ( 1 ) {
    sys_inb_cnt(STAT_REG, &status);

    if ( (status & ST_IBF) == 0 ) {
      sys_outb(KBC_CMD_REG, kbc_command);
      break;
    }
    tickdelay(micros_to_ticks(DELAY));
  }

  if ( expect_res ) {

    while ( 1 ) {
      sys_inb_cnt(STAT_REG, &status);

      if ( (status & ST_OBF) && !(status & (ST_PARITY | ST_TIMEOUT)) ) {
        sys_inb_cnt(OUT_BUF, &kbc_command);
        *arg_or_res = (uint8_t) kbc_command;
        break;
      }
      tickdelay(micros_to_ticks(DELAY));
    }

  }
  else {
    kbc_command = *arg_or_res;

    while ( 1 ) {
      sys_inb_cnt(STAT_REG, &status);

      if ( !(status & ST_OBF) ) {
        sys_outb(OUT_BUF, kbc_command);
        break;
      }
      tickdelay(micros_to_ticks(DELAY));
    }
  }
  return 0;
}
