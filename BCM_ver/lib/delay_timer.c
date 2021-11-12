#include "delay_timer.h"


INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23)
{
  TimingDelay_Decrement();
  /* Cleat Interrupt Pending bit */
  TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}
/**
  * @brief  Configure Output Compare Active Mode for TIM2 Channel1, Channel2 and 
  *         channel3  
  * @param  None
  * @retval None
  */
void TIM2_Config(void)
{
  
  /*TIM2_TimeBaseInit(TIM2_PRESCALER_256, 128);

  TIM2_PrescalerConfig(TIM2_PRESCALER_256, TIM2_PSCRELOADMODE_UPDATE);

  TIM2_OC1Init(TIM2_OCMODE_ACTIVE, TIM2_OUTPUTSTATE_ENABLE,128, TIM2_OCPOLARITY_HIGH);  
  TIM2_OC1PreloadConfig(ENABLE);

  TIM2_ARRPreloadConfig(ENABLE);
  
  TIM2_ITConfig(TIM2_IT_CC1, ENABLE); */
  TIM2->PSCR = 0x07;        // Делитель 256.
TIM2->ARRH = 0x00;        // старший байт числа 255
TIM2->ARRL = 0xFF;        // младший байт числа 255
TIM2->CCR1H = 0x00;       // старший байт числа 128 (50% шим)
TIM2->CCR1L = 0x80;        // сладший байт числа 128(50% шм)
TIM2->CCER1 = 0x00;
TIM2->CCMR1 = 0x00;
TIM2_ITConfig(TIM2_IT_CC1, ENABLE);
  /* enable interrupts */
  enableInterrupts();
}

void TIM4_Config(void)
{
  /* TIM4 configuration:
   - TIM4CLK is set to 10 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
   clock used is 10 MHz / 32 = 156 300 kHz
  - With 78 125 Hz we can generate time base:
  - In this example we need to generate a time base equal to 1 ms
   so TIM4_PERIOD = (0.001 * 156300000 - 1) = 155.25 */

  /* Time base configuration */
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 77);
  /* Clear TIM4 update flag */
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  /* Enable update interrupt */
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  
  /* enable interrupts */
  enableInterrupts();

  /* Enable TIM4 */
  TIM4_Cmd(ENABLE);
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Set_Time(__IO uint32_t nTime)
{
  TimingDelay = nTime;
}

uint32_t Read_Time()
{
  return TimingDelay;
}
/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
}
