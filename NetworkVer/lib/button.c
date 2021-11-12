#include "button.h"

sPIN_CFG button[3] = 
{
  GPIOC,
  GPIO_PIN_2,
  GPIOC,
  GPIO_PIN_1,
  GPIOE,
  GPIO_PIN_5,
};

/**
  * @brief  Configure pin as input 
  * @param  None
  * @retval None
  */
void Button_config()
{
  for(uint8_t i=0; i<3; i++)
  {
    GPIO_Init(button[i].GPIOx, button[i].GPIO_Pin_x, GPIO_MODE_IN_FL_NO_IT);
  }
  
}

/**
  * @brief  Read data from keyboard  
  * @param  None
  * @retval bit mask for pressed button
  */
uint8_t Read_button()
{
  uint8_t but_mask=0x00;
  for(uint8_t i=0; i<3; i++)
  {
    if(GPIO_ReadInputPin(button[i].GPIOx, button[i].GPIO_Pin_x))
    {
      SetBit(but_mask, i);
    }
  }
  return but_mask;
}