/*
*  Header for Impact Sensor Project
*/

#ifndef TEMP_SENSOR_TASK_H
#define TEMP_SENSOR_TASK_H

#include <inttypes.h>
#include "OneWire.h"

class TempSensorTask
{
private:
  //
  // Temp Sensort Monitoring States
  #define TS_INIT          0x00
  #define TS_WAIT_START    0x01
  #define TS_WAIT_READ     0x02

  #define READ_WAIT_TICKS  1000
  #define START_WAIT_TICKS 5000

  #define DS_18B20_DEV_ADDR_LEN 8
  #define DS_18B20_DATA_LEN     12

  uint16_t sensorPin;
  float eventThreshold;

  uint8_t task_initialzed;
  uint8_t temp_data_valid;
  uint8_t temp_state;
  uint8_t type_s;

  uint8_t dev_addr[DS_18B20_DEV_ADDR_LEN];
  uint8_t dev_data[DS_18B20_DATA_LEN];

  uint32_t ticks_last_start;
  uint32_t ticks_last_read;
  float celsius;
  float fahrenheit;
  float tempEventTriggerFahrenheit;

  //
  // Onewire Class object for the DS18B20
  OneWire *ds;

  //
  // Private methods to do the work
  int SendTempConversionStart(void);
  int ReadTempSensor(void);

public:
  TempSensorTask(void);
  void TempSensorInit(uint16_t pin, float eventThreshold);

  float GetLastTemp(int mode);
  void TempSensorTaskHandler(void);
};

#endif
