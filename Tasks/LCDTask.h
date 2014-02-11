#ifndef __LCD_TASK_H__
#define __LCD_TASK_H__

#include "FreeRTOS.h"
#include "queue.h"

struct ALCDMessage
{
	char ucX;
	char ucY;
	char ucData[ 16 ];
};

xQueueHandle xLCDQueue;

void LCDTask ( void * pvParameters );

#endif // __LCD_TASK_H__
