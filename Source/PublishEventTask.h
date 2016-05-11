/*
*  Header for Impact Sensor Project
*/

#ifndef PUBLISH_EVENT_TASK_H
#define PUBLISH_EVENT_TASK_H

#include <inttypes.h>
#include "OneWire.h"

#define MAX_PUB_EVENT_LEN 20
#define MAX_PUB_PAYLOAD_LEN 80

#define PUBLISH_EVENT_QUEUE 0x40
#define PUBLISH_EVENT_MASK  0x3F
/*
 *   Publish Event Data Structure
 */
struct PublishEventData {
  char event[MAX_PUB_EVENT_LEN];
  char payload[MAX_PUB_PAYLOAD_LEN];
};

class PublishEventTask
{
private:

  uint32_t popIndex  = 0;
  uint32_t pushIndex = 0;

  uint32_t lastPublishTicks = 0;

  PublishEventData queue[PUBLISH_EVENT_QUEUE];
  //
  // Private methods to do the work

public:
  PublishEventTask(void);
  void PushPublishEvent(char *eventName, char *eventPayload);
  void PublishEventTaskHandler(void);
};

#endif
