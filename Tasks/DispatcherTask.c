#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4_discovery.h"
#include "DispatcherTask.h"
#include "stm32_ub_rtc.h"
#include "printf.h"
#include "LCDTask.h"

#include "string.h"
#include "stdlib.h"

void DispatcherTask ( void * pvParameters )
{
	struct ALCDMessage xLCDMessage, *pxLCDMessage;
	struct ADispatcherMessage *pxDispatcherMessage;

	pxLCDMessage = & xLCDMessage;

	UB_RTC_Init();

    xDispatcherQueue = xQueueCreate ( 8, sizeof ( unsigned long ) );

	while ( 1 )
	{
		if ( 0 != xLCDQueue )
		{
			UB_RTC_GetClock(RTC_DEC);
			xLCDMessage.ucX = 0;
			xLCDMessage.ucY = 8;
			sprintf ( xLCDMessage.ucData, "%02d:%02d:%02d", UB_RTC.std, UB_RTC.min, UB_RTC.sek );
			xQueueSendToBack ( xLCDQueue, ( void * ) &pxLCDMessage, ( portTickType ) 0 );
		};

		if ( xQueueReceive ( xDispatcherQueue, & ( pxDispatcherMessage ), 250 / portTICK_RATE_MS ) )
		{
			if ( !strncmp ( pxDispatcherMessage->Domain, "RTC", 3 ) && !strncmp ( pxDispatcherMessage->Command, "SET", 3 ) )
			{
				char  *yy, *mon, *dd, *hh, *mm, *ss;
				hh = strchr ( pxDispatcherMessage->Parameter, ' ' );
				if ( hh )
				{
					*hh = '\0';
					hh++;
					dd = pxDispatcherMessage->Parameter;
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
						    UB_RTC.wotag=3;
						};
					};
				}
				else
				{
					hh = pxDispatcherMessage->Parameter;
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
		};
	};
};
