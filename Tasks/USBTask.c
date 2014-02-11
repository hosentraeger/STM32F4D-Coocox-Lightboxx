#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4_discovery.h"
#include "USBTask.h"
#include "stm32_ub_usb_cdc.h"
#include "stdio.h"
#include "string.h"
#include "DispatcherTask.h"

void USBTask ( void * pvParameters )
{
	char buf[APP_TX_BUF_SIZE]; // puffer fuer Datenempfang
	USB_CDC_RXSTATUS_t check;
	struct ADispatcherMessage xDispatcherMessage, *pxDispatcherMessage;
	pxDispatcherMessage = & xDispatcherMessage;

	char * domain, * command, * parameter;

	// Init vom USB-OTG-Port als CDC-Device
	// (Virtueller-ComPort)
	UB_USB_CDC_Init ( );

	while ( 1 )
	{
		// Test ob USB-Verbindung zum PC besteht
		if(UB_USB_CDC_GetStatus()==USB_CDC_CONNECTED) {
			// Ceck ob Daten per USB empfangen wurden
			check=UB_USB_CDC_ReceiveString ( buf );
			if ( check == RX_READY )
			{
				domain = buf;
				command = strchr ( domain, ' ' );
				if ( 0 != command )
				{
					*command = '\0';
					command++;
					parameter = strchr ( command, ' ' );
					if ( 0 != parameter )
					{
						*parameter = '\0';
						parameter++;
						if ( 0 != xDispatcherQueue )
						{
							strncpy ( xDispatcherMessage.Domain, domain, 8 );
							strncpy ( xDispatcherMessage.Command, command, 8 );
							strncpy ( xDispatcherMessage.Parameter, parameter, 16 );
							xQueueSendToBack ( xDispatcherQueue, ( void * ) &pxDispatcherMessage, ( portTickType ) 0 );
						}
					};
				};
			};
		};
	};
};