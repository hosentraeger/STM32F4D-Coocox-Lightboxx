#ifndef __AUDIO_TASK_H__
#define __AUDIO_TASK_H__

#include "FreeRTOS.h"
#include "queue.h"

struct AAudioMessage
{
	char Command[8];
	char Parameter[128];
};

xQueueHandle xAudioQueue;

void AudioTask ( void * pvParameters );

#endif // __AUDIO_TASK_H__
