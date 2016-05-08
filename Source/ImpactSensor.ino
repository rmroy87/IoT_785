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
#include "TempSensorTask.h"
#include "ImpactSensor.h"
#include "LedControlTask.h"
#include "MotionDetectTask.h"




// Cabling for i2c using Sparkfun breakout with a Spar Core
// Spark Core <-> Breakout board
// Gnd         -  GND
// 3.3v        -  VCC
// 3.3v        -  CS
// Digital 0   -  SDA
// Digital 1   -  SCL

// Setup the LED Stuff

int TEMP_SENSOR = D6; // One-wire DS18B20

float Init_Threshold = 80.0;

//OneWire ds = OneWire(D6);  // 1-wire signal on pin D4
// Setup the Accelerometer Device

//#define DEVICE (0xA6) // Device address as specified in data sheet
void MagnitudeThresholdEventHandler(short currMagnitude);

int analogvalue = 0;
double tempC = 0;

int helloWorldFromCloud(String command);
int CommandControlFromCloud(String command);


int helloWorldFromCloud(String command)
{
  Serial.print("*** Hello World - CMD: ");
  Serial.println(command);
  Serial.print("*** Hello World - Recv Ticks: ");
  Serial.println(millis());
}

int CommandControlFromCloud(String command)
{
  Serial.print("*** Command Control - CMD: ");
  Serial.print(command);
  Serial.println(" ***");
  Particle.publish("twilio", "RED State Hit" );
}

//
// Program Control Data Structure
ProgramCntrl progCntrl;
TempSensorTask ts  = TempSensorTask();
LedControlTask led = LedControlTask(BLUE, LED_ON);
MotionDetectTask motion = MotionDetectTask((uint8_t) 1,
                                           (uint32_t) 30,
                                           (short) 50,
                                           MagnitudeThresholdEventHandler);

void setup()
{
  ts.TempSensorInit(TEMP_SENSOR, Init_Threshold);

  Particle.function("hello", helloWorldFromCloud);
  Particle.function("CmdCntrl", CommandControlFromCloud);

  Serial.begin(57600);  // start serial for output. Make sure you set your Serial Monitor to the same!
  Serial.print("init");

}


void loop()
{

    static short last_mag = 0;
    static short max_mag = 0;
    static int count = 0;
    static uint8_t ledIndex = OFF;
    static uint8_t ledMode = LED_OFF;
    static uint32_t currTickCount;
    static uint32_t lastTickCount;
    uint32_t ticksIn;
    uint32_t ticksOut;

    char tempArray[20];

    currTickCount = millis();

    if((currTickCount - lastTickCount) > 1000){
      lastTickCount = currTickCount;

      led.SetLed(ledIndex, ledMode);

#if 0
      //mag_change = testMagnitudeAccel(&curr_x, &curr_y, &curr_z);

      if(mag_change > 20){
          Serial.print("Mag Hit: ");
          Serial.print( mag_change );
          Serial.print(" - Count: ");
          Serial.println( count );
      }

      if(mag_change > max_mag){
          max_mag = mag_change;
      }
#endif
      if(count >= 10){
          //Serial.print("Max Mag Hit: ");
          //Serial.println( max_mag );
          count = 0;

          sprintf(tempArray,"%f",
          ts.GetLastTemp(1));
          Serial.print("Librato-Publish: ");
          Serial.println( tempArray );
          //Particle.publish("temperature", tempArray);

          ticksIn = millis();

          Particle.publish("librato_A0", tempArray);
          ticksOut = millis();

          sprintf(tempArray,"TI=%d, TO=%d", ticksIn, ticksOut);
          Serial.println( tempArray );
      }
      count = count + 1;

/*
    Serial.print(" X = ");
    Serial.print(curr_x);
    Serial.print(" Y = ");
    Serial.print(curr_y);
    Serial.print(" Z = ");
    Serial.println(curr_z);
    delay(500); // only read every 0,5 seconds
    */


      ledIndex = ledIndex + 1;
      if(ledIndex >= WHITE){
        ledIndex = OFF;

        ledMode += 1;
        if(ledMode > LED_BLINK_FAST){
            ledMode = LED_OFF;
        }
      }
    }
    //writeRGB(0xff, 0xff, 0xff);
    //delay(500); // only read every 0,5 seconds

    ts.TempSensorTaskHandler();
    led.LedControlTaskHandler();
    motion.MotionDetectTaskHandler();
}

//
// Event Handler for the Max Threshold
void MagnitudeThresholdEventHandler(short currMagnitude)
{
  Serial.print("*** Magnitude Thershold Hit: ");
  Serial.print(currMagnitude);
  Serial.print(" Ticks: ");
  Serial.print(millis());
  Serial.println(" ***");
}
