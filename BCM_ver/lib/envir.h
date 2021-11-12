#ifndef _ENVIR_H_
#define _ENVIR_H_

#include "stm8s.h"
#include "delay_timer.h"
#include "button.h"
#include "bcm.h"
#define MAX_BRIGHT 150

#define ST_IDLE     0
#define ST_BUSY     1

#define DIR_UP   ((uint8_t)0)
#define DIR_DWN  ((uint8_t)1)
#define DIR_BOTH ((uint8_t)2)

#define ST_LSB ADC1_CHANNEL_0
#define ST_MSB ADC1_CHANNEL_1
#define BRIGHT ADC1_CHANNEL_2

#define MENU_WRC 0 //Worck state
#define MENU_BRT 1 //Set bright
#define MENU_SSD 2 //Set step to step delay
#define MENU_TME 3 //Set worc time



typedef struct stairs
{
  uint8_t steps_number;
  uint8_t max_bright;
  uint8_t min_bright;
  uint16_t s2s_delay;
  uint8_t work_time;
  uint8_t ch_index;
  uint8_t up_index;
  uint8_t down_index;
  uint8_t pwmValue;
  uint8_t state;
  uint16_t rangeTrashold;
} sSTAIRS_CONF;



#endif