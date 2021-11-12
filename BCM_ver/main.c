#include "envir.h"

#define CS_PORT GPIOE
#define CS_PIN GPIO_PIN_3
/*GPIO macros*/
#define CSN_LO()            GPIO_WriteLow(CS_PORT, CS_PIN)
#define CSN_HI()            GPIO_WriteHigh(CS_PORT, CS_PIN)

#ifdef _RAISONANCE_
#define PUTCHAR_PROTOTYPE int putchar (char c)
#define GETCHAR_PROTOTYPE int getchar (void)
#elif defined (_COSMIC_)
#define PUTCHAR_PROTOTYPE char putchar (char c)
#define GETCHAR_PROTOTYPE char getchar (void)
#else /* _IAR_ */
#define PUTCHAR_PROTOTYPE int putchar (int c)
#define GETCHAR_PROTOTYPE int getchar (void)
#endif /* _RAISONANCE_ */

/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t TimingDelay = 0;
sSTAIRS_CONF stairs_conf;
uint8_t bam_buffer[8][2];
static uint16_t bam_step = 0;

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
  
  GPIO_Init(GPIOB,GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2,GPIO_MODE_IN_FL_NO_IT);     // set IO mode on port B
  ADC1_DeInit();
  
  ADC1_DataBufferCmd(ENABLE);		// enable buffer
  ADC1_ScanModeCmd(ENABLE);		// enable scan mode
  ADC1_ITConfig(ADC1_IT_EOCIE,DISABLE);   // disable interrupt when using scan mode


// scan to channel that's need , this code is scan from ch0 to ch2
  ADC1_Init(ADC1_CONVERSIONMODE_CONTINUOUS,ADC1_CHANNEL_2,ADC1_PRESSEL_FCPU_D18,\
				ADC1_EXTTRIG_TIM     ,DISABLE,ADC1_ALIGN_RIGHT,\
				ADC1_SCHMITTTRIG_CHANNEL2  ,DISABLE);  				

  ADC1_StartConversion(); 

  UART2_Init((uint32_t)9600, UART2_WORDLENGTH_8D, UART2_STOPBITS_1, UART2_PARITY_NO,
              UART2_SYNCMODE_CLOCK_ENABLE, UART2_MODE_TXRX_ENABLE);
  
  
  return 0;
}


void spi_write_buff (uint8_t *buffer, uint8_t length)
{
  uint8_t inv_buf_data;
 
  CSN_LO();
  for(uint8_t i=length; i > 0 ; i--)
  {
    inv_buf_data = ~buffer[i-1];
    while (SPI_GetFlagStatus(SPI_FLAG_TXE)== RESET);
    SPI->DR = inv_buf_data;
    while (SPI_GetFlagStatus(SPI_FLAG_BSY)== SET);
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
  
  if (TIM2_GetITStatus(TIM2_IT_CC1) != RESET)
  {
    /* Clear TIM2 Capture Compare1 interrupt pending bit*/
    
    TIM2_SetCompare1(1<<bam_step);
    
    spi_write_buff(bam_buffer[bam_step], 2);

    bam_step = (bam_step+1)%8;
    TIM2->CNTRH = 0x00;
    TIM2->CNTRL = 0x00;
    TIM2_ClearITPendingBit(TIM2_IT_CC1);
  }
  
}

/*
* @brief Stairs state run
* @param State select
* @retval None
*/
void turn_on_led(uint8_t direction)
{
  stairs_conf.state = ST_BUSY;
  
  Set_Time (stairs_conf.s2s_delay/stairs_conf.max_bright);
    while(Read_Time()!=0);
    
    stairs_conf.pwmValue=(stairs_conf.pwmValue+1)%stairs_conf.max_bright;
    if(stairs_conf.pwmValue == stairs_conf.max_bright-1)
    {
      switch(direction)
      {
      case DIR_UP://Up
          if(stairs_conf.up_index != stairs_conf.steps_number-1)
          {
            stairs_conf.ch_index = ++stairs_conf.up_index;
          }
          else
          {
            stairs_conf.state = ST_IDLE;
          }
          
        break;
      
      case DIR_DWN://Down
          if(stairs_conf.down_index != 0)
          {
            stairs_conf.ch_index = --stairs_conf.down_index;
          }
          else
          {
            stairs_conf.state = ST_IDLE;
          }
          
        break;
      }
    }
    set_bam_data(stairs_conf.ch_index, stairs_conf.pwmValue);
}

void turn_off_led(uint8_t direction)
{
  stairs_conf.state = ST_BUSY;

  Set_Time (stairs_conf.s2s_delay/stairs_conf.max_bright);
  while(Read_Time()!=0);
    
    stairs_conf.pwmValue=(stairs_conf.pwmValue+1)%stairs_conf.max_bright;
    if(stairs_conf.pwmValue == stairs_conf.max_bright-1)
    {
      switch(direction)
      {
      case DIR_UP://Up
          if(stairs_conf.up_index != stairs_conf.steps_number-1)
          {
            stairs_conf.ch_index = ++stairs_conf.up_index;
          }
          else
          {
            stairs_conf.state = ST_IDLE;
          }
        break;
      
      case DIR_DWN://Down
          if(stairs_conf.down_index != 0)
          {
            stairs_conf.ch_index = --stairs_conf.down_index;
          }
          else
          {
            stairs_conf.state = ST_IDLE;
          }
        break;
      }
    }
    set_bam_data(stairs_conf.ch_index, stairs_conf.max_bright-2 - stairs_conf.pwmValue);
}

void stairs_idle (uint8_t direction)
{
    stairs_conf.pwmValue = 0;
    stairs_conf.up_index = 0;
    stairs_conf.down_index = stairs_conf.steps_number - 1;
    if(direction == 0) 
      stairs_conf.ch_index = stairs_conf.up_index;
    else 
      stairs_conf.ch_index = stairs_conf.down_index;
}

int Motion_detect(uint16_t treshold, uint8_t* direction)
{
  uint16_t down_detect = 0;
  uint16_t up_detect = 0;
  
  down_detect = ADC1_GetBufferValue(0);
  up_detect = ADC1_GetBufferValue(1);
  
  if(down_detect > treshold || up_detect > treshold)
  {
    if(down_detect > treshold)
       *direction = DIR_DWN;
    else if(up_detect > treshold)
       *direction = DIR_UP;
    
    stairs_conf.state = ST_BUSY;
    stairs_idle (*direction);
    
    return 1;
  }
  
  return 0;  
}

int main()
{
  uint8_t direction = DIR_UP;
  uint8_t menu_index       = 0;
  stairs_conf.max_bright   = 50;
  stairs_conf.min_bright   = 10;
  stairs_conf.s2s_delay    = 500; //mSeconds
  stairs_conf.steps_number = 15;
  stairs_conf.work_time    = 4; //Seconds
  stairs_conf.state        = ST_IDLE;
  stairs_conf.rangeTrashold = 150;
  
  init_hw();
  
  clear_bam_data();
  
  spi_write_buff(bam_buffer[0], 2);
  set_bam_data(0, stairs_conf.min_bright);
  set_bam_data(stairs_conf.steps_number-1, stairs_conf.min_bright);
  stairs_idle (DIR_UP);
  TIM2_Cmd(ENABLE);
 
  while(1)
  {  
   if(Motion_detect(stairs_conf.rangeTrashold, &direction) != 0)
   {
     while(stairs_conf.state != ST_IDLE)
       turn_on_led(direction);
     
     stairs_idle (direction);
     stairs_conf.state = ST_BUSY;
          
     Set_Time(stairs_conf.work_time*500);
     while(Read_Time()!=0);
     
     while(stairs_conf.state != ST_IDLE)
       turn_off_led(direction);
     
     stairs_idle (direction);
     
     set_bam_data(0, stairs_conf.min_bright);
     set_bam_data(stairs_conf.steps_number-1, stairs_conf.min_bright);  
   }
  }
  return 0;
}
 
/**
  * @brief Retargets the C library printf function to the UART.
  * @param c Character to send
  * @retval char Character sent
  */
PUTCHAR_PROTOTYPE
{
  /* Write a character to the UART2 */
  UART2_SendData8(c);
  /* Loop until the end of transmission */
  while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);

  return (c);
}

/**
  * @brief Retargets the C library scanf function to the USART.
  * @param None
  * @retval char Character to Read
  */
GETCHAR_PROTOTYPE
{
#ifdef _COSMIC_
  char c = 0;
#else
  int c = 0;
#endif
  /* Loop until the Read data register flag is SET */
  while (UART2_GetFlagStatus(UART2_FLAG_RXNE) == RESET);
    c = UART2_ReceiveData8();
  return (c);
}