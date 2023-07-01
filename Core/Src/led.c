#include "led.h"
#include "stm32f1xx_hal.h"

#define LED_PORT GPIOA
#define LED1_PIN GPIO_PIN_4
#define LED2_PIN GPIO_PIN_5
#define LED3_PIN GPIO_PIN_6
#define LED4_PIN GPIO_PIN_7

void led_init()
{
	GPIO_InitTypeDef gpio = {0U};

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_NOPULL;
	gpio.Pin = LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN;

	HAL_GPIO_Init(LED_PORT, &gpio);
}

void led_set(uint8_t led, bool set)
{
	if(led & LED_1)
		HAL_GPIO_WritePin(LED_PORT, LED1_PIN, set ? GPIO_PIN_SET : GPIO_PIN_RESET);
	if(led & LED_2)
		HAL_GPIO_WritePin(LED_PORT, LED2_PIN, set ? GPIO_PIN_SET : GPIO_PIN_RESET);
	if(led & LED_3)
		HAL_GPIO_WritePin(LED_PORT, LED3_PIN, set ? GPIO_PIN_SET : GPIO_PIN_RESET);
	if(led & LED_4)
		HAL_GPIO_WritePin(LED_PORT, LED4_PIN, set ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
