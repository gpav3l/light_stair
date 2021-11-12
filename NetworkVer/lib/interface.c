#include "interface.h"

static struct sMasterDataPacket
{
  unsigned char buff[10];
  unsigned char buffLength;
  unsigned char address;
  bool masterMode;
} masterPacket;

static struct sSlaveDataPacket
{
  unsigned char buff[10];
  unsigned char buffLength;
}slavePacket;

/**
  * @brief  Parse PWM value for selected chanel in BAM array 
  * @param  Chanel number
  * @param  PWM value for parsing
  * @retval None
  */
void i2c_init(uint16_t ownAddres)
{
  I2C_DeInit();
   
  I2C_Init(100000, ownAddres, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);
  /* Enable Error Interrupt*/
  I2C_ITConfig((I2C_IT_TypeDef)(I2C_IT_ERR | I2C_IT_EVT | I2C_IT_BUF), ENABLE);

  I2C_Cmd(ENABLE);

  masterPacket.masterMode = FALSE;  
}

void i2c_master_sendData(uint8_t address, uint8_t *data, unsigned char length)
{
  masterPacket.masterMode = TRUE;
  masterPacket.address = address;
  masterPacket.buffLength = length;
  for(int i=0; i < masterPacket.buffLength; i++)
    masterPacket.buff[i]= data[i];
  
  I2C_AcknowledgeConfig(I2C_ACK_CURR);
  //while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY));
  I2C_GenerateSTART(ENABLE);
  
}

void i2c_slave_sendData(uint8_t *data, uint8_t length)
{
  slavePacket.buffLength = length;
  
   for(int i=0; i < slavePacket.buffLength; i++)
      slavePacket.buff[i]= data[i];
}
/**
  * @brief  I2C Interrupt routine
  * @param None
  * @retval
  * None
  */
INTERRUPT_HANDLER(I2C_IRQHandler, ITC_IRQ_I2C)
{
  static uint8_t rxData = 0x00;
  static uint8_t cfgMod = 0x00;
  static uint8_t state = 0;
  /* Read SR2 register to get I2C error */
  if ((I2C->SR2) != 0)
  {
    /* Clears SR2 register */
    I2C->SR2 = 0;
  }
  
  I2C_Event_TypeDef Event =  I2C_GetLastEvent();
  
  if(masterPacket.masterMode == TRUE)
  {
    switch(state)
    {
    case 0:
       if(I2C_GetFlagStatus(I2C_FLAG_STARTDETECTION))
       {
         I2C_Send7bitAddress(masterPacket.address, I2C_DIRECTION_TX);
         state++;
       }
       break;
       
    case 1:
       if(I2C_GetFlagStatus(I2C_FLAG_TRANSMITTERRECEIVER))
       {
         if(masterPacket.buffLength != 0)
            I2C_SendData(masterPacket.buff[--masterPacket.buffLength]);
         else
          state++;
       }
       else
       {
         state = 0;
         I2C_GenerateSTOP(ENABLE);
         masterPacket.masterMode = FALSE;
       }
       break;
       
    case 2:
       if(I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED))
       {
         I2C_GenerateSTOP(ENABLE);
          masterPacket.masterMode = FALSE;
       }
      break;
    }
    
  }
  else
  {
  
    if(I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) != RESET)
    {
      if(state == 0)
      {
        cfgMod = I2C_ReceiveData();
        state = 1;
      }
      else if(state == 1)
      {
        rxData = I2C_ReceiveData();
        state = 2;
      }
      
      if(state == 2)
      {
        state = 0;
        switch(cfgMod)
        {
          case 0x01: step_conf.max_bright = rxData; break;
          case 0x02: step_conf.s2s_delay = ((uint16_t)rxData)*100; break;
          case 0x03: step_conf.state = ( rxData >= 0x03) ? 0x00 : rxData; break;
        }
      }
    }
    if(I2C_GetFlagStatus(I2C_FLAG_STOPDETECTION)!= RESET)
       I2C->CR2 |= I2C_CR2_ACK;
    
    if(I2C_GetFlagStatus(I2C_FLAG_TXEMPTY))
      I2C_SendData(slavePacket.buff[0]);
  }
}