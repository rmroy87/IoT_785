/*
*  Header for Impact Sensor Project
*/
#ifndef IMPACT_SENSOR_CONTROL_H
#define IMPACT_SENSOR_CONTROL_H

#include <inttypes.h>
#include "application.h"
#include "TempSensorTask.h"
#include "ImpactSensor.h"
#include "LedControlTask.h"
#include "MotionDetectTask.h"

//
// State Machine - Events
#define E0_INIT_DONE        0x00
#define E1_CONNECTED        0x01
#define E2_TEMP_THRES_UP    0x02
#define E3_TEMP_THRES_DOWN  0x03
#define E4_IMPACT           0x04
#define E5_SEVERE_IMPACT    0x05
#define E6_PLAYER_OUT       0x06
#define E7_PLAYER_CLEARED   0x07
#define E8_DISCONNECTED     0x08

//
// State Machine - States
#define S0_SETUP              0
#define S1_NOT_CONNECTED      1
#define S2_CONNECTED          2
#define S3_NOT_ACTIVE         3
#define S4_SESSION_ACTIVE     4
#define S5_IMPACTS_DETECTED   5
#define S6_WAIT_FOR_CLEAR     6

#define MAX_EVENT_QUEUE_DEPTH 0x0008
#define EVENT_QUEUE_MASK      0x0007

#define INIT_TEMP_THRESHOLD 80.0

#define POLL_MODE_MOTION 1
#define POLL_RATE_MS     50
#define IMPACT_BASE_THRESHOLD     100
#define SEVERE_IMPACT_THRESHOLD   250
#define CRITICAL_IMPACT_THRESHOLD 650
#define SEVER_IMPACT_COUNT  10

//
// Impact Sensor Control -- The Main State Machine for the Impact Sensort
// Application.  All states, and state transitions will occur here.
//
class ImpactSensorCntrol
{
private:
  uint8_t  ledColor  = BLUE;   // Determined purely by state
  uint8_t  ledMode   = LED_ON; // Connected/Unconnected/Changing
  uint8_t  connected = 0;      // Set when collected to cloud
  //
  // Thresholds defaults and used when not connected in the stand alone Mode
  short baseImpact       = IMPACT_BASE_THRESHOLD;
  short severeImpact     = SEVERE_IMPACT_THRESHOLD;
  short criticalImpact   = CRITICAL_IMPACT_THRESHOLD;
  short maxSevereImpactCount = SEVER_IMPACT_COUNT;
  short severImpactCount = 0;

  //
  // Simple circular event queue
  uint8_t   eventQueue[MAX_EVENT_QUEUE_DEPTH];
  uint32_t  eventPushIndex = 0;
  uint32_t  eventPopIndex  = 0;

  uint8_t  currentState = S0_SETUP; // State executing now
  uint8_t  nextState    = S0_SETUP; // State that will execute next
  uint8_t  stateChangePending = 0;  // Next Entry will be new state
  uint8_t  publishImpacts     = 0;  // Set to 1 when active

  uint8_t SM_Setup(void);
  uint8_t SM_Not_Connected(void);
  uint8_t SM_Connected(void);
  uint8_t SM_Not_Active(void);
  uint8_t SM_Session_Active(void);
  uint8_t SM_Impacts_Detected(void);
  uint8_t SM_Wait_For_Clear(void);
  void PublishStateTransition(char *nextState);

  uint8_t TestSeverImpactCount(void);
  int PushEvent(uint8_t newEvent);
  int PopEvent(uint8_t *nextEvent);

public:
  void MagnitudeThresholdEventHandler(short currMagnitude);
  void TempThresholdEventHandler(float currTemp);
  ImpactSensorCntrol(void);
  void RunImpactSensorControl(void);
  void InsertPlayerClearedEvent(void);
  void InsertPlayerOutEvent(void);


};

#endif
