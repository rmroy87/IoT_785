/*
  Task Handler for Publishing Events to the Cloud
*/

// Only include One of the following depending on your environment!
#include "PublishEventTask.h"

// Comment this out for normal operation
//SYSTEM_MODE(SEMI_AUTOMATIC);  // skip connecting to the cloud for (Electron) testing

#define PUBLISH_RATE 1000



//
// Temp Sensor Task Constructor
//
PublishEventTask::PublishEventTask(void)
{

}

void PublishEventTask::PushPublishEvent(char *eventName, char *eventPayload)
{
  uint32_t  index = pushIndex & PUBLISH_EVENT_MASK;

  sprintf(&queue[index].event[0], "%s", eventName);
  sprintf(&queue[index].payload[0], "%s", eventPayload);

  pushIndex++;

}

void PublishEventTask::PublishEventTaskHandler(void)
{
  uint32_t currTicks = millis();
  uint32_t index;

  //
  // Check to see if it is time to report
  if((currTicks - lastPublishTicks) >= (uint32_t) PUBLISH_RATE){
      if(popIndex != pushIndex){
        index = popIndex & PUBLISH_EVENT_MASK;
        Particle.publish(&queue[index].event[0], &queue[index].payload[0]);
        popIndex++;
        lastPublishTicks = currTicks;
      }
  }
}
