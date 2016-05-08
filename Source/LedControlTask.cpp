/*

 LED Control Class - Controls the Color and Mode.  The task handler must
 be called at least twice as fast as the longest blink period.

*/

// Only include One of the following depending on your environment!
#include "LedControlTask.h"

int redLed     = D3; // Red LED is on pin D3
int greenLed   = D4; // Green LED in on pin D4
int blueLed    = D5; // Blue LED is on pin D5



LedControlTask::LedControlTask(uint8_t ledColor, uint8_t ledMode)
{
  //
  // Setup the LED pins
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  slowBlinkRate   = SLOW_BLINK_PERIOD;
  fastBlinkRate   = FAST_BLINK_PERIOD;
  ticksLastChange = millis();
  handlerPollRate = DEFAULT_POLL_RATE;

  //
  // Turn everything off to get to a know state, this will matches
  // the "LED_OFF" state.
  TurnLedOff();
  //
  // Now use the class methods to set the LED to the mode desired
  // by the constructor parameters.
  SetLedMode(ledMode);
  SetLedColor(ledColor);
}

//
// Private method to turn the LED ON
void LedControlTask::TurnLedOff(void)
{
  //
  // Turn everything off to get to a know state, this will matches
  // the "LED_OFF" state.
  digitalWrite(blueLed, LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);

  ledState = LED_OFF;
}
//
// Private method to turn the LED ON
void LedControlTask::TurnLedOn(void)
{
  //
  // Toggle the individual LED lines based on the
  // active color selection
  if((activeColor & RED) == RED){
      digitalWrite(redLed, HIGH);
  }else{
      digitalWrite(redLed, LOW);
  }

  if((activeColor & BLUE) == BLUE){
      digitalWrite(blueLed, HIGH);
  }else{
      digitalWrite(blueLed, LOW);
  }

  if((activeColor & GREEN) == GREEN){
      digitalWrite(greenLed, HIGH);
  }else{
      digitalWrite(greenLed, LOW);
  }

  ledState = LED_ON;
}


//
// LED Control Task Handler
void LedControlTask::LedControlTaskHandler(void)
{
  uint32_t currTicks;
  //
  // If we are "OFF" or "ON" then there is really no work to do
  if((activeMode == LED_BLINK_SLOW) || (activeMode == LED_BLINK_FAST) ){
    //
    // Grab the ticks for test, and assignment if time to toggle
    currTicks = millis();
    //
    // See if it is time to toggle the LED yet.
    if((currTicks - ticksLastChange) >= activeBlinkRate){
      if(ledState == LED_OFF){
        TurnLedOn();
      }else{
        TurnLedOff();
      }
      ticksLastChange = currTicks;
    }
  }
}

//
// Set the LED for both color and mode
//
void LedControlTask::SetLed(uint8_t newColor, uint8_t newMode)
{

  SetLedColor(newColor);

  SetLedMode(newMode);

}

//
// Set the LED Active Color
//
void LedControlTask::SetLedColor(uint8_t newColor)
{
  //
  // If the newMode is not valid, it is a NO OP
  if( (newColor >= OFF) && (newColor <= WHITE) ){
      //
      // Setup the MODE stuff, and then the RATE Info if needed
      activeColor     = newColor;
      ticksLastChange = millis();

      if(newColor == OFF){
          TurnLedOff();
      }
  }
}

//
// Set the LED Active Mode
//
void LedControlTask::SetLedMode(uint8_t newMode)
{
  //
  // If the newMode is not valid, it is a NO OP
  if( (newMode >= LED_OFF) && (newMode <= LED_BLINK_FAST) ){
      //
      // Setup the MODE stuff, and then the RATE Info if needed
      activeMode     = newMode;
      ticksLastChange = millis();

      switch(newMode){
        case LED_OFF:
          TurnLedOff();
          break;
        case LED_ON:
          TurnLedOn();
          break;
        case LED_BLINK_SLOW:
          activeBlinkRate = slowBlinkRate;
          TurnLedOff();
          break;
        case LED_BLINK_FAST:
          activeBlinkRate = fastBlinkRate;
          TurnLedOff();
          break;
      }
  }
}

//
// Get the MAX Poll Rate
//
uint32_t LedControlTask::GetMaxTaskPollRate(void)
{
  return handlerPollRate;
}
