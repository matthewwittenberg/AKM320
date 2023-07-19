/*
 * sustain.c
 *
 *  Created on: Jul 19, 2023
 *      Author: matt
 */

#include "sustain.h"
#include "stm32f1xx_hal.h"

#define SUSTAIN_PORT GPIOA
#define SUSTAIN_PIN GPIO_PIN_3

void sustain_init()
{
    GPIO_InitTypeDef gpio = {0U};

    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin = SUSTAIN_PIN;

    HAL_GPIO_Init(SUSTAIN_PORT, &gpio);
}

bool sustain_get()
{
    if(GPIO_PIN_SET == HAL_GPIO_ReadPin(SUSTAIN_PORT, SUSTAIN_PIN))
        return false;

    return true;
}


