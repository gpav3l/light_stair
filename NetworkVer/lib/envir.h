#ifndef _ENVIR_H_
#define _ENVIR_H_

#include "stm8s.h"
#include "delay_timer.h"
#include "bcm.h"

#define MAX_BRIGHT 100
#define STEP_TIME  500
#define SL_ADDRES  16

#define ADC_EN TRUE

#define ST_IDLE    0x00
#define ST_LED_ON  0x01
#define ST_LED_OFF 0x02

typedef struct step
{
  uint8_t max_bright;
  uint16_t s2s_delay;
  uint8_t pwmValue;
  uint8_t state;
} sSTEP_CONF;

extern sSTEP_CONF step_conf;


#endif