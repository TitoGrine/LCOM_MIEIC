#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

#include "keyboard.h"

extern uint8_t asm_OUT;
extern int g_counter;
extern uint32_t sys_inb_counter;


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)(bool assembly) {

  uint8_t size_scancode = 1;
  uint8_t scancode[2] = {0, 0};

  uint8_t bit_no;

  kbd_subscribe_int(&bit_no);

  int ipc_status, r;
  message msg;

  uint8_t irq_set = 0 | BIT(bit_no);

  while (!(scancode[0] == ESC_BREAK)) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE: /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) {

            if (assembly) {
              kbc_asm_ih();

              scancode[0] = asm_OUT;

              if (asm_OUT == 0xE0) {
                scancode[0] = asm_OUT;
                size_scancode = 2;
              }
              else {
                if (size_scancode == 2) {
                  scancode[1] = asm_OUT;
                  kbd_print_scancode(!(scancode[1] & BIT(7)), size_scancode, scancode);
                }
                else {
                  scancode[0] = asm_OUT;
                  kbd_print_scancode(!(scancode[0] & BIT(7)), size_scancode, scancode);
                }
                size_scancode = 1;
              }
            }
            else {
              kbc_ih();

              if (!error_flag) {

                if (data == 0xE0) {
                  scancode[0] = data;
                  size_scancode = 2;
                }
                else {
                  if (size_scancode == 2) {
                    scancode[1] = data;
                    kbd_print_scancode(!(scancode[1] & BIT(7)), size_scancode, scancode);
                  }
                  else {
                    scancode[0] = data;
                    kbd_print_scancode(!(scancode[0] & BIT(7)), size_scancode, scancode);
                  }
                  size_scancode = 1;
                }
              }
            }
          }
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    else {
      continue; /* received a standard message, not a notification */
                /* no standard messages expected: do nothing */
    }
  }

  if ( !assembly )
    kbd_print_no_sysinb(sys_inb_counter);

  kbd_unsubscribe_int();

  return 0;
}

int(kbd_test_poll)() {
  uint32_t status_register;

  uint8_t out_data = 0;
  uint8_t scancode[2];
  uint8_t size_scancode = 1;
  bool error_scancode;

  do {

    sys_inb_cnt(STAT_REG, &status_register);


    if ( (status_register & ST_PARITY) || (status_register & ST_TIMEOUT) )
      error_scancode = true;
    else 
      error_scancode = false;


    if ((status_register & ST_OBF) && ((status_register & ST_AUX) == 0)) {

      uint32_t output;

      sys_inb_cnt(OUT_BUF, &output);

      out_data = (uint8_t) output;

      if (!error_scancode) {

        if (out_data == 0xE0) {
          scancode[0] = out_data;
          size_scancode = 2;
        }
        else {
          if (size_scancode == 2) {
            scancode[1] = out_data;
            kbd_print_scancode(!(out_data & BIT(7)), size_scancode, scancode);
          }
          else {
            scancode[0] = out_data;
            kbd_print_scancode(!(out_data & BIT(7)), size_scancode, scancode);
          }
          size_scancode = 1;
        }
      }
    }

    tickdelay(micros_to_ticks(DELAY));

  } while (out_data != ESC_BREAK);

  uint8_t kbc_command = 0;

  kbc_cmd(READ_CMD_BYTE, &kbc_command, 1);

  kbc_command = kbc_command | INT;

  kbc_cmd(WRITE_CMD_BYTE, &kbc_command, 0);

  kbd_print_no_sysinb(sys_inb_counter);

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {

  uint8_t size_scancode = 1;
  uint8_t scancode[2] = {0, 0};

  uint8_t bit_no_keyboard;
  uint8_t bit_no_timer;

  timer_subscribe_int(&bit_no_timer);
  kbd_subscribe_int(&bit_no_keyboard);

  int ipc_status, r;
  message msg;


  uint8_t irq_set_timer = 0 | BIT(bit_no_timer);
  uint8_t irq_set_kbd = 0 | BIT(bit_no_keyboard);

  while (!(scancode[0] == ESC_BREAK) && (g_counter <  n * 60 )) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE: /* hardware interrupt notification */

          if (msg.m_notify.interrupts & irq_set_timer)  
            timer_int_handler();
          

          if (msg.m_notify.interrupts & irq_set_kbd) {
            kbc_ih();

            if (!error_flag) {

              if (data == 0xE0) {
                scancode[0] = data;
                size_scancode = 2;
              }
              else {
                if (size_scancode == 2) {
                  scancode[1] = data;
                  kbd_print_scancode(!(scancode[1] & BIT(7)), size_scancode, scancode);
                }
                else {
                  scancode[0] = data;
                  kbd_print_scancode(!(scancode[0] & BIT(7)), size_scancode, scancode);
                }
                size_scancode = 1;
              }
            }

            g_counter = 0;
          }
        default:
          break; /* no other notifications expected: do nothing */
      }
    }
    else {
      continue; /* received a standard message, not a notification */
                /* no standard messages expected: do nothing */
    }
  }

  kbd_unsubscribe_int();
  timer_unsubscribe_int();

  return 0;
}
