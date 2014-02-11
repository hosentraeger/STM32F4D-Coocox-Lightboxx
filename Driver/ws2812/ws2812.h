#ifndef __WS2812_H__
#define __WS2812_H__

/*
 * FreeRTOS includes
 */
#include "FreeRTOS.h"
#include "semphr.h"

void WS2812_init ( uint16_t led_count );
void WS2812_send(uint8_t (*color)[3], uint16_t len);

#endif // __WS2812_H__
