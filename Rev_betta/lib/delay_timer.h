#ifndef _DELAY_TIMER_H_
#define _DELAY_TIMER_H_
#include "stm8s.h"

extern __IO uint32_t TimingDelay;

void     TIM4_Config           (void);
void     TIM2_Config           (void);
void     Set_Time              (__IO uint32_t nTime);
uint32_t Read_Time             (void);
void     TimingDelay_Decrement (void);

#endif