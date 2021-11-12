#ifndef _STEP_CONTROL_H_
#define _STEP_CONTROL_H_
#include <arduino.h>
#include <Wire.h>

#define STEP_AMOUNT 16
#define S2S_DELAY 700
#define LIGHT_DELAY 1000
#define RANGE_TH_MAX 35
#define RANGE_TH_MIN 20

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
 
class cStepControl
{
  public:
   cStepControl(int rSensor, int lSensor);
   cStepControl();
   void Init(int rSensor, int lSensor);
   int Count();
   void FsmWork();
   void Up(int);
   void Down(int);
   
  private:
    byte stepList[STEP_AMOUNT];
    byte i2cAdcAddress;
    int rangeValue;
    int rangeSensor;
    int lightSensor;
    
    void StepOn(int);
    void StepOff(int);
    int ADC_read(int);
};



#endif
