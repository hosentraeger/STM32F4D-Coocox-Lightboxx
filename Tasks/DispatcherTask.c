#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "DispatcherTask.h"
#include "LCDTask.h"
#include "AudioTask.h"
#include "USBTask.h"

#include "main.h"
#include "stm32_ub_rtc.h"
#include "led_btn_buz.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"

xQueueHandle xDispatcherQueue = NULL;

void PrintHelp ( )
{
	printf ( "Lightboxx (v%s)\n", VERSION );
	printf ( "================\n" );
	printf ( "RTC\n" );
	printf ( "  GET\n");
	printf ( "  SET dd.mm.yy hh:mm:ss\n");
	printf ( "AUDIO\n" );
	printf ( "  PLAY filename\n" );
	printf ( "  STOP\n" );
	printf ( "LED\n" );
	printf ( "  SET lednum,mode,pulses\n" );
	printf ( "  GET buttonnum\n" );
	printf ( " \n" );
};

void SendTimeToLCD ( struct ALCDMessage *pxLCDMessage )
{
	if ( 0 != xLCDQueue )
	{
		UB_RTC_GetClock(RTC_DEC);
		pxLCDMessage->ucX = 0;
		pxLCDMessage->ucY = 8;
		sprintf ( pxLCDMessage->ucData, "%02d:%02d:%02d", UB_RTC.std, UB_RTC.min, UB_RTC.sek );
		xQueueSendToBack ( xLCDQueue, ( void * ) &pxLCDMessage, ( portTickType ) 0 );
	};
};

void GetRTCTime ( )
{
	if( xSemaphoreTake ( xCDCSemaphore, ( portTickType ) 10 ) == pdTRUE )
	{
		UB_RTC_GetClock(RTC_DEC);
		printf ( "%02d.%02d.%02d %02d:%02d:%02d\r\n", UB_RTC.tag, UB_RTC.monat, UB_RTC.jahr, UB_RTC.std, UB_RTC.min, UB_RTC.sek );
		xSemaphoreGive( xCDCSemaphore );
	};
};

void SetRTCTime ( char * sTimeExpression )
{
	char  *yy, *mon, *dd, *hh, *mm, *ss;
	hh = strchr ( sTimeExpression, ' ' );
	if ( hh )
	{
		*hh = '\0';
		hh++;
		dd = sTimeExpression;
		mon = strchr ( dd, '.' );
		if ( 0 != mon )
		{
			*mon = '\0';
			mon++;
			yy = strchr ( mon, '.' );
			if ( 0 != yy )
			{
				*yy = '\0';
				yy++;
				UB_RTC.tag = atoi ( dd );
				UB_RTC.monat = atoi ( mon );
				UB_RTC.jahr = atoi ( yy );
				UB_RTC.wotag=5;
			};
		};
	}
	else
	{
		hh = sTimeExpression;
	};
	mm = strchr ( hh, ':' );
	if ( 0 != mm )
	{
		*mm = '\0';
		mm++;
		ss = strchr ( mm, ':' );
		if ( 0 != ss )
		{
			*ss = '\0';
			ss++;
			UB_RTC.std = atoi ( hh );
			UB_RTC.min = atoi ( mm );
			UB_RTC.sek = atoi ( ss );

			UB_RTC_SetClock ( RTC_DEC );
		};
	};
};

void LedSet ( const char * sCommand )
{
	if ( 0 == sCommand ) return;
	char * sMode, *sPulses;

	uint8_t nLed = 0;
	uint8_t nMode = 0;
	uint8_t nPulses = -1;

	sMode = strchr ( sCommand, ',' );
	if ( 0 != sMode )
	{
		*sMode = '\0';
		sMode++;
		sPulses = strchr ( sMode, ',' );
		if ( 0 != sPulses )
		{
			* sPulses = '\0';
			sPulses++;
		};
	};
	if ( sCommand ) nLed = atoi ( sCommand );
	if ( sMode ) nMode = atoi ( sMode );
	if ( sPulses ) nPulses = atoi ( sPulses );

	LedModeSet ( nLed, nMode, nPulses );
};

void LedGet ( const char * sCommand )
{
	uint8_t i = 0;
	uint8_t from = 0;
	uint8_t to = LED_NUM_BUTTONS;
	if ( 0 != sCommand )
	{
		from = atoi ( sCommand );
		to = from + 1;
	};

	for ( i = from; i < to; i++ )
	{
		if ( LedButtonGet ( i ) == LED_BUTTON_STATE_NOT_PRESSED ) printf ( "Button %d NOT PRESSED\n", i );
		if ( LedButtonGet ( i ) == LED_BUTTON_STATE_SHORT_PRESSED ) printf ( "Button %d SHORT PRESSED\n", i );
		if ( LedButtonGet ( i ) == LED_BUTTON_STATE_LONG_PRESSED ) printf ( "Button %d LONG PRESSED\n", i );
	};
	printf ( " \n" );
};

void ForwardAudioMessage ( const char * sCommand, const char * sFileName )
{
	struct AAudioMessage xAudioMessage, *pxAudioMessage = & xAudioMessage;
	strncpy ( xAudioMessage.Command, sCommand, 8 - 1 );
	strncpy ( xAudioMessage.Parameter, sFileName, 128 - 1 );
	xQueueSendToBack ( xAudioQueue, ( void * ) &pxAudioMessage, ( portTickType ) 0 );
};

void DispatcherTask ( void * pvParameters )
{
	struct ADispatcherMessage *pxDispatcherMessage;
	struct ALCDMessage xLCDMessage, *pxLCDMessage = & xLCDMessage;

	UB_RTC_Init ( );
	LedInit ( );

	xDispatcherQueue = xQueueCreate ( 8, sizeof ( unsigned long ) );

	while ( 1 )
	{
		SendTimeToLCD ( pxLCDMessage );

		if ( xQueueReceive ( xDispatcherQueue, & ( pxDispatcherMessage ), 250 / portTICK_RATE_MS ) )
		{
			if ( !strnicmp ( pxDispatcherMessage->Domain, "HELP", 3 ) )
			{
				PrintHelp ( );
			}
			else if ( !strnicmp ( pxDispatcherMessage->Domain, "RTC", 3 ) )
			{
				if ( !strnicmp ( pxDispatcherMessage->Command, "SET", 3 ) )
				{
					SetRTCTime ( pxDispatcherMessage->Parameter );
				}
				else if ( !strnicmp ( pxDispatcherMessage->Command, "GET", 3 ) )
				{
					GetRTCTime ( );
				};
			}
			else if ( !strnicmp ( pxDispatcherMessage->Domain, "AUDIO", 5 ) )
			{
				ForwardAudioMessage ( pxDispatcherMessage->Command, pxDispatcherMessage->Parameter );
			}
			else if ( !strnicmp ( pxDispatcherMessage->Domain, "LED", 3 ) )
			{
				if ( !strnicmp ( pxDispatcherMessage->Command, "SET", 3 ) )
				{
					LedSet ( pxDispatcherMessage->Parameter );
				}
				else if ( !strnicmp ( pxDispatcherMessage->Command, "GET", 3 ) )
				{
					LedGet ( pxDispatcherMessage->Parameter );
				};
			}
			else
			{
			};
		};
	};
};
