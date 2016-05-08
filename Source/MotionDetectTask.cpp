/*
  Motion Detection Task - This Class will handle all access to the
  Acceleormeter.
*/
#include "MotionDetectTask.h"
//
// Constructor for the Motion Detection Task, it will control actuall
// access to the Accelerometer
//
MotionDetectTask::MotionDetectTask( uint8_t runPollMode,
                                    uint32_t initPollRateMS,
                                    short initialEventThershold,
                                    void (*EventHandler)(short))
{
  accelReady           = 0;
  currMagnitude        = 0;
  maxRecordedMagnitude = 0;
  ticksCurrRead        = 0;

  //
  // Save the needed constructor parameters
  pollMode = runPollMode;
  pollRateMS = initPollRateMS;
  magnitudeEventTheshold = initialEventThershold;
  EventThresholdHandler  = EventHandler;

  Wire.begin();        // join i2c bus (address optional for master)

  //Put the ADXL345 into +/- 16G range by writing the value 0x03 to the DATA_FORMAT register.
  writeTo(DATA_FORMAT, 0x03);
  //Put the ADXL345 into Measurement Mode by writing 0x08 to the POWER_CTL register.
  writeTo(POWER_CTL, 0x08);

  //
  // Prime the pump and setup the last read values
  readAccel( &x_axis.lastVal,
             &y_axis.lastVal,
             &z_axis.lastVal);

  ticksLastRead = millis();

}

//
// Motion detection Handler
void MotionDetectTask::MotionDetectTaskHandler(void)
{
  uint32_t currTicks;//, savedTicks;
  char tempBuff[40];

  //
  // Grab the ticks for test, and assignment if time to toggle
  currTicks = millis();
  //
  // If we are in poll mode, check to see if it is time.
  if(pollMode == 1){
    if((currTicks - ticksLastRead) >= pollRateMS){
      //savedTicks = ticksLastRead;
      accelReady = 1;
    }
  }
  //
  // Either mode, this flag indicates that it is time to
  // calculate the magnitude
  if(accelReady == 1){
      accelReady = 0;

      CalculateMagnitude();
      //
      // Check to see if we have exceed the event thershold as
      // configured.  If so call the event handler, assuming it
      // is not NULL
      if(currMagnitude >= magnitudeEventTheshold){
        if(EventThresholdHandler != NULL){
          //sprintf(tempBuff,"CT=%d,LT=%d,PR=%d",
          //    currTicks,savedTicks,pollRateMS );
          //Serial.println(tempBuff);
          EventThresholdHandler(currMagnitude);
        }
      }
  }
}

//
// Return the Magnitudes for the last and the MAX recorded
void MotionDetectTask::GetLastMagnitude(  short *lastMagnitude,
                                          short *maxMagnitude)
{
  *lastMagnitude = currMagnitude;
  *maxMagnitude  = maxRecordedMagnitude;
}

//
// Save the Event Threshold value
void MotionDetectTask::SetNewEventThershold(short newEventThreshold)
{
    magnitudeEventTheshold = newEventThreshold;
}

//
// Calculate the magnitude of the current acceleration
// if there is any delta between this reading and the last
void MotionDetectTask::CalculateMagnitude(void)
{
    short get_x;
    short get_y;
    short get_z;

    ticksCurrRead = millis();

    //
    // Grab the current readings, then compare to the
    // last read value and then calculate the magnitude
    readAccel( &x_axis.currVal,
               &y_axis.currVal,
               &z_axis.currVal);

    get_x = (x_axis.currVal >= x_axis.lastVal) ?
               (x_axis.currVal - x_axis.lastVal ) :
               (x_axis.lastVal - x_axis.currVal );

    get_y = (y_axis.currVal >= y_axis.lastVal) ?
               (y_axis.currVal - y_axis.lastVal ) :
               (y_axis.lastVal - y_axis.currVal );

    get_z = (z_axis.currVal >= z_axis.lastVal) ?
               (z_axis.currVal - z_axis.lastVal ) :
               (z_axis.lastVal - z_axis.currVal );

    //
    // Calculate the current magnitude based on the Delta
    // of the last two readings
    currMagnitude = sqrt( ((int) (get_x * get_x)) +
                          ((int)( get_y * get_y)) +
                          ((int) (get_z * get_z) )  );

    if(currMagnitude > maxRecordedMagnitude){
      maxRecordedMagnitude = currMagnitude;
    }

    ticksLastRead  = ticksCurrRead;
    x_axis.lastVal = x_axis.currVal;
    y_axis.lastVal = y_axis.currVal;
    z_axis.lastVal = z_axis.currVal;

}

//
// Read the raw data from the accelerometer and convert into
// the 3 axis vriables.
void MotionDetectTask::readAccel( short *x_axis,
                                  short *y_axis,
                                  short *z_axis)
{

  uint8_t howManyBytesToRead = ADXL345_ACCEL_DATA_READ_LEN;

  //
  // Read the acceleration data from the ADXL345
  readFrom( DATAX0, howManyBytesToRead, raw_buff);

  //
  // Each axis reading comes in 10 bit resolution, ie 2 bytes.
  // Least Significat Byte first!!
  // thus we are converting both bytes in to one int
  *x_axis = (((short)raw_buff[1]) << 8) | raw_buff[0];
  *y_axis = (((short)raw_buff[3]) << 8) | raw_buff[2];
  *z_axis = (((short)raw_buff[5]) << 8) | raw_buff[4];
}

//
// Write raw data to the I2C Device
void MotionDetectTask::writeTo(byte address, byte val) {
  Wire.beginTransmission(DEVICE);  // start transmission to device
  Wire.write(address);             // send register address
  Wire.write(val);                 // send value to write
  Wire.endTransmission();          // end transmission
}

//
// Reads num bytes starting from address register on device
// in to _buff array
void MotionDetectTask::readFrom(byte address, int num, byte _buff[])
{
  Wire.beginTransmission(DEVICE); // start transmission to device
  Wire.write(address);            // sends address to read from
  Wire.endTransmission();         // end transmission

  Wire.beginTransmission(DEVICE); // start transmission to device
  Wire.requestFrom(DEVICE, num);  // request 6 bytes from device

  int i = 0;
  while(Wire.available())         // device may send less than requested (abnormal)
  {
    _buff[i] = Wire.read();       // receive a byte
    i++;
  }
  Wire.endTransmission();         // end transmission
}
