#ifndef __DISPATCHER_TASK_H__
#define __DISPATCHER_TASK_H__

#include "FreeRTOS.h"
#include "queue.h"

struct ADispatcherMessage
{
	char Domain[8];
	char Command[8];
	char Parameter[16];
};

xQueueHandle xDispatcherQueue;


void DispatcherTask ( void * pvParameters );

#endif // __DISPATCHER_TASK_H__
