#ifndef SRC_LED_H_
#define SRC_LED_H_


#include <stdint.h>
#include <stdbool.h>

#define LED_1 0x01
#define LED_2 0x02
#define LED_3 0x04
#define LED_4 0x08

void led_init();
void led_set(uint8_t led, bool set);


#endif /* SRC_LED_H_ */
