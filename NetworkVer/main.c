#include "envir.h"
#include "interface.h"

/*GPIO macros*/
#define LED_OFF()            GPIO_WriteLow(GPIOC, GPIO_PIN_7)
#define LED_ON()            GPIO_WriteHigh(GPIOC, GPIO_PIN_7)

/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t TimingDelay = 0;
static uint8_t curPwmVal;
static uint8_t adcData;
sSTEP_CONF step_conf;

uint8_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max)
{
    if(x < in_min) return out_min;
    else if(x > in_max) return out_max;
    else return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void ADC_Config()
{
  /*  Init GPIO for ADC2 */
  GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_FL_NO_IT);
  
  /* De-Init ADC peripheral*/
  ADC1_DeInit();

  /* Init ADC2 peripheral */
  ADC1_Init(ADC1_CONVERSIONMODE_CONTINUOUS, ADC1_CHANNEL_4, ADC1_PRESSEL_FCPU_D8, \
            ADC1_EXTTRIG_TIM, DISABLE, ADC1_ALIGN_RIGHT, ADC1_SCHMITTTRIG_CHANNEL4,\
            DISABLE);

  /* Enable EOC interrupt */
  //ADC1_ITConfig(ADC1_IT_EOCIE, ENABLE);
  
  ADC1_Cmd(ENABLE);
  /*Start Conversion */
  ADC1_StartConversion();
}

int init_hw()
{
    //Инициализируем CLK
  CLK->ECKR|=CLK_ECKR_HSEEN; //Разрешаем работу генератора с внешним кварцем (HSEEN)
  CLK->SWCR|=CLK_SWCR_SWEN;  //Разрешаем автопереключение источника Clock при неисправности генератора
  CLK->SWR=0xB4;             //Включаем clock от кварцевого генератора (HSE)
  CLK->CKDIVR=0;             //Делители частоты внутреннего и внешнего генератора на 1 - частота ядра максимальная
   
  TIM4_Config();
  TIM2_Config(); 
  
  GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUT_PP_LOW_SLOW); 
  
  i2c_init(1+SL_ADDRES*2);
  
#ifdef ADC_EN
  ADC_Config();
#endif
  
  adcData = 0;
  return 0;
}


/**
  * @brief  Timer2 Capture/Compare Interrupt routine
  * @param  None
  * @retval None
  */
 INTERRUPT_HANDLER(TIM2_CAP_COM_IRQHandler, 14)
{
  
  if (TIM2_GetITStatus(TIM2_IT_CC1) != RESET)
  {
    curPwmVal++;
    if(step_conf.pwmValue == 0)
      LED_OFF();
    else
    {
      if(curPwmVal > step_conf.pwmValue)
        LED_OFF();
      else
        LED_ON();
    }
    /* Clear TIM2 Capture Compare1 interrupt pending bit*/
    
    TIM2_SetCompare1(50);

    TIM2->CNTRH = 0x00;
    TIM2->CNTRL = 0x00;
    TIM2_ClearITPendingBit(TIM2_IT_CC1);
  }
  
}

int main()
{
  uint16_t Conversion_Value;
  uint8_t adcData = 0;
  step_conf.max_bright   = MAX_BRIGHT;
  step_conf.s2s_delay    = STEP_TIME; //mSeconds
  step_conf.state        = ST_IDLE;
  step_conf.pwmValue     = 0;
  init_hw();
  
  enableInterrupts();
  
  TIM2_Cmd(ENABLE);
  
  while(1)
  { 
#ifdef ADC_EN
    //Conversion_Value = ADC1_GetConversionValue();
    adcData = map(ADC1_GetConversionValue(), 0, 1023, 0, 255);
#endif
    i2c_slave_sendData(&adcData, 1);
    
    if(step_conf.state == ST_LED_ON)
    {
      Set_Time (step_conf.s2s_delay/step_conf.max_bright);
      while(Read_Time()!=0);
      if(step_conf.pwmValue < step_conf.max_bright)
        step_conf.pwmValue++;
      else
        step_conf.state = ST_IDLE;
    }
    else if(step_conf.state == ST_LED_OFF)
    {
      Set_Time (step_conf.s2s_delay/step_conf.max_bright);
      while(Read_Time()!=0);
      if(step_conf.pwmValue != 0)
        step_conf.pwmValue--;
      else
        step_conf.state = ST_IDLE;
    }
    
    
  }
  return 0;
}
 