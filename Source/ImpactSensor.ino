/*****************************************************************************************
 *                                                                                       *
 * ADXL345 Driver for Spark Core                                                         *
 * Original code from: https://github.com/jenschr/Arduino-libraries/tree/master/ADXL345  *
 *                                                                                       *
 * This program is free software; you can redistribute it and/or modify                  *
 * it under the terms of the GNU License.                                                *
 * This program is distributed in the hope that it will be useful,                       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                         *
 * GNU License V2 for more details.                                                      *
 *                                                                                       *
 * Bare bones ADXL345 i2c example for Arduino 1.0                                        *
 * by Jens C Brynildsen <http://www.flashgamer.com>                                      *
 * This version is not reliant of any external lib                                       *
 * (Adapted for Arduino 1.0 from http://code.google.com/p/adxl345driver)                 *
 *                                                                                       *
 * Demonstrates use of ADXL345 (using the Sparkfun ADXL345 breakout)                     *
 * with i2c communication. Datasheet:                                                    *
 * http://www.sparkfun.com/datasheets/Sensors/Accelerometer/ADXL345.pdf                  *
 * If you need more advanced features such as freefall and tap                           *
 * detection, check out:                                                                 *
 * https://github.com/jenschr/Arduino-libraries                                          *
 *                                                                                       *
 * Updated on: 25 April 2014                                                             *
 ****************************************************************************************/
#include "math.h"
//#include "Particle-OneWire/Particle-OneWire.h"
#include "ImpactSensorControl.h"

int analogvalue = 0;
double tempC = 0;

int helloWorldFromCloud(String command);
int CommandControlFromCloud(String command);
int ClearPlayerFromCloud(String command);
int RemovePlayerFromCloud(String command);

//
// Program Control Data Structure
//ProgramCntrl progCntrl;

ImpactSensorCntrol cntrl = ImpactSensorCntrol();

int helloWorldFromCloud(String command)
{
  Serial.print("*** Hello World - CMD: ");
  Serial.println(command);
  Serial.print("*** Hello World - Recv Ticks: ");
  Serial.println(millis());
}

//
// Events from the Cload or The medical Staff
int ClearPlayerFromCloud(String command)
{
  Serial.print("*** Command Control - CMD: ");
  Serial.print(command);
  Serial.println(" ***");
  Particle.publish("MedicalEvent", "Player Cleared to Return");
  //Particle.publish("twilio", "RED State Hit" );
  cntrl.InsertPlayerClearedEvent();
}

//
// Events from the Cload or The medical Staff
int RemovePlayerFromCloud(String command)
{
  Serial.print("*** Command Control - CMD: ");
  Serial.print(command);
  Serial.println(" ***");
  Particle.publish("MedicalEvent", "Player Removed from Play");
  //Particle.publish("twilio", "RED State Hit" );
  cntrl.InsertPlayerOutEvent();
}




void setup()
{


  Particle.function("hello", helloWorldFromCloud);
  Particle.function("ClearPlayer", ClearPlayerFromCloud);
  Particle.function("RemovePlayer", RemovePlayerFromCloud);

  Serial.begin(57600);  // start serial for output. Make sure you set your Serial Monitor to the same!
  Serial.print("init");

}


void loop()
{
  cntrl.RunImpactSensorControl();
}
