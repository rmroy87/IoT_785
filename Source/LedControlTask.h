/*
*  Header for Impact Sensor Project
*/
#ifndef LED_CONTROL_TASK_H
#define LED_CONTROL_TASK_H

#include <inttypes.h>
#include "application.h"

//
// Color modes for the LED
#define OFF    0x00
#define RED    0x01
#define BLUE   0x02
#define VIOLET 0x03
#define GREEN  0x04
#define YELLOW 0x05
#define CYAN   0x06
#define WHITE  0x07
//
// LED Modes
#define LED_OFF         0x00
#define LED_ON          0x01
#define LED_BLINK_SLOW  0x02
#define LED_BLINK_FAST  0x03
//
// Blink Rates
#define SLOW_BLINK_PERIOD 500 // Really half-period, the ON or the OFF time
#define FAST_BLINK_PERIOD 250 // Really half-period, the ON or the OFF time
//
//
#define DEFAULT_POLL_RATE 20 // Milliseconds
//
// LED Control Class - Controls the Color and Mode.  The task handler must
// be called at least twice as fast as the longest blink period.
//
class LedControlTask
{
private:
  //
  // Temp Sensort Monitoring States
  uint8_t activeColor;
  uint8_t activeMode;

  uint8_t  ledState;         // On or off as last state
  uint32_t slowBlinkRate;    // Slow blink period, all 50% duty cycle
  uint32_t fastBlinkRate;    // Fast blink period, all 50% duty cycle
  uint32_t ticksLastChange;  // Ticks last LED toggle
  uint32_t activeBlinkRate;  // Currently selected Poll Rate
  uint32_t handlerPollRate;  // Max poll period in milliseconds

  void TurnLedOff(void);
  void TurnLedOn(void);
public:
  LedControlTask(uint8_t ledColor, uint8_t ledMode);
  void SetLed(uint8_t newColor, uint8_t newMode);
  void SetLedMode(uint8_t newMode);
  void SetLedColor(uint8_t newColor);
  void LedControlTaskHandler(void);
  uint32_t GetMaxTaskPollRate(void);
};

#endif
