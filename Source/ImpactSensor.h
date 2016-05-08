/*
*  Header for Impact Sensor Project
*/

#ifndef IMPACT_SENSOR_H
#define IMPACT_SENSOR_H

#include <inttypes.h>


//
// Definitions for all used pins
//#define RED_LED     D3
//#define GREEN_LED   D4
//#define BLUE_LED    D5
//#define TEMP_SENSOR D6

/*
 *   Impact Sensor - Program Control Structure
 */
struct ProgramCntrl {

  uint8_t programInitialized;

  void *tempSensorControl;

};

void temp_sensor_task(void *taskCntrl);
void *setup_temp_sensor_task(float tempEventTrigger);
float get_last_temp(void *taskCntrl, int celsius);

#endif
