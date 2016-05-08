/*
*  Header for Motion Detection Task
*/

#ifndef MOTION_DETECT_TASK_H
#define MOTION_DETECT_TASK_H

#include <inttypes.h>
#include "math.h"
#include "application.h"

/*
 *   Accelrometer data for 1-Axis
 */
typedef struct {
  short currVal;      // Reading for current iteration
  short lastVal;      // Reading last iteration
  short absValDelta;  // Delta between curr and last
} axis_data;

class MotionDetectTask
{
private:
  //
  // Device address as specified in data sheet
  #define DEVICE (0x53)
  #define ADXL345_ACCEL_DATA_READ_LEN 6

  char POWER_CTL   = 0x2D;	//Power Control Register
  char DATA_FORMAT = 0x31;
  char DATAX0      = 0x32;	//X-Axis Data 0
  char DATAX1      = 0x33;	//X-Axis Data 1
  char DATAY0      = 0x34;	//Y-Axis Data 0
  char DATAY1      = 0x35;	//Y-Axis Data 1
  char DATAZ0      = 0x36;	//Z-Axis Data 0
  char DATAZ1      = 0x37;	//Z-Axis Data 1

  uint8_t pollMode;    // Set to 1 if in poll mode
  uint8_t accelReady;  // If not poll, flag indicates read
  uint32_t pollRateMS; // How often to POLL
  //
  // Temp Sensort Monitoring States
  axis_data x_axis;
  axis_data y_axis;
  axis_data z_axis;

  short currMagnitude;        // Current magnitude
  short maxRecordedMagnitude; // Max value read

  uint32_t ticksCurrRead;
  uint32_t ticksLastRead;

  //
  // Thrshold stuff
  short magnitudeEventTheshold;
  void (*EventThresholdHandler)(short);

  //
  // The raw buffer for the sensor read data
  byte raw_buff[ADXL345_ACCEL_DATA_READ_LEN];

  //
  // Private methods to do the work
  void readFrom(byte address, int num, byte _buff[]);
  void writeTo(byte address, byte val);
  void CalculateMagnitude(void);
  void readAccel( short *x_axis,short *y_axis,short *z_axis);

public:
  MotionDetectTask( uint8_t runPollMode,
                    uint32_t initPollRateMS,
                    short initialEventThershold,
                    void (*EventHandler)(short) );

  void MotionDetectTaskHandler(void);
  void GetLastMagnitude(short *lastMagnitude, short *maxMagnitude);
  void SetNewEventThershold(short newEventThreshold);
};

#endif
