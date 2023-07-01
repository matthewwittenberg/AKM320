#include "stm32f1xx_hal.h"

ADC_HandleTypeDef hadc1;

void adc_init()
{
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
      //Error_Handler();
    }
}

uint32_t adc_read_channel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
      //Error_Handler();
    }

    HAL_ADC_Start(&hadc1);

    while(HAL_OK != HAL_ADC_PollForConversion(&hadc1, 10000)){}
    uint32_t val = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return val;
}

uint16_t adc_get_volume()
{
    return adc_read_channel(2);
}

uint16_t adc_get_pitch()
{
    return adc_read_channel(1);
}

uint16_t adc_get_modulation()
{
    return adc_read_channel(0);
}
