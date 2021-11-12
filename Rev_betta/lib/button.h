#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "stm8s.h"

typedef struct pin_desc
{
	GPIO_TypeDef* GPIOx;
	uint8_t GPIO_Pin_x;
}sPIN_CFG;

void    Button_config (void);
uint8_t Read_button   (void);
#endif
