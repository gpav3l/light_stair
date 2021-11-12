// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

#include "step_control.hpp"

cStepControl lightStairs;

void setup() {
  
  Serial.begin(9600);

  Serial.println("Init stairs");
  lightStairs.Init(A1, A0);
  Serial.print("Found ");
  Serial.print(lightStairs.Count(), DEC);
  Serial.println(" steps");
  
  delay(1500);
 
}

void loop() 
{
  lightStairs.FsmWork();
}
