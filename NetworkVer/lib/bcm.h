#ifndef _BCM_H_
#define _BCM_H_

#include "stm8s.h"

extern uint8_t bam_buffer[8][2];

void set_bam_data(uint8_t channel_number, uint8_t pwmValue);
void clear_bam_data();
#endif