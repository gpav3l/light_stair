#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "stm8s.h"

#define BUTTON_ENT 0x01
#define BUTTON_I   0x04
#define BUTTON_D   0x02

typedef struct pin_desc
{
	GPIO_TypeDef* GPIOx;
	uint8_t GPIO_Pin_x;
}sPIN_CFG;

void    Button_config (void);
uint8_t Read_button   (void);
#endif
