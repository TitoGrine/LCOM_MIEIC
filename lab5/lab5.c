// IMPORTANT: you must include the following line in all your C files
#include <lcom/lab5.h>
#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "graphics.h"
#include "keyboard.h"
#include "sprite.h"

extern int g_counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  vg_init(mode);

  sleep(delay);

  vg_exit();

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  // Draw a rectangle //

  vg_init(mode);

  if (width == 0 || height == 0) color = BLACK;

  vg_draw_rectangle(x, y, width, height, color);

  // Read Keyboard Input //

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

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();

            if (!error_flag) {

              if (data == 0xE0) {
                scancode[0] = data;
                size_scancode = 2;
              }
              else {
                if (size_scancode == 2) {
                  scancode[1] = data;
                }
                else {
                  scancode[0] = data;
                }
                size_scancode = 1;
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

  kbd_unsubscribe_int();

  vg_exit();

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  // Draw Pattern //

  vg_init(mode);


  if (no_rectangles != 0)
    draw_pattern(no_rectangles, first, step);

  // Keyboard Input control //

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

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();

            if (!error_flag) {

              if (data == 0xE0) {
                scancode[0] = data;
                size_scancode = 2;
              }
              else {
                if (size_scancode == 2) {
                  scancode[1] = data;
                }
                else {
                  scancode[0] = data;
                }
                size_scancode = 1;
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

  kbd_unsubscribe_int();

  vg_exit();

  return 0;
}

int(video_test_xpm)(const char *xpm[], uint16_t x, uint16_t y) {

  // Initializes in mode 105

  vg_init(MODE_105);

  // Print PIXMAP

  int width, height;
  char *map;

  // get the pix map from the XPM
  map = read_xpm(xpm, &width, &height);

  draw_pixmap(map, x, y, width, height);
  // Read Keyboard Input //

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

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();

            if (!error_flag) {

              if (data == 0xE0) {
                scancode[0] = data;
                size_scancode = 2;
              }
              else {
                if (size_scancode == 2) {
                  scancode[1] = data;
                }
                else {
                  scancode[0] = data;
                }
                size_scancode = 1;
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

  kbd_unsubscribe_int();

  vg_exit();

  return 0;
}

int(video_test_move)(const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate) {

  vg_init(MODE_105);

  int movement;

  uint8_t frames = 60/fr_rate;

  uint32_t frame_counter = 0;
  uint32_t counter = 0;

  bool negative_speed = false;
  bool backwards = false;
  bool horizontal = true;
  Sprite *sp;

  // Calculate both axis speed //

  if (speed < 0) {
    negative_speed = true;
    movement = 1;
  }
  else
    movement = speed;

  if (xi == xf) {
    horizontal = false;

    if (yi > yf) {
      movement = -movement;
      backwards = true;
    }

    sp = create_sprite(xpm, xi, yi, 0, movement);
  }
  else {
    horizontal = true;

    if (xi > xf) {
      movement = -movement;
      backwards = true;
    }

    sp = create_sprite(xpm, xi, yi, movement, 0);
  }

  draw_Sprite(sp);

  // Periferals //

  uint8_t size_scancode = 1;
  uint8_t scancode[2] = {0, 0};

  uint8_t bit_no_keyboard;
  uint8_t bit_no_timer;

  int ipc_status, r;
  message msg;

  timer_subscribe_int(&bit_no_timer);
  kbd_subscribe_int(&bit_no_keyboard);

  uint8_t irq_set_timer = 0 | BIT(bit_no_timer);
  uint8_t irq_set_kbd = 0 | BIT(bit_no_keyboard);

  while (!(scancode[0] == ESC_BREAK)) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {

        case HARDWARE: /* hardware interrupt notification */

          if (msg.m_notify.interrupts & irq_set_timer) {
            counter++;

            if (counter % frames == 0) {

              if (negative_speed) {
                frame_counter++;

                if (frame_counter % -speed == 0) {

                  if (horizontal) {
                    if (sp->x != xf)
                      move_xpm(sp, movement);
                  }
                  else if (!horizontal) {
                    if (sp->y != yf)
                      move_xpm(sp, movement);
                  }
                }
              }
              else {
                if (!backwards && horizontal) {
                  if (sp->x + movement > xf){
                    move_xpm(sp, xf - sp->x);
                  }
                  else
                    move_xpm(sp, movement);
                }
                else if (!backwards && !horizontal) {
                  if (sp->y + movement > yf)
                    move_xpm(sp, yf - sp->y);
                  else
                    move_xpm(sp, movement);
                }
                else if (backwards && horizontal) {
                  if (sp->x + movement < xf)
                    move_xpm(sp, xf - sp->x);
                  else
                    move_xpm(sp, movement);
                }
                else if (backwards && !horizontal) {
                  if (sp->y + movement < yf)
                    move_xpm(sp, yf - sp->y);
                  else
                    move_xpm(sp, movement);
                }
              }
            }
          }

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
                }
                else {
                  scancode[0] = data;
                }
                size_scancode = 1;
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

  kbd_unsubscribe_int();
  timer_unsubscribe_int();

  destroy_sprite(sp);
  vg_exit();

  return 0;
}

int(video_test_controller)() {


  mmap_t m;
	struct reg86u r;

  memset(&r, 0, sizeof(r)); /* zero the structure */

  void *base_address = lm_init(false);

  if (base_address == NULL) {
    return -1;
  }

  uint32_t res = (uint32_t) base_address;

	if (lm_alloc(sizeof(vbe_info_t), &m) == NULL)
    return -1;

  
  vbe_info_t* vbe_info = (vbe_info_t*) m.virt;

  vbe_info->vbe_signature[0] = 'V';
  vbe_info->vbe_signature[1] = 'B';
  vbe_info->vbe_signature[2] = 'E';
  vbe_info->vbe_signature[3] = '2';

  

	r.u.w.ax = 0x4F00; /* VBE get mode info */
	/* translate the buffer linear address to a far pointer */
	r.u.w.es = PB2BASE(m.phys); /* set a segment base */
	r.u.w.di = PB2OFF(m.phys); /* set the offset accordingly */
	r.u.b.intno = 0x10;

  if(sys_int86(&r) != OK)
    return -1;

  if(r.u.b.ah != OK)
    return -1;

  vg_vbe_contr_info_t info;
  

  info.VBESignature[0] = vbe_info->vbe_signature[0];
  info.VBESignature[1] = vbe_info->vbe_signature[1];
  info.VBESignature[2] = vbe_info->vbe_signature[2];
  info.VBESignature[3] = vbe_info->vbe_signature[3];

  info.VBEVersion[0] = vbe_info->vbe_version[0];
  info.VBEVersion[1] = vbe_info->vbe_version[1];
  
  info.OEMString = (char*) (res + PB2BASE(vbe_info->oem_string_ptr) + PB2OFF(vbe_info->oem_string_ptr));

  info.VideoModeList = (uint16_t*) (res + PB2BASE(vbe_info->mode_list_ptr) + PB2OFF(vbe_info->mode_list_ptr));
  
  info.TotalMemory = (uint32_t) vbe_info->total_memory * 64;

  info.OEMVendorNamePtr = (char*) (res + PB2BASE(vbe_info->oem_vendor_name_ptr) + PB2OFF(vbe_info->oem_vendor_name_ptr));

  info.OEMProductNamePtr = (char*) (res + PB2BASE(vbe_info->oem_product_name_ptr) + PB2OFF(vbe_info->oem_product_name_ptr));

  info.OEMProductRevPtr = (char*) (res + PB2BASE(vbe_info->oem_product_rev_ptr) + PB2OFF(vbe_info->oem_product_rev_ptr));


  if ( ! lm_free(&m) )
    return -1;

  vg_display_vbe_contr_info(&info);
  

  return 0;
}
