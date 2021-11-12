#ifndef _ENVIR_H_
#define _ENVIR_H_

#include "stm8s.h"

#define MAX_BRIGHT 150

typedef struct stairs
{
  uint8_t steps_number;
  uint8_t step_mask[256];
  uint8_t step_buff[256];
  uint8_t step_buff_length;
  uint8_t cur_step;
  uint8_t max_bright;
  uint8_t cur_bright;
  uint8_t min_bright;
  uint8_t step_pwm_bright;
  uint8_t s2s_delay;
  uint8_t work_time;
} sSTAIRS_CONF;



#endif