#include "IRTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4_discovery.h"
#include "stdio.h"
#include "ff.h"
#include "irmp.h"

/*
 * TSOP4838 -> PB.01
 */

void TimerInit ( void )
{
	TIM_TimeBaseInitTypeDef TIM_TimeBase_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	//Set Timer TIM2
	TIM_TimeBase_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBase_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBase_InitStructure.TIM_Period = 5250000 / F_INTERRUPTS - 1;
	TIM_TimeBase_InitStructure.TIM_Prescaler = ( ( SystemCoreClock / 2 ) / 5250000 ) - 1;
	TIM_TimeBaseInit ( TIM2, &TIM_TimeBase_InitStructure );

	TIM_ITConfig ( TIM2, TIM_IT_Update, ENABLE );

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_Init ( &NVIC_InitStructure );

	TIM_Cmd ( TIM2, ENABLE );
};

void TIM2_IRQHandler ( void )
{
	TIM_ClearITPendingBit ( TIM2, TIM_IT_Update );
	irmp_ISR ( );             // call irmp ISR
};

void IRTask ( void * pvParameters )
{
	TimerInit ( );
	irmp_init ( );                         // initialize irmp

	IRMP_DATA irmp_data;

	while ( 1 )
	{
		if ( irmp_get_data ( &irmp_data ) )
		{
			printf ( "Protocol: %d, Address: %d, Command: %d, Flags: %d\n",
					( uint16_t ) irmp_data.protocol,
					irmp_data.command,
					irmp_data.flags,
					( uint16_t ) irmp_data.protocol );
		};
	}
}
