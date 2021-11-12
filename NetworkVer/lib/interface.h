#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "stm8s.h"
#include "envir.h"
void i2c_init(uint16_t ownAddres);
uint8_t i2c_getData();
void i2c_master_sendData(uint8_t address, uint8_t *data, unsigned char length);
void i2c_slave_sendData(uint8_t *data, uint8_t length);

#endif