/*
Use this sketch to read the temperature from 1-Wire devices
you have attached to your Particle device (core, p0, p1, photon, electron)

Temperature is read from: DS18S20, DS18B20, DS1822, DS2438

Expanding on the enumeration process in the address scanner, this example
reads the temperature and outputs it from known device types as it scans.

I/O setup:
These made it easy to just 'plug in' my 18B20

D3 - 1-wire ground, or just use regular pin and comment out below.
D4 - 1-wire signal, 2K-10K resistor to D5 (3v3)
D5 - 1-wire power, ditto ground comment.

A pull-up resistor is required on the signal line. The spec calls for a 4.7K.
I have used 1K-10K depending on the bus configuration and what I had out on the
bench. If you are powering the device, they all work. If you are using parisidic
power it gets more picky about the value.

*/

// Only include One of the following depending on your environment!
#include "ImpactSensor.h"
#include "OneWire.h" // Use this include for Particle Dev where everything is in one directory.
#include "TempSensorTask.h"

// Comment this out for normal operation
//SYSTEM_MODE(SEMI_AUTOMATIC);  // skip connecting to the cloud for (Electron) testing

#define HIGH_RANGE_F 100.0
#define LOW_RANGE_F   60.0

#define START_FAHRENHEIT 68.0
#define START_CELSIUS    20.0


unsigned long lastUpdate = 0;
//
// Temp Sensor Task Constructor
//
TempSensorTask::TempSensorTask( float eventThreshold,
                                void (*TempThresholdEventHandler)(float) )
{
  int i;

  //
  // Setup the Task Control States
  temp_state       = TS_INIT;
  temp_data_valid  = 0;
  tempEventTriggerFahrenheit = eventThreshold;
  lowReport        = LOW_RANGE_F;
  highReport       = HIGH_RANGE_F;
  TempEventHandler = TempThresholdEventHandler;
  fahrenheit       = (float) START_FAHRENHEIT;
  celsius          = (float) START_CELSIUS;
  belowThreshold   = 1;

  //int ts_pin     = (int) D6;
  //ds = OneWire((uint16_t) D6);  // 1-wire signal on pin D4

  //
  // Grab the Address for our attached sensor.
  ds.search(dev_addr);
  delay(250);

  //
  // Dump the ROM bytes
  #if 0
  Serial.print("Temp Sensor ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(dev_addr[i], HEX);
  }
  Serial.println();
  #endif

  //
  // Display the device type
  // the first ROM byte indicates which chip
  switch (dev_addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS1820/DS18S20");
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    case 0x26:
      //Serial.println("  Chip = DS2438");
      type_s = 2;
      break;
    default:
      //Serial.println("Unknown device type.");
      type_s = 0xFF;
      break;
  }
  //
  // Issue the Start, and we will actually read later
  SendTempConversionStart();
  temp_state = TS_WAIT_READ;
}

//
// Send the command to start the temperature conversion, we will warranty
// to wait roughly one second before we read out the calculated value
//
int TempSensorTask::SendTempConversionStart(void){

  ds.reset();               // first clear the 1-wire bus
  ds.select(dev_addr); // now select the device we just found
  ds.write(0x44, 0);  // or start conversion in powered mode (bus finishes low)

  ticks_last_start = millis();
}

//
// Time to read the actual temperature data.
//
int TempSensorTask::ReadTempSensor(void){
  int i;
  byte present = ds.reset();
  float t_cel;
  float t_fer;

  ds.select(dev_addr);
  ds.write(0xB8,0);         // Recall Memory 0
  ds.write(0x00,0);         // Recall Memory 0

  // now read the scratch pad
  present = ds.reset();
  ds.select(dev_addr);
  ds.write(0xBE,0);         // Read Scratchpad
  if (type_s == 2) {
    ds.write(0x00,0);       // The DS2438 needs a page# to read
  }

  // transfer and print the values
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    dev_data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (dev_data[1] << 8) | dev_data[0];
  if (type_s == 2) raw = (dev_data[2] << 8) | dev_data[1];
  byte cfg = (dev_data[4] & 0x60);

  switch (type_s) {
    case 1:
      raw = raw << 3; // 9 bit resolution default
      if (dev_data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - dev_data[6];
      }
      t_cel = (float)raw * 0.0625;
      break;
    case 0:
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      // default is 12 bit resolution, 750 ms conversion time
      t_cel = (float)raw * 0.0625;
      break;

    case 2:
      dev_data[1] = (dev_data[1] >> 3) & 0x1f;
      if (dev_data[2] > 127) {
        t_cel = (float)dev_data[2] - ((float)dev_data[1] * .03125);
      }else{
        t_cel = (float)dev_data[2] + ((float)dev_data[1] * .03125);
      }
  }

  t_fer = t_cel * 1.8 + 32.0;
  //
  // Smooth out bad readings, if we are not in the range, don't
  // save this reading.
  //
  if((t_fer >= lowReport) && (t_fer <= highReport)){
      fahrenheit = t_fer;
      celsius = t_cel;
  }else{
    ds.reset();   // first clear the 1-wire bus
  }


  ticks_last_read = millis();

  //Serial.print("  Temperature = ");
//  Serial.print(celsius);
//  Serial.print(" Celsius, ");
//  Serial.print(fahrenheit);
//  Serial.println(" Fahrenheit");

}

//
// Get the last read temperature
float TempSensorTask::GetLastTemp(int mode)
{
    float rc;

    if(mode == 1){
        rc = celsius;
    }else{
        rc = fahrenheit;
    }

    return rc;
}
//
// Register an EVENT Handler to call when the Threshold is HIT
void TempSensorTask::RegisterEventHandler(void (*NewEventHandler)(float))
{
  TempEventHandler = NewEventHandler;
}
//
// Register an EVENT Handler to call when the Threshold is HIT
void TempSensorTask::RegisterThreshold(float newEventThreshold)
{
  tempEventTriggerFahrenheit = newEventThreshold;
}

//
// Temp Sensor Task Loop
void TempSensorTask::TempSensorTaskHandler(void)
{
  int reportEvent = 0;
  char tempBuff[40];

  if(temp_state == TS_WAIT_READ){
      if((millis() - ticks_last_start) > READ_WAIT_TICKS){
        ReadTempSensor();
        temp_state = TS_WAIT_START;

        //
        // Only act if we crossed the Threshold
        if(fahrenheit >= tempEventTriggerFahrenheit){
          if(belowThreshold == 1){
            belowThreshold = 0;
            reportEvent = 1;
          }
        }else{
          //
          // Only act if we crossed the Threshold
          if(belowThreshold == 0){
            belowThreshold = 1;
            reportEvent = 1;
          }
        }
        sprintf(tempBuff, "T=%f, C=%f, R=%d",fahrenheit, tempEventTriggerFahrenheit ,reportEvent );
        Serial.println(tempBuff);
        //
        // Call the actual Handler
        if((reportEvent == 1) && (TempEventHandler != NULL)){
            TempEventHandler(fahrenheit);
        }
      }
  }else if(temp_state == TS_WAIT_START){
      if((millis() - ticks_last_read) > START_WAIT_TICKS){
        SendTempConversionStart();
        temp_state = TS_WAIT_READ;
      }
  }else{
      //
      // We are in a bad state
      //
      Serial.print("Unexpected State = ");
      Serial.println(temp_state);
  }
}
