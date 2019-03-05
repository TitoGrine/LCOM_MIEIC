#include <lcom/lcf.h>

#include "mouse.h"


int hook_id_mouse = 1;
uint8_t data;
bool error_flag;

static bool right_button = false;
static bool left_button = false;


int (mouse_subscribe_int)(uint8_t *bit_no) {

  *bit_no = hook_id_mouse;
  sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_mouse);

  return 0;
}

int (mouse_unsubscribe_int)() {

  sys_irqrmpolicy(&hook_id_mouse);

  return 0;
}

void (mouse_ih)() {

  uint32_t status_register;

  sys_inb(STAT_REG, &status_register);

  if ((status_register & ST_PARITY) || (status_register & ST_TIMEOUT))
    error_flag = true;
  else
    error_flag = false;

  if (status_register & ST_OBF) {
    uint32_t output;

    sys_inb(OUT_BUF, &output);

    data = (uint8_t) output;
  }
}


int (kbc_cmd)(uint8_t command, uint8_t *arg_or_res, bool expect_res) {
  
  uint32_t status;
  uint32_t kbc_command = command;

  // issues a command to the KBC
  while ( 1 ) {
    sys_inb(STAT_REG, &status);

    if ( (status & ST_IBF) == 0 ) {
      sys_outb(KBC_CMD_REG, kbc_command);
      break;
    }
    tickdelay(micros_to_ticks(DELAY));
  }

  if ( expect_res ) {
    // if a result is expected
    // reads the output buffer to get the response to the command
    while ( 1 ) {
      sys_inb(STAT_REG, &status);

      if ( (status & ST_OBF) && !(status & (ST_PARITY | ST_TIMEOUT)) ) {
        sys_inb(OUT_BUF, &kbc_command);
        *arg_or_res = (uint8_t) kbc_command;
        break;
      }
      tickdelay(micros_to_ticks(DELAY));
    }

  }
  else {
    // if a result is not expected
    // it means the command has an argument
    kbc_command = *arg_or_res;

    sys_inb(STAT_REG, &status);

    sys_outb(OUT_BUF, kbc_command);
  }

  return 0;
}


int (mouse_cmd)(uint8_t command) {

  uint32_t status;

  do {
   
    kbc_cmd(WRITE_BYTE_MOUSE, &command, false);
    
    tickdelay(micros_to_ticks(DELAY));

    while ( 1 ) {
      sys_inb(STAT_REG, &status);

      if ( status & ST_OBF ) {
        sys_inb(OUT_BUF, &status);
        break;
      }
      tickdelay(micros_to_ticks(DELAY));
    }

  } while ( status != ACK );

  return 0;
}

int16_t (complement_to_decimal)(uint16_t binary) {
	
	uint8_t bit_no = 16;
	int16_t power = 1;	
	int16_t sum = 0;

  uint8_t i = 0;
	while ( i < bit_no ) {
    
    if ( i == bit_no - 1 && binary == 1)
      sum -= power;
    else
      sum += binary % 10 * power;

    binary /= 10;
    power *= 2;
    i++;
  }

  return sum;
}

void (check_event)(struct packet *data_packet, struct mouse_ev *event_ptr) {

  if (!left_button && data_packet->lb && !data_packet->rb && !data_packet->mb){
    event_ptr->type = LB_PRESSED;
    event_ptr->delta_x = complement_to_decimal(data_packet->delta_x);
    event_ptr->delta_y = complement_to_decimal(data_packet->delta_y);
  }
  else if (left_button && !data_packet->lb && !data_packet->rb && !data_packet->mb){              event_ptr->type = LB_RELEASED;
    event_ptr->delta_x = complement_to_decimal(data_packet->delta_x);
    event_ptr->delta_y = complement_to_decimal(data_packet->delta_y);
  }
  else if (!right_button && data_packet->rb && !data_packet->lb && !data_packet->mb){             event_ptr->type = RB_PRESSED;
    event_ptr->delta_x = complement_to_decimal(data_packet->delta_x);
    event_ptr->delta_y = complement_to_decimal(data_packet->delta_y);
  }
  else if (right_button && !data_packet->rb && !data_packet->lb && !data_packet->mb){             event_ptr->type = RB_RELEASED;
    event_ptr->delta_x = complement_to_decimal(data_packet->delta_x);
    event_ptr->delta_y = complement_to_decimal(data_packet->delta_y);
  }
  else if ((right_button == data_packet->rb) && (left_button == data_packet->lb) && !data_packet->mb){
    event_ptr->type = MOUSE_MOV;
    event_ptr->delta_x = complement_to_decimal(data_packet->delta_x);
    event_ptr->delta_y = complement_to_decimal(data_packet->delta_y);
  }
  else{
    event_ptr->type = BUTTON_EV;
    event_ptr->delta_x = complement_to_decimal(data_packet->delta_x);
    event_ptr->delta_y = complement_to_decimal(data_packet->delta_y);
  }

  left_button = data_packet->lb;
  right_button = data_packet->rb;

}

