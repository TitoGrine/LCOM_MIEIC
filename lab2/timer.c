#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int hook_id = 0;
int g_counter = 0;

int (timer_set_frequency)(uint8_t (timer), uint32_t (freq)) {
  
  uint8_t ctrl_word;

  /* getting current control word of the specified timer and making sure the 
  last 4 bits aren't changed */
  timer_get_conf(timer, &ctrl_word);
  ctrl_word &= 0x0f;

  ctrl_word |= TIMER_LSB_MSB;

  switch (timer) {
    case 0:
      ctrl_word |= TIMER_SEL0;
      break;

    case 1:
      ctrl_word |= TIMER_SEL1;
      break;

    case 2:
      ctrl_word |= TIMER_SEL2;
      break;

    default:
      return 1;
  }

  uint16_t initial_value = (TIMER_FREQ / freq);
  uint8_t LSB, MSB;

  sys_outb(TIMER_CTRL, ctrl_word);

  util_get_LSB(initial_value, &LSB);
  util_get_MSB(initial_value, &MSB);

  switch (timer) {
    case 0:
      sys_outb(TIMER_0, LSB);
      sys_outb(TIMER_0, MSB);
      break;

    case 1:
      sys_outb(TIMER_1, LSB);
      sys_outb(TIMER_1, MSB);
      break;

    case 2:
      sys_outb(TIMER_2, LSB);
      sys_outb(TIMER_2, MSB);
      break;

    default:
      return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *(bit_no)) {
  
  *bit_no = hook_id;
  sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id);

  return 0;
}

int (timer_unsubscribe_int)() {
  
  sys_irqrmpolicy(&hook_id);

  return 0;
}

void (timer_int_handler)() {
  g_counter ++;
}

int (timer_get_conf)(uint8_t (timer), uint8_t *(st)) {
  
  uint32_t st32 = 0;  

  uint8_t config = 0; //readback command

  config |= TIMER_RB_CMD;
  config |= TIMER_RB_COUNT_;
  // config |= ~TIMER_RB_STATUS_; not necessary because status is initialized as 0
  config |= TIMER_RB_SEL(timer);

  printf("Configuration = %u\n", config);

  sys_outb(TIMER_CTRL, config);
  
  switch (timer) {
    
    case 0:
      sys_inb(TIMER_0, &st32);
      break;
    
    case 1:
      sys_inb(TIMER_1, &st32);
      break;
    
    case 2:
      sys_inb(TIMER_2, &st32);
      break;
    
    default:
      return 1;
  }

  printf("Output = %u\n", st32);

  *st = (uint8_t) st32;
  return 0;
}

int (timer_display_conf)(uint8_t (timer), uint8_t (st), enum timer_status_field (field)) {

  union timer_status_field_val conf;

  switch(field) {
    
    case all:
      conf.byte = st; //to get the status byte out of the union
      break;

    case initial:
      st &= 0x30;
      st >>= 4;
      conf.in_mode = st;
      break;
    
    case mode:
      st &= 0x0e;
      st >>= 1;
      conf.count_mode = st;
      break;

    case base:
      st &= 0x01;
      conf.bcd = st;
      break;
    
    default:
      return 0;
  }

  timer_print_config(timer, field, conf); 

  return 0;
}
