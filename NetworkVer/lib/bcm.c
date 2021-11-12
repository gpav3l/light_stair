#include "bcm.h"

/**
  * @brief  Parse PWM value for selected chanel in BAM array 
  * @param  Chanel number
  * @param  PWM value for parsing
  * @retval None
  */
void set_bam_data(uint8_t channel_number, uint8_t pwmValue)
{
  uint8_t index_select=0;
  if(channel_number>7)
    index_select = 1;
  else
    index_select = 0;
  
  for(uint8_t i=0; i<8; i++)
  {
    if (ValBit(pwmValue, i))
	SetBit(bam_buffer[i][index_select], channel_number%8);
    else
	ClrBit(bam_buffer[i][index_select], channel_number%8);
  }
}


/**
  * @brief  Clear BAM array  
  * @param  None
  * @retval None
  */
void clear_bam_data()
{
  for(uint8_t i=0; i<8; i++)
  {
    for(uint8_t j=0; j<2; j++)
    {
      bam_buffer[i][j]=0x00;
    }
  }
}