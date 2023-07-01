#ifndef SRC_ADC_H_
#define SRC_ADC_H_

#include <stdint.h>
#include <stdbool.h>

void adc_init();
uint16_t adc_get_volume();
uint16_t adc_get_pitch();
uint16_t adc_get_modulation();

#endif
