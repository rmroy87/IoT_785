/*

 LED Control Class - Controls the Color and Mode.  The task handler must
 be called at least twice as fast as the longest blink period.

*/

// Only include One of the following depending on your environment!
#include "ImpactSensorControl.h"
//
// Event Handler function prototypes
//
void TempThresholdEventHandler(float currTemp);
void MagnitudeThresholdEventHandler(short currMagnitude);


char *StateMessages[] =
  {
    "S0_SETUP",
    "S1_NOT_CONNECTED",
    "S2_CONNECTED",
    "S3_NOT_ACTIVE",
    "S4_SESSION_ACTIVE",
    "S5_IMPACTS_DETECTED",
    "S6_WAIT_FOR_CLEAR",
    "Error-Unkown State"
  };


  TempSensorTask   ts  = TempSensorTask( (float) INIT_TEMP_THRESHOLD,
                                        TempThresholdEventHandler);
  LedControlTask   led = LedControlTask(BLUE, LED_ON);
  MotionDetectTask motion = MotionDetectTask((uint8_t) POLL_MODE_MOTION,
                                             (uint32_t) POLL_RATE_MS,
                                             (short) IMPACT_BASE_THRESHOLD,
                                             MagnitudeThresholdEventHandler);

 extern ImpactSensorCntrol cntrl;
 //
 // Event Handler that is called when the temperature Threshold
 // has been HIT on the way up, and called again when the
 // temperature drops below threshold
 void TempThresholdEventHandler(float currTemp)
 {
   cntrl.TempThresholdEventHandler(currTemp);
 }

 //
 // Event Handler for the Max Threshold
 void MagnitudeThresholdEventHandler(short currMagnitude)
 {
   cntrl.MagnitudeThresholdEventHandler(currMagnitude);
 }





/*******************************************************************/
// Private Class Functions
/*******************************************************************/


//
// Method to PUSH the next event, if no event return -1
int ImpactSensorCntrol::PushEvent(uint8_t newEvent)
{
  int rc;
  char tempBuff[20];
  //
  // If the queue is full return an error, else push it
  if((eventPushIndex - eventPopIndex) < MAX_EVENT_QUEUE_DEPTH){
    //
    // Push into the queue, use the mask as to not write past the
    // end of the queue.
    eventQueue[(eventPushIndex & EVENT_QUEUE_MASK)] = newEvent;
    eventPushIndex++;
    sprintf(tempBuff,"NE=%d,Index=%d", newEvent,eventPushIndex );
    Serial.println(tempBuff);
    rc = 0;
  }else{
    rc = -1;  /* THe queue is full */
  }
  return rc;
}


//
// Method to POP the next event, if no event return -1
int ImpactSensorCntrol::PopEvent(uint8_t *nextEvent)
{
  int rc;
  //
  // If the queue is empty, there is nothing to do
  if(eventPushIndex == eventPopIndex){
    *nextEvent = 0xFF; //  Set to bogus event, just in case
    rc = -1;  /* We are empty */
  }else{
    //
    // Pop the event using the MASK to make sure we stay in the array
    *nextEvent = eventQueue[(eventPopIndex & EVENT_QUEUE_MASK)];
    eventPopIndex++; // Onto the next
    rc= 0;
  }
  return rc;
}

//
// When we transition states, publish them to the cloud and the
// console
void ImpactSensorCntrol::PublishStateTransition(char *nextState)
{
    Particle.publish("StateTransition", nextState);
}

//
// All of the State Machine Routines are her.
uint8_t ImpactSensorCntrol::SM_Setup(void)
{
  int count = 0;
  //
  // Do any setup work for state Machine here
  while((Particle.connected() == 0) && (count++ < 100)){
    delay(250);
  }

  return S1_NOT_CONNECTED;
}
//
// Not yet connected, look for the connection state
uint8_t ImpactSensorCntrol::SM_Not_Connected(void)
{
  uint8_t nextState = S1_NOT_CONNECTED;

  //
  // If we are connected, set next state to connected
  if(Particle.connected()){
      connected = 1;
      nextState = S2_CONNECTED;
  }else{
      connected = 0;
      nextState = S3_NOT_ACTIVE;
  }

  return nextState;
}
//
// Set the LED to a Blinking State to indicate connection
uint8_t ImpactSensorCntrol::SM_Connected(void)
{
  uint8_t nextState = S3_NOT_ACTIVE;

  ledMode = LED_BLINK_SLOW;
  led.SetLed(ledColor, ledMode);

  return nextState;
}
//
// Not yet active based on the TEMPERATURE
uint8_t ImpactSensorCntrol::SM_Not_Active(void)
{
  uint8_t nextState = S3_NOT_ACTIVE;
  uint8_t newEvent;
  int ec;

  publishImpacts = 0;  // NOt active, so do not publish

  ec = PopEvent(&newEvent);
  if(ec == 0){
    if(E2_TEMP_THRES_UP){
        ledColor       = GREEN;
        led.SetLed(ledColor, ledMode);
        nextState = S4_SESSION_ACTIVE;
    }
  }

  return nextState;
}
//
// Session is now active and will start recording Impact
uint8_t ImpactSensorCntrol::SM_Session_Active(void)
{
  uint8_t nextState = S4_SESSION_ACTIVE;
  uint8_t newEvent;
  int ec;

  publishImpacts = 1; // Active, start publishing

  ec = PopEvent(&newEvent);
  if(ec == 0){
    switch(newEvent){
      case E3_TEMP_THRES_DOWN:
        ledColor  = BLUE;
        nextState = S3_NOT_ACTIVE;
        break;
      case E4_IMPACT:
        ledColor = YELLOW;
        nextState = S5_IMPACTS_DETECTED;
        break;
      case E5_SEVERE_IMPACT:
        ledColor = YELLOW;
        ledMode  = LED_BLINK_FAST;
        severImpactCount++;
        TestSeverImpactCount();
        nextState = S5_IMPACTS_DETECTED;
        break;
      case E6_PLAYER_OUT:
        ledColor  = RED;
        ledMode   = LED_BLINK_SLOW;
        nextState = S6_WAIT_FOR_CLEAR;
        break;
    }
    led.SetLed(ledColor, ledMode);
  }

  return nextState;
}
//
// Impacts have been recorded
uint8_t ImpactSensorCntrol::SM_Impacts_Detected(void)
{
  uint8_t nextState = S5_IMPACTS_DETECTED;
  uint8_t newEvent;
  int ec;

  ec = PopEvent(&newEvent);
  if(ec == 0){
    switch(newEvent){
      case E3_TEMP_THRES_DOWN:
        ledColor  = BLUE;
        nextState = S3_NOT_ACTIVE;
        break;
      case E5_SEVERE_IMPACT:
        ledColor = YELLOW;
        ledMode  = LED_BLINK_FAST;
        severImpactCount++;
        TestSeverImpactCount();
        nextState = S5_IMPACTS_DETECTED;
        break;
      case E6_PLAYER_OUT:
        ledColor  = RED;
        ledMode   = LED_BLINK_SLOW;
        nextState = S6_WAIT_FOR_CLEAR;
        break;
    }
    led.SetLed(ledColor, ledMode);
  }

  return nextState;
}
//
// Player is OUT, must be cleared to return to play
uint8_t ImpactSensorCntrol::SM_Wait_For_Clear(void)
{
  uint8_t nextState = S6_WAIT_FOR_CLEAR;
  uint8_t newEvent;
  int ec;

  ec = PopEvent(&newEvent);
  if(ec == 0){
    switch(newEvent){
      case E7_PLAYER_CLEARED:
        ledColor  = YELLOW;
        ledMode   = LED_BLINK_FAST;
        nextState = S5_IMPACTS_DETECTED;

        if(ts.GetLastTemp(0) < (float) INIT_TEMP_THRESHOLD){
          PushEvent(E3_TEMP_THRES_DOWN);
        }
        break;
    }
    led.SetLed(ledColor, ledMode);
  }

  return nextState;
}

uint8_t ImpactSensorCntrol::TestSeverImpactCount(void)
{
  uint8_t rc = 0;;

  if(severImpactCount > maxSevereImpactCount){
    PushEvent(E6_PLAYER_OUT);
    rc =1;
  }
  return rc;
}
/*******************************************************************/
// PUblic Class Functions
/*******************************************************************/
//
// Event Handler that is called when the temperature Threshold
// has been HIT on the way up, and called again when the
// temperature drops below threshold
void ImpactSensorCntrol::TempThresholdEventHandler(float currTemp)
{
  int ec;

  Serial.print("*** Temp Thershold Hit: ");
  Serial.print(currTemp);
  Serial.print(" Ticks: ");
  Serial.print(millis());
  Serial.println(" ***");
  //
  // Push the EVENT
  if(currTemp >= (float) INIT_TEMP_THRESHOLD ){
      ec = PushEvent(E2_TEMP_THRES_UP);
  }else if(currTemp < (float) INIT_TEMP_THRESHOLD){
      ec = PushEvent(E3_TEMP_THRES_DOWN);
  }
  //
  // If the queue is full post an ERROR Msg
  if(ec < 0){
    Particle.publish("ERROR_LOG", "Event Queue Full - Temp Event");
    Serial.println("Event Queue Full - Temp Event");
  }
}

//
// Event Handler for the Max Threshold
void ImpactSensorCntrol::MagnitudeThresholdEventHandler(short currMagnitude)
{
  char tempBuff[20];
  int ec;

  Serial.print("*** Magnitude Thershold Hit: ");
  Serial.print(currMagnitude);
  Serial.print(" Ticks: ");
  Serial.print(millis());
  Serial.println(" ***");

  //
  // Only publish events if we are in an active session
  if(publishImpacts == 1){
    sprintf(tempBuff,"%d", currMagnitude);
    Particle.publish("librato_impact", tempBuff);

    //
    // Check to see the level of impact to
    if(currMagnitude >= criticalImpact){
      ec = PushEvent(E6_PLAYER_OUT);
    }else if(currMagnitude >= severeImpact){
      ec = PushEvent(E5_SEVERE_IMPACT);
    }else{
      ec = PushEvent(E4_IMPACT);
    }
    //
    // If the queue is full post an ERROR Msg
    if(ec < 0){
      Particle.publish("ERROR_LOG", "Event Queue Full - Impact Event");
      Serial.print("Event Queue Full - Impact Event");
    }
  }
}




//
// Impact Sensor Control -- The Main State Machine for the Impact Sensort
// Application.  All states, and state transitions will occur here.
//
ImpactSensorCntrol::ImpactSensorCntrol(void)
{

}

//
// Run the ImpactSensor Control State Machine, ,ust be called everythime
// through the loop, cannot be blocked
//
void ImpactSensorCntrol::RunImpactSensorControl(void)
{
  static uint32_t currTickCount = 0;
  static uint32_t lastTickCount = 0;
  char   tempArray[20];
  float  currTemp;

  ts.TempSensorTaskHandler();
  led.LedControlTaskHandler();
  motion.MotionDetectTaskHandler();

  if(stateChangePending == 1){
      //
      // Publish the State Change
      PublishStateTransition(StateMessages[currentState]);
      stateChangePending = 0;
      delay(250);
  }else{
      //
      // Publish the temp every ten seconds
      currTickCount = millis();
      if((currTickCount - lastTickCount) >= 10000){
        currTemp      = ts.GetLastTemp(0);
        lastTickCount = currTickCount;
        sprintf(tempArray,"%f", currTemp);
        Particle.publish("librato_temp", tempArray);
        delay(250);
      }
  }
  //
  // Execute the event handler for the current state
  switch(currentState){
    case S0_SETUP:
      nextState = SM_Setup();
      break;
    case S1_NOT_CONNECTED:
      nextState = SM_Not_Connected();
      break;
    case S2_CONNECTED:
      nextState = SM_Connected();
      break;
    case S3_NOT_ACTIVE:
      nextState = SM_Not_Active();
      break;
    case S4_SESSION_ACTIVE:
      nextState = SM_Session_Active();
      break;
    case S5_IMPACTS_DETECTED:
      nextState = SM_Impacts_Detected();
      break;
    case S6_WAIT_FOR_CLEAR:
      nextState = SM_Wait_For_Clear();
      break;
  }
  //
  // If we have a change pending, set the state for the next Loop
  if(currentState != nextState){
      stateChangePending = 1;
      currentState = nextState;

      //
      // Some state transitions require sepcial handling
      if(nextState == S6_WAIT_FOR_CLEAR){
        Particle.publish("twilio", "Player Removed - Needs Concusion Protocol to be Administered" );
        delay(250);
      }
  }
}

//
// Events can be pushed from the Trainer or Medical Staff, This
// event is to Cleaf a Player to return.
void ImpactSensorCntrol::InsertPlayerClearedEvent()
{
  PushEvent(E7_PLAYER_CLEARED);
}

//
// This event is to remove a player and mark them as OUT
void ImpactSensorCntrol::InsertPlayerOutEvent()
{
  PushEvent(E6_PLAYER_OUT);
}
