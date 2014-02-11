#include "stm32f4xx.h"

/*
 * FreeRTOS includes
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ws2812.h"

/*
 * T0H	0 code, high voltage time	0.5 usec	+-150nsec
 * T1H	1 code, high voltage time	1.2 usec 	+-150nsec
 * T0L	0 code, low voltage time	2.0 usec	+-150nsec
 * T1L	1 code, low voltage time	1.3 usec	+-150nsec
 * RES	low voltage time			Above 50usec
 * Note: it is one half of the time when high speed mode (reset time unchanged)
 *
 * T0H 	T0L			T1H	T1L
 * 0.25	1.0			0.6	0.65
 * 20%				48%			Duty Cycle
 * 7				16.8		Capture Compare Value
  */
#define CCR1_L	7
#define CCR1_H	17

/*
 * physical memory address of Timer 3 CCR1 register
 */
#define TIM3_CCR1_Address 0x40000434

xSemaphoreHandle xDMASemaphore = NULL;


/* Buffer that holds one complete DMA transmission
 *
 * Ensure that this buffer is big enough to hold
 * all data bytes that need to be sent
 *
 * The buffer size can be calculated as follows:
 * number of LEDs * 24 bytes + 42 bytes
 *
 * This leaves us with a maximum string length of
 * (2^16 bytes per DMA stream - 42 bytes)/24 bytes per LED = 2728 LEDs
 */
uint16_t * DMABuffer = 0;

void DMA1_Stream4_IRQHandler ( void )
{
	long lHigherPriorityTaskWoken = pdFALSE;
	/* Test on DMA Stream Transfer Complete interrupt */
	if ( DMA_GetITStatus ( DMA1_Stream4, DMA_IT_TCIF4 ) )
	{
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit ( DMA1_Stream4, DMA_IT_TCIF4 );

		TIM_Cmd( TIM3, DISABLE ); 						// disable Timer 3
		DMA_Cmd( DMA1_Stream4, DISABLE ); 				// disable DMA stream 4

		lHigherPriorityTaskWoken = pdFALSE;
		// Unblock the task by releasing the semaphore.
		xSemaphoreGiveFromISR ( xDMASemaphore, &lHigherPriorityTaskWoken );
		portEND_SWITCHING_ISR ( lHigherPriorityTaskWoken );
	};
};

void timer_init(void)
{
	/*
	 * Timer 3 Channel 1 (using PB.04)
	 * assigned DMA = DMA1 Stream 4 Channel 5
	 */

	uint16_t PrescalerValue;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOB, ENABLE );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_PinAFConfig ( GPIOB, GPIO_PinSource4, GPIO_AF_TIM3 );
	GPIO_Init ( GPIOB, &GPIO_InitStructure );

	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_TIM3, ENABLE );
	/* Compute the prescaler value */
	PrescalerValue = ( uint16_t ) ( ( SystemCoreClock / 2 ) / 28000000 ) - 1; // timer runs on 28 MHz
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 34; // 28MHz / ( 34 + 1 ) = 800kHz pwm frequency
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig ( TIM3, TIM_OCPreload_Enable );
};

void dma_init ( )
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/* configure DMA */
	/* DMA clock enable */
	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_DMA1, ENABLE );

	/* DMA1 Stream4 Channel5 Config */
	DMA_DeInit ( DMA1_Stream4 );

	DMA_InitStructure.DMA_Channel = DMA_Channel_5;
	DMA_InitStructure.DMA_Memory0BaseAddr =  ( uint32_t ) DMABuffer;		// this is the buffer memory
	DMA_InitStructure.DMA_PeripheralBaseAddr = ( uint32_t ) TIM3_CCR1_Address;	// physical address of Timer 3 CCR1
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_BufferSize = 42;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;

	DMA_Init ( DMA1_Stream4, &DMA_InitStructure );

	/* Enable DMA Stream Transfer Complete interrupt */
	DMA_ITConfig ( DMA1_Stream4, DMA_IT_TC, ENABLE );

	/* Enable the DMA Stream IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init ( &NVIC_InitStructure );

	/* TIM3 CC1 DMA Request enable */
	TIM_DMACmd ( TIM3, TIM_DMA_CC1, ENABLE );
};

/* This function sends data bytes out to a string of WS2812s
 * The first argument is a pointer to the first RGB triplet to be sent
 * The seconds argument is the number of LEDs in the chain
 *
 * This will result in the RGB triplet passed by argument 1 being sent to
 * the LED that is the farthest away from the controller (the point where
 * data is injected into the chain)
 */
void WS2812_send ( uint8_t ( * color ) [3], uint16_t len )
{
	uint8_t j;
	uint8_t led;
	uint16_t memaddr;
	uint16_t buffersize;

	buffersize = ( len * 24 ) + 42;	// number of bytes needed is #LEDs * 24 bytes + 42 trailing bytes
	memaddr = 0;				// reset buffer memory index
	led = 0;					// reset led index

	// fill transmit buffer with correct compare values to achieve
	// correct pulse widths according to color values
	while ( len )
	{
		for ( j = 0; j < 8; j++ )					// GREEN data
		{
			if ( ( color[led][1] << j ) & 0x80 )	// data sent MSB first, j = 0 is MSB j = 7 is LSB
			{
				DMABuffer[memaddr] = CCR1_H; 	// compare value for logical 1
			}
			else
			{
				DMABuffer[memaddr] = CCR1_L;	// compare value for logical 0
			}
			memaddr++;
		}

		for ( j = 0; j < 8; j++ )					// RED data
		{
			if ( ( color[led][0] << j ) & 0x80 )	// data sent MSB first, j = 0 is MSB j = 7 is LSB
			{
				DMABuffer[memaddr] = CCR1_H; 	// compare value for logical 1
			}
			else
			{
				DMABuffer[memaddr] = CCR1_L;	// compare value for logical 0
			}
			memaddr++;
		}

		for ( j = 0; j < 8; j++ )					// BLUE data
		{
			if ( ( color[led][2] << j ) & 0x80 )	// data sent MSB first, j = 0 is MSB j = 7 is LSB
			{
				DMABuffer[memaddr] = CCR1_H; 	// compare value for logical 1
			}
			else
			{
				DMABuffer[memaddr] = CCR1_L;	// compare value for logical 0
			}
			memaddr++;
		}

		led++;
		len--;
	}

	// add needed delay at end of byte cycle, pulsewidth = 0
	while( memaddr < buffersize )
	{
		DMABuffer[memaddr] = 0;
		memaddr++;
	}

	DMA_SetCurrDataCounter ( DMA1_Stream4, buffersize ); 	// load number of bytes to be transferred
	DMA_Cmd ( DMA1_Stream4, ENABLE ); 			// enable DMA channel 6

	/*
	 * Check if the DMA Stream has been effectively enabled.
	 * The DMA Stream Enable bit is cleared immediately by hardware if there is an
	 * error in the configuration parameters and the transfer is no started (ie. when
	 * wrong FIFO threshold is configured ...)
	*/
	while ( DMA_GetCmdStatus ( DMA1_Stream4 ) != ENABLE )
	{
	}

	TIM_Cmd ( TIM3, ENABLE ); 						// enable Timer 3

	if ( pdFALSE ==  xSemaphoreTake ( xDMASemaphore, 10 / portTICK_RATE_MS ) )
	{
		// TODO DMA transaction failed
	}
};

void WS2812_init ( uint16_t led_count )
{
	DMABuffer = ( uint16_t * ) malloc ( sizeof ( * DMABuffer ) * ( led_count * 24 + 42 ) );
	vSemaphoreCreateBinary ( xDMASemaphore );
	timer_init ( );
	dma_init ( );
};
