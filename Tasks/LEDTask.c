#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4_discovery.h"
#include "LEDTask.h"

void LEDTask ( void * pvParameters )
{
	while ( 1 )
	{
		STM_EVAL_LEDToggle ( 0 );
		vTaskDelay ( 500 / portTICK_RATE_MS );
	};
};
