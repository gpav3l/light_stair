
#include "step_control.hpp"

//Class constructor
cStepControl::cStepControl(int rSensor, int lSensor)
{
  Init(rSensor, lSensor);
}

cStepControl::cStepControl()
{
  
}

//Initialization
void cStepControl::Init(int rSensor, int lSensor)
{
  int error = 0;
  int stepCount=0;
  
  rangeSensor = rSensor;
  lightSensor = lSensor;
  
  Wire.begin(); // join i2c bus (address optional for master)
  Wire.setClock(1000L);
 
  for(int i=0; i < STEP_AMOUNT; i++ )
  {
    stepList[i] = 0xFF;
  }

  //Search step
  for(int i=0; (i < 127); i++)
  {
    Wire.beginTransmission(i);
    error =  Wire.endTransmission();
    if(error == 0)
    {
      if(stepCount < STEP_AMOUNT-1)
      {
        StepOn(i);
        stepList[stepCount++] = i;
      }
      else
      {
        i2cAdcAddress = STEP_AMOUNT;
        break;
      }
    }
  }

  delay(LIGHT_DELAY);
  
  for(int i=0; i < STEP_AMOUNT; i++ )
  {
    StepOff(stepList[i]);
  }
}

//Count step in list
int cStepControl::Count()
{
  int temp=0;
  
  for(;temp < STEP_AMOUNT; temp++)
  {
    if(stepList[temp] == 0xFF) return temp;
  }

  return STEP_AMOUNT;     
}

//Fsm work
void cStepControl::FsmWork()
{
  static long upTime = 0;
  static long downTime = 0;
  static int upIndex = STEP_AMOUNT*2;
  static int downIndex = 0;
  
  if(ADC_read(1)){ Serial.println("up triger"); upIndex = 0; }
  
  if(ADC_read(0)){ Serial.println("down triger"); downIndex = STEP_AMOUNT*2 - 1;}

  if(upIndex < STEP_AMOUNT)
  {
   
   StepOn(stepList[upIndex]);
  }
  else
  {
    StepOff(stepList[upIndex - (STEP_AMOUNT)]);
  }

  if(downIndex > STEP_AMOUNT)
  {
   if(downIndex == (STEP_AMOUNT*2 - 1))
   {
    StepOn(stepList[(downIndex-1) - (STEP_AMOUNT)]);
    StepOn(stepList[(downIndex-2) - (STEP_AMOUNT)]);
    StepOn(stepList[(downIndex-3) - (STEP_AMOUNT)]);
   }
   StepOn(stepList[downIndex - (STEP_AMOUNT)]);
  }
  else
  {
    StepOff(stepList[downIndex]);
  }
  
  if(upIndex != STEP_AMOUNT*2)
  {
    if((millis() - upTime) > S2S_DELAY)
    {
      upTime = millis();
      upIndex++;
    }
  }

  if(downIndex != 0)
  {
    if((millis() - downTime) > S2S_DELAY)
    {
      downTime = millis();
      downIndex--;
    }
  }
 
}
/*********Private part****************/
#define STEP_ON  0x01
#define STEP_OFF  0x02

void cStepControl::StepOn(int address)
{
  Wire.beginTransmission(address); // transmit step number
  Wire.write(0x03);              // Command control
  Wire.write(STEP_ON);              // State turn on
  Wire.endTransmission();    // stop transmitting
}

void cStepControl::StepOff(int address)
{
  Wire.beginTransmission(address); // transmit step number
  Wire.write(0x03);              // Command control
  Wire.write(STEP_OFF);          // State turn on
  Wire.endTransmission();    // stop transmitting
}

int cStepControl::ADC_read(int channel)
{
  static int upTriger = 25;
  static int downTriger = 25;
  int temp = 0;
 
  {
    switch(channel)
    {
      case 0:
      Wire.requestFrom(i2cAdcAddress, 1);    // request 1 bytes from slave device #16
      while (Wire.available()) 
      { // slave may send less than requested
        temp = Wire.read(); // receive a byte as character
      }
      downTriger = 9*downTriger + temp;
      downTriger /=10;
      if(downTriger < RANGE_TH_MIN || downTriger > RANGE_TH_MAX) return 1;
      else return 0;     
  
      case 1:
      temp = analogRead(rangeSensor); 
      temp = map(temp, 0, 1023, 0, 255);
      upTriger = 9*upTriger + temp;
      upTriger /=10;
      if(upTriger < RANGE_TH_MIN || upTriger > RANGE_TH_MAX) return 1;
      else return 0;
      
      case 2: temp = analogRead(lightSensor); break;
      default: temp = 0;
    }
  }
  return 0;
}

