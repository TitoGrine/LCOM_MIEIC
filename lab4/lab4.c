// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "mouse.h"

extern int g_counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(mouse_test_packet)(uint32_t cnt) {

  struct packet packet_data;
  uint8_t packet_size = 0;
  bool x_sign, y_sign;

  uint8_t bit_no;

  mouse_subscribe_int(&bit_no);

  sys_irqdisable(&hook_id_mouse);
  mouse_cmd(ENABLE_DT_RPT);
  sys_irqenable(&hook_id_mouse);

  int ipc_status, r;
  message msg;

  uint8_t irq_set = 0 | BIT(bit_no);

  while (cnt > 0) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE: /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) {

            mouse_ih();

            if (!error_flag) {

              if (packet_size == 0 && (data & BIT(3))) {

                packet_data.bytes[packet_size] = data;

                packet_data.y_ov = data & Y_OVF;
                packet_data.x_ov = data & X_OVF;

                if (data & Y_SIGNAL)
                  y_sign = true;
                else
                  y_sign = false;
                if (data & X_SIGNAL)
                  x_sign = true;
                else
                  x_sign = false;

                packet_data.mb = data & MMB;
                packet_data.rb = data & RMB;
                packet_data.lb = data & LMB;

                packet_size++;
              }
              else if (packet_size == 1) {
                packet_data.bytes[packet_size] = data;
                if (x_sign)
                  packet_data.delta_x = 0xFF00 | data;
                else
                  packet_data.delta_x = data;
                packet_size++;
              }
              else if (packet_size == 2) {
                packet_data.bytes[packet_size] = data;
                if (y_sign)
                  packet_data.delta_y = 0xFF00 | data;
                else
                  packet_data.delta_y = data;

                packet_size = 0;
                mouse_print_packet(&packet_data);
                cnt--;
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

  sys_irqdisable(&hook_id_mouse);
  mouse_cmd(DISABLE_DT_RPT);
  sys_irqenable(&hook_id_mouse);

  mouse_unsubscribe_int(&hook_id_mouse);

  return 0;
}

int(mouse_test_async)(uint8_t idle_time) {

  uint8_t bit_no_mouse;
  uint8_t bit_no_timer;

  struct packet packet_data;
  uint8_t packet_size = 0;
  bool x_sign, y_sign;

  mouse_subscribe_int(&bit_no_mouse);

  sys_irqdisable(&hook_id_mouse);
  mouse_cmd(ENABLE_DT_RPT);
  sys_irqenable(&hook_id_mouse);

  timer_subscribe_int(&bit_no_timer);

  int ipc_status, r;
  message msg;

  int timer_freq = sys_hz();

  uint8_t irq_set_timer = 0 | BIT(bit_no_timer);
  uint8_t irq_set_mouse = 0 | BIT(bit_no_mouse);

  while (g_counter < idle_time * timer_freq) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE: /* hardware interrupt notification */

          if (msg.m_notify.interrupts & irq_set_timer)
            timer_int_handler();

          if (msg.m_notify.interrupts & irq_set_mouse) {
            mouse_ih();

            if (!error_flag) {

              if (packet_size == 0 && (data & BIT(3))) {

                packet_data.bytes[packet_size] = data;

                packet_data.y_ov = data & Y_OVF;
                packet_data.x_ov = data & X_OVF;

                if (data & Y_SIGNAL)
                  y_sign = true;
                else
                  y_sign = false;
                if (data & X_SIGNAL)
                  x_sign = true;
                else
                  x_sign = false;

                packet_data.mb = data & MMB;
                packet_data.rb = data & RMB;
                packet_data.lb = data & LMB;

                packet_size++;
              }
              else if (packet_size == 1) {
                packet_data.bytes[packet_size] = data;
                if (x_sign)
                  packet_data.delta_x = 0xFF00 | data;
                else
                  packet_data.delta_x = data;
                packet_size++;
              }
              else if (packet_size == 2) {
                packet_data.bytes[packet_size] = data;
                if (y_sign)
                  packet_data.delta_y = 0xFF00 | data;
                else
                  packet_data.delta_y = data;

                packet_size = 0;
                mouse_print_packet(&packet_data);
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

  sys_irqdisable(&hook_id_mouse);
  mouse_cmd(DISABLE_DT_RPT);
  sys_irqenable(&hook_id_mouse);

  mouse_unsubscribe_int();
  timer_unsubscribe_int();

  return 0;
}

int(mouse_test_remote)(uint16_t period, uint8_t cnt) {

  mouse_cmd(SET_REMOTE_MODE);

  while (cnt > 0) {

    struct packet packet_data;
    bool x_sign, y_sign;
    uint8_t index = 0;

    mouse_cmd(READ_DATA);

    while (index < 3) {

      uint32_t status_register;
      uint8_t data;

      sys_inb(STAT_REG, &status_register);

      if (!((status_register & ST_PARITY) || (status_register & ST_TIMEOUT)))

        if (status_register & ST_OBF) {
          sys_inb(OUT_BUF, &status_register);
          data = (uint8_t) status_register;
        }

      if (index == 0 && (data & BIT(3))) {

        packet_data.bytes[index] = data;

        packet_data.y_ov = data & Y_OVF;
        packet_data.x_ov = data & X_OVF;

        if (data & Y_SIGNAL)
          y_sign = true;
        else
          y_sign = false;

        if (data & X_SIGNAL)
          x_sign = true;
        else
          x_sign = false;

        packet_data.mb = data & MMB;
        packet_data.rb = data & RMB;
        packet_data.lb = data & LMB;

        index++;
      }
      else if (index == 1) {
        packet_data.bytes[index] = data;

        if (x_sign)
          packet_data.delta_x = 0xFF00 | data;
        else
          packet_data.delta_x = data;

        index++;
      }
      else if (index == 2) {
        packet_data.bytes[index] = data;
        if (y_sign)
          packet_data.delta_y = 0xFF00 | data;
        else
          packet_data.delta_y = data;

        index++;

        mouse_print_packet(&packet_data);
        cnt--;
      }
    }

    tickdelay(micros_to_ticks(period));
  }

  mouse_cmd(SET_STREAM_MODE);
  mouse_cmd(DISABLE_DT_RPT);

  uint8_t command_byte = minix_get_dflt_kbc_cmd_byte();
  kbc_cmd(WRITE_CMD_BYTE, &command_byte, false);

  return 0;
}

int(mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {

  struct packet packet_data;
  struct mouse_ev event;
  uint8_t packet_size = 0;
  bool x_sign, y_sign;

  uint8_t bit_no;

  mouse_subscribe_int(&bit_no);

  sys_irqdisable(&hook_id_mouse);
  mouse_cmd(ENABLE_DT_RPT);
  sys_irqenable(&hook_id_mouse);

  int ipc_status, r;
  message msg;

  uint8_t irq_set = 0 | BIT(bit_no);

  state_t state = INIT;

  int16_t length_x = 0, length_y = 0;
  /*
  bool rmb_pressed = 0;
  bool lmb_pressed = 0;
  */

  while (state != FINAL) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {

            mouse_ih();

            if (!error_flag) {

              if (packet_size == 0 && (data & BIT(3))) {

                packet_data.bytes[packet_size] = data;

                packet_data.y_ov = data & Y_OVF;
                packet_data.x_ov = data & X_OVF;

                if (data & Y_SIGNAL)
                  y_sign = true;
                else
                  y_sign = false;
                if (data & X_SIGNAL)
                  x_sign = true;
                else
                  x_sign = false;

                packet_data.mb = data & MMB;
                packet_data.rb = data & RMB;
                packet_data.lb = data & LMB;

                packet_size++;
              }
              else if (packet_size == 1) {
                packet_data.bytes[packet_size] = data;
                if (x_sign)
                  packet_data.delta_x = 0xFF00 | data;
                else
                  packet_data.delta_x = data;
                packet_size++;
              }
              else if (packet_size == 2) {
                packet_data.bytes[packet_size] = data;
                if (y_sign)
                  packet_data.delta_y = 0xFF00 | data;
                else
                  packet_data.delta_y = data;

                packet_size = 0;

                mouse_print_packet(&packet_data);

                check_event(&packet_data, &event);

                struct mouse_ev *event_ptr = &event;

                switch (state) {
                  case INIT:
                    if (event_ptr->type == LB_PRESSED)
                      state = LMB_PRESSED;
                    else {
                      length_x = 0;
                      length_y = 0;
                    }
                    break;
                  case LMB_PRESSED:
                    if (event_ptr->type == LB_RELEASED && length_x >= x_len && length_y >= x_len) {
                      length_x = 0;
                      length_y = 0;
                      state = MOVE;
                    }
                    else if (event_ptr->type == MOUSE_MOV && !x_sign && !y_sign) {
                      length_x += event_ptr->delta_x;
                      length_y += event_ptr->delta_y;
                    }
                    else if (event_ptr->type == MOUSE_MOV && abs(event_ptr->delta_x) <= tolerance && abs(event_ptr->delta_x) <= tolerance) {
                      state = LMB_PRESSED;
                    }
                    else
                      state = INIT;
                    break;
                  case MOVE:
                    if (event_ptr->type == RB_PRESSED && abs(event_ptr->delta_x) <= tolerance && abs(event_ptr->delta_x) <= tolerance) {
                      length_x = 0;
                      length_y = 0;
                      state = RMB_PRESSED;
                    }
                    else if (event_ptr->type == MOUSE_MOV && abs(event_ptr->delta_x) <= tolerance && abs(event_ptr->delta_x) <= tolerance)
                      state = MOVE;
                    else if (event_ptr->type == LB_PRESSED){
                      length_x = 0;
                      length_y = 0;
                      state = LMB_PRESSED;
                    }
                    else
                      state = INIT;
                    break;
                  case RMB_PRESSED:
                    if (event_ptr->type == RB_RELEASED && length_x >= x_len && abs(length_y) >= x_len) {
                      length_x = 0;
                      length_y = 0;
                      state = FINAL;
                    }
                    else if (event_ptr->type == MOUSE_MOV && !x_sign && y_sign) {
                      length_x += event_ptr->delta_x;
                      length_y += event_ptr->delta_y;
                    }
                    else if (event_ptr->type == MOUSE_MOV && abs(event_ptr->delta_x) <= tolerance && abs(event_ptr->delta_x) <= tolerance) {
                      state = RMB_PRESSED;
                    }
                    else
                      state = INIT;
                    break;
                  case FINAL:
                    break;
                }
              }
            }
          }
        default:
          break;
      }
    }
    else {
      continue;
    }
  }

  sys_irqdisable(&hook_id_mouse);
  mouse_cmd(DISABLE_DT_RPT);
  sys_irqenable(&hook_id_mouse);

  mouse_unsubscribe_int(&hook_id_mouse);

  return 0;
}
