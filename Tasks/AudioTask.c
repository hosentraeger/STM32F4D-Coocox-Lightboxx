#include "AudioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32f4_discovery.h"
#include <stdio.h>
#include <string.h>
#include "USBTask.h"

void AudioTask ( void * pvParameters )
{
	struct AAudioMessage *pxAudioMessage;

	xAudioQueue = xQueueCreate ( 8, sizeof ( unsigned long ) );

	while ( 1 )
	{
		if ( xQueueReceive ( xAudioQueue, & ( pxAudioMessage ), 250 / portTICK_RATE_MS ) )
		{
			if ( !strnicmp ( pxAudioMessage->Command, "PLAY", 4 ) )
			{
				if( xSemaphoreTake ( xCDCSemaphore, ( portTickType ) 10 ) == pdTRUE )
				{
					printf ( "play file %s\r\n", pxAudioMessage->Parameter );
					xSemaphoreGive( xCDCSemaphore );
				};
			};
			if ( !strnicmp ( pxAudioMessage->Command, "STOP", 4 ) )
			{
				if( xSemaphoreTake ( xCDCSemaphore, ( portTickType ) 10 ) == pdTRUE )
				{
					printf ( "stop playing\r\n" );
					xSemaphoreGive( xCDCSemaphore );
				};
			};
		};
	};
};
