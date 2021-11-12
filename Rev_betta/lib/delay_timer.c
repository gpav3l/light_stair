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
  /* Time base configuration */
  TIM2_TimeBaseInit(TIM2_PRESCALER_32, 155);

  /* Prescaler configuration */
 // TIM2_PrescalerConfig(TIM2_PRESCALER_32, TIM2_PSCRELOADMODE_IMMEDIATE);

  /* Output Compare Active Mode configuration: Channel1 */
  /*
	  TIM2_OCMode = TIM2_OCMODE_INACTIVE
       TIM2_OCPolarity = TIM2_OCPOLARITY_HIGH
       TIM2_Pulse = CCR1_Val
	*/
  TIM2_OC1Init(TIM2_OCMODE_INACTIVE, TIM2_OUTPUTSTATE_ENABLE,100, TIM2_OCPOLARITY_HIGH); 
  TIM2_OC1PreloadConfig(DISABLE);

  /* Output Compare Active Mode configuration: Channel2 */
  
  /*TIM2_Pulse = CCR2_Val;  */
  TIM2_OC2Init(TIM2_OCMODE_INACTIVE, TIM2_OUTPUTSTATE_ENABLE,200, TIM2_OCPOLARITY_HIGH); 
  TIM2_OC2PreloadConfig(DISABLE);

  /* Output Compare Active Mode configuration: Channel3 */
  /*TIM2_Pulse = CCR3_Val  */
  TIM2_OC3Init(TIM2_OCMODE_INACTIVE, TIM2_OUTPUTSTATE_ENABLE,300, TIM2_OCPOLARITY_HIGH); 
  TIM2_OC3PreloadConfig(DISABLE);

  TIM2_ARRPreloadConfig(ENABLE);
  
  /* TIM IT enable */
  TIM2_ITConfig(TIM2_IT_CC1, ENABLE);
  TIM2_ITConfig(TIM2_IT_CC2, ENABLE);
  TIM2_ITConfig(TIM2_IT_CC3, ENABLE);
  
  
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
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 155);
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
