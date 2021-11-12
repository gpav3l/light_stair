#include "stm8s.h"
#include "envir.h"
#include "delay_timer.h"
#include "button.h"

#define CS_PORT GPIOE
#define CS_PIN GPIO_PIN_3
/*GPIO macros*/
#define CSN_LO()            GPIO_WriteLow(CS_PORT, CS_PIN)
#define CSN_HI()            GPIO_WriteHigh(CS_PORT, CS_PIN)


/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t TimingDelay = 0;
sSTAIRS_CONF stairs_conf;

/* Private function prototypes -----------------------------------------------*/

int init_hw()
{
    //Инициализируем CLK
  CLK->ECKR|=CLK_ECKR_HSEEN; //Разрешаем работу генератора с внешним кварцем (HSEEN)
  CLK->SWCR|=CLK_SWCR_SWEN;  //Разрешаем автопереключение источника Clock при неисправности генератора
  CLK->SWR=0xB4;             //Включаем clock от кварцевого генератора (HSE)
  CLK->CKDIVR=0;             //Делители частоты внутреннего и внешнего генератора на 1 - частота ядра максимальная
   
  GPIO_Init(CS_PORT, CS_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);
  CSN_HI();
  SPI_DeInit();
  SPI_Init(
      SPI_FIRSTBIT_MSB,
      SPI_BAUDRATEPRESCALER_2,
      SPI_MODE_MASTER,
      SPI_CLOCKPOLARITY_LOW,
      SPI_CLOCKPHASE_1EDGE,
      SPI_DATADIRECTION_2LINES_FULLDUPLEX,
      SPI_NSS_SOFT,
      (uint8_t)0x07
  );
  SPI_Cmd(ENABLE);
  
  TIM4_Config();
  TIM2_Config();
  
  Button_config();
  
  return 0;
}

void spi_write_byte(uint8_t w)
{
  while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET);
  SPI->DR = w;//SPI_SendData(w);
  while (SPI_GetFlagStatus(SPI_FLAG_BSY)== SET);
}

void spi_write_buff (uint8_t *buff, uint8_t length)
{
  uint8_t inv_buf_data;
 
  CSN_LO();
  for(uint8_t i=length; i > 0 ; i--)
  {
    inv_buf_data = ~buff[i-1];
    spi_write_byte(inv_buf_data);
  }
  CSN_HI();
}

/**
  * @brief  Timer2 Capture/Compare Interrupt routine
  * @param  None
  * @retval None
  */
 INTERRUPT_HANDLER(TIM2_CAP_COM_IRQHandler, 14)
{
  //Max bright IRQ
  if (TIM2_GetITStatus(TIM2_IT_CC1) != RESET)
  {
    /* Clear TIM2 Capture Compare1 interrupt pending bit*/
    TIM2_ClearITPendingBit(TIM2_IT_CC1);
    if(stairs_conf.max_bright != 255)
    {
      stairs_conf.step_buff[0] = 0x00;
      stairs_conf.step_buff[1] = 0x00;
    }
  }
  
  //Current bright value
  if (TIM2_GetITStatus(TIM2_IT_CC2) != RESET)
  {
    /* Clear TIM2 Capture Compare2 interrupt pending bit*/
    TIM2_ClearITPendingBit(TIM2_IT_CC2);
    ClrBit(stairs_conf.step_buff[stairs_conf.cur_step/8], (stairs_conf.cur_step%8)-1);
  }
  
  //Start PWM
  if (TIM2_GetITStatus(TIM2_IT_CC3) != RESET)
  {
    /* Clear TIM2 Capture Compare3 interrupt pending bit*/
    TIM2_ClearITPendingBit(TIM2_IT_CC3);
    TIM2->CNTRH = 0x00;
    TIM2->CNTRL = 0x00;
    if(stairs_conf.cur_bright != 0)
    {
      stairs_conf.step_buff[0] = stairs_conf.step_mask[0];
      stairs_conf.step_buff[1] = stairs_conf.step_mask[1];
    }
  }
  
  spi_write_buff(stairs_conf.step_buff, stairs_conf.step_buff_length);
}


int main()
{
  uint16_t temp;
  stairs_conf.max_bright      = 50;
  stairs_conf.cur_bright      = 1;
  stairs_conf.min_bright      = 10;
  stairs_conf.s2s_delay       = 4; //Seconds
  stairs_conf.steps_number    = 16;
  stairs_conf.work_time       = 1; //Seconds
  
  stairs_conf.step_pwm_bright = 5;
  
  if(stairs_conf.steps_number%8)
    stairs_conf.step_buff_length = stairs_conf.steps_number/8+1;
  else
    stairs_conf.step_buff_length = stairs_conf.steps_number/8;
  init_hw();
  
  TIM2_SetCompare1(stairs_conf.max_bright);
  TIM2_SetCompare2(stairs_conf.cur_bright);
  TIM2_SetCompare3(255);
  
  for(uint8_t i=0; i<stairs_conf.step_buff_length; i++)
  {
    stairs_conf.step_buff[i]=0x00;
    stairs_conf.step_mask[i]=0x00;
  }
  
  spi_write_buff(stairs_conf.step_buff, stairs_conf.step_buff_length);
  stairs_conf.step_mask[0] = 0x01;
  stairs_conf.cur_step = 1;
  
  /* TIM2 enable counter */
  TIM2_Cmd(ENABLE);
  
  
  while(1)
  {
    temp = stairs_conf.work_time*1000/stairs_conf.max_bright*2;
    Set_Time (temp);
    while(Read_Time()!=0);

    if(stairs_conf.cur_bright >= stairs_conf.max_bright)
    {
      stairs_conf.cur_bright=0x01;
      if(stairs_conf.cur_step==0x00 || stairs_conf.cur_step == stairs_conf.steps_number)
      {
        for(uint8_t i=0; i<stairs_conf.step_buff_length; i++)
        {
          stairs_conf.step_buff[i]=0x00;
          stairs_conf.step_mask[i]=0x00;
        }
        
        stairs_conf.step_mask[0] = 0x01;
        stairs_conf.cur_step = 1;
      }
      else
      {
        stairs_conf.step_mask[stairs_conf.cur_step/8]=stairs_conf.step_mask[stairs_conf.cur_step/8] | 1 << stairs_conf.cur_step%8;
        stairs_conf.cur_step++;
      }
    }
    else
      stairs_conf.cur_bright+=stairs_conf.step_pwm_bright;
    
    TIM2_SetCompare2(stairs_conf.cur_bright);
    
  }
  return 0;
}