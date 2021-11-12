// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

#define STEP_AMOUNT 16
#define S2S_DELAY 700
#define LIGHT_DELAY 1000

//#define ADC_DEBUG

#define RANGE_TH_MAX 35
#define RANGE_TH_MIN 20
#include <Wire.h>
byte stepList [STEP_AMOUNT];
int stepCount = 0;
int lastStepIndex=0;
int rangeValue=0;
int repeatValue = 0;
int lightValue = 0;
int rangeSenosr = A1;
int lightSensor = A0;
int i2cADCaddress;

#define STEP_ON  0x01
#define STEP_OFF  0x02

void StepState(byte address, byte state)
{
  if(state == STEP_ON || state == STEP_OFF)
  {
    Wire.beginTransmission(address); // transmit step number
    Wire.write(0x03);              // Command control
    Wire.write(state);              // State turn on
    Wire.endTransmission();    // stop transmitting
  }
}

void setup() {
  int error = 0;
  
  Serial.begin(9600);
  Wire.begin(); // join i2c bus (address optional for master)
  Wire.setClock(1000L);

  delay(1500);
  Serial.println("Stairs start scan");
  for(int i=0; (i < 127) && (stepCount < STEP_AMOUNT); i++)
  {
    Serial.print("Step scan ");
    Serial.println(i, DEC);
    Wire.beginTransmission(i);
    error =  Wire.endTransmission();
    if(error == 0)
    {
      StepState(i, STEP_ON);
      lastStepIndex = stepCount;
      stepList[stepCount++] = i;
    }
  }
  
  i2cADCaddress = stepCount-1;
  
  for(int i=stepCount; i < STEP_AMOUNT; i++ )
  {
    stepList[i] = 0x00;
  }
  
  Serial.print("Found ");
  Serial.print(stepCount, DEC);
  Serial.println(" steps");
  stepCount = 0;

  stepCount = 0;
    
  while(stepCount != i2cADCaddress+1)
  {
     rangeValue = ADC_read(1);
     ADC_read(0);
     ADC_read(2);
     delay(200);
     StepState(stepList[stepCount], STEP_OFF);
     stepCount++;
   }
  //StepState(stepList[i2cADCaddress-1], STEP_ON);
}

/* Turn on step
    Wire.beginTransmission(<stepNumber>); // transmit step number
    Wire.write(0x03);              // Command control
    Wire.write(0x01);              // State turn on
    Wire.endTransmission();    // stop transmitting
 * Turn off step
    Wire.beginTransmission(<stepNumber>); // transmit step number
    Wire.write(0x03);              // Command control
    Wire.write(0x02);              // State turn on
    Wire.endTransmission();    // stop transmitting
 *  Read sensor Analog
    rangeValue = analogRead(rangeSenosr);
 *  Read sensor I2C   
    Wire.requestFrom(16, 1);    // request 1 bytes from slave device #16
    while (Wire.available()) 
    { // slave may send less than requested
      char c = Wire.read(); // receive a byte as character
      Serial.print(c);         // print the character
    }
 */

void refreshTerminal()
{
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");
}

int ADC_read(int channel)
{
  static int upTriger = 25;
  static int downTriger = 25;
  int temp = 0;
  for(int i=0; i< 5; i++)
  {
    switch(channel)
    {
      case 0:
      Wire.requestFrom(stepList[i2cADCaddress], 1);    // request 1 bytes from slave device #16
      while (Wire.available()) 
      { // slave may send less than requested
        temp = Wire.read(); // receive a byte as character
      }
      downTriger = 9*downTriger + temp;
      downTriger /=10;
      temp = downTriger;
      break;
  
      case 1:
      temp = analogRead(rangeSenosr); 
      temp = map(temp, 0, 1023, 0, 255);
      upTriger = 9*upTriger + temp;
      upTriger /=10;
      temp = upTriger;
      break;
      case 2: temp = analogRead(lightSensor); break;
      default: temp = 0;
    }
  }
  return temp;
}

void loop() 
{
  rangeValue = ADC_read(0);
  Serial.print("ADC I2C value: ");
  Serial.println(rangeValue, DEC);
#ifndef ADC_DEBUG  
  if(rangeValue < RANGE_TH_MIN || rangeValue > RANGE_TH_MAX)
  {
      stepCount = i2cADCaddress;
      StepState(stepList[stepCount-=1], STEP_ON);
      StepState(stepList[stepCount-=1], STEP_ON);
      StepState(stepList[stepCount-=1], STEP_ON);
       StepState(stepList[stepCount-=1], STEP_ON);
    while(stepCount != 0)
    {
      rangeValue = ADC_read(0);
      delay(S2S_DELAY);
      StepState(stepList[stepCount-1], STEP_ON);
      stepCount--;
    }
  
    delay(LIGHT_DELAY);
    stepCount = i2cADCaddress;
    
    while(stepCount != 0)
    {
      rangeValue = ADC_read(0);
      delay(S2S_DELAY);
      StepState(stepList[stepCount-1], STEP_OFF);
      stepCount--;
    }
  }
#endif
  
  rangeValue = ADC_read(1);
  Serial.print("ADC value: ");
  Serial.println(rangeValue, DEC);
  
#ifndef ADC_DEBUG
  if(rangeValue < RANGE_TH_MIN || rangeValue > RANGE_TH_MAX)
  {
      stepCount = 0;
      StepState(stepList[stepCount], STEP_ON);
      StepState(stepList[stepCount+=1], STEP_ON);
      StepState(stepList[stepCount+=1], STEP_ON);
   
    while(stepCount != i2cADCaddress)
    {
      rangeValue = ADC_read(1);
      delay(S2S_DELAY);
      StepState(stepList[stepCount+1], STEP_ON);
      stepCount++;
    }
  
    delay(LIGHT_DELAY);
    stepCount = 0;
    
    while(stepCount != i2cADCaddress)
    {
      rangeValue = ADC_read(1);
      delay(S2S_DELAY);
      StepState(stepList[stepCount+1], STEP_OFF);
      stepCount++;
    }
  }
#endif
}
