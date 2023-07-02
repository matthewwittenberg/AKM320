#include "keyboard.h"
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include "midi_device.h"
#include "midi_spec.h"
#include <string.h>
#include <math.h>

#define KEYBOARD_PORT GPIOB
#define KEYBOARD_TIMER TIM1
#define KEYBOARD_DELAY_LOOP_COUNT 20

typedef enum
{
	KEYBOARD_KEY_RELEASE,
	KEYBOARD_KEY_DETECT,
	KEYBOARD_KEY_PRESS,
} KEYBOARD_PRESS_STATUS_T;

typedef struct
{
	KEYBOARD_PRESS_STATUS_T status;
	uint32_t detect_tick;
	uint16_t velocity;
} KEYBOARD_KEY_STATUS_T;

static osThreadId _keyboard_task_handle = NULL;
static KEYBOARD_KEY_STATUS_T _key_status[KEYBOARD_TOTAL_KEYS];
volatile uint32_t _keyboard_timer_tick = 0;
static uint8_t _keyboard_start_note = KEYBOARD_START_NOTE;

/**
 * @brief Timer 1 ISR
 *
 */
void TIM1_CC_IRQHandler(void)
{
	if (KEYBOARD_TIMER->SR & TIM_SR_CC1IF)
	{
	    // clear the interrupt flag
		KEYBOARD_TIMER->SR &= ~TIM_SR_CC1IF;

		// increment the count
		++_keyboard_timer_tick;
	}
}

/**
 * @brief Setup timer 1
 * Used to measure time between initial press and full press of each individual key
 *
 */
void keyboard_setup_timer()
{
	KEYBOARD_TIMER->SMCR &= ~TIM_SMCR_SMS;

	KEYBOARD_TIMER->PSC = 1;
	KEYBOARD_TIMER->ARR = 240;
	KEYBOARD_TIMER->CR1 = TIM_CR1_CEN;

	KEYBOARD_TIMER->DIER |= TIM_DIER_CC1IE;

	NVIC_SetPriority(TIM1_CC_IRQn, 0);
	NVIC_EnableIRQ(TIM1_CC_IRQn);
}

/**
 * @brief Notify MIDI of a note on message
 * NOTE: this is linear for now, should consider a better response curve
 *
 * @param note
 * @param velocity
 */
void keyboard_note_on(uint32_t note, uint32_t velocity)
{
    uint32_t velocity_adjusted;

    if(velocity > 10100)
        velocity = 10100;
    if(velocity < 100)
        velocity = 100;

    // make 0-10000 range
    velocity -= 100;

    if(MIDI_DEVICE.version == MIDI_VERSION_1_0)
    {
        // convert to 0-127 range (7 bit)
        velocity_adjusted = 0x7F & (0x7F - (velocity/78));
    }
    else
    {
        // convert to 0-60000 range (16 bit)
        velocity_adjusted = 65535 - (velocity * 6);
    }

    MIDI_DEVICE.note_on(note + _keyboard_start_note, KEYBOARD_CHANNEL, velocity_adjusted);
}

/**
 * @brief Notify MIDI of a note off message
 *
 * @param note
 */
void keyboard_note_off(uint32_t note)
{
    MIDI_DEVICE.note_off(note + _keyboard_start_note, KEYBOARD_CHANNEL, 0);
}

/**
 * @brief Scan the keys on a particular detect/press pin
 * There are 8 groups of 4 keys. Scan for key pressed by checking the detect/press inputs while
 * individually setting the group pins low. This allows for detecting multiple key presses at the
 * same time (like a chord).
 *
 * @param press_pin_mask
 * @param detect_pin_mask
 * @param key
 */
static inline void keyboard_scan_key(uint16_t press_pin_mask, uint16_t detect_pin_mask, uint8_t key)
{
	uint16_t keys;
	uint8_t group;
	uint8_t group_mask;
	uint8_t key_index;
	volatile uint32_t delay;

	// read detect/down switches
	keys = KEYBOARD_PORT->IDR & 0xFF00;

	// if any detect/down switches are pressed
	if((keys & detect_pin_mask) == 0x0000)
	{
	    // all group pins high
		KEYBOARD_PORT->ODR |= 0xFF;

		// find the groups
		group = 0;
		group_mask = 0x01;
		while(group_mask)
		{
		    // clear one group pin
			KEYBOARD_PORT->ODR &= ~group_mask;
			// delay is important to filter out cross talk
			delay = KEYBOARD_DELAY_LOOP_COUNT;
			while(--delay){}

			key_index = key + group;

			keys = KEYBOARD_PORT->IDR & 0xFF00;

			// check to see if the detect pin is pressed
			if((keys & detect_pin_mask) == 0x0000)
			{
			    // if currently released, note the timer time and go to detect phase
				if(_key_status[key_index].status == KEYBOARD_KEY_RELEASE)
				{
					_key_status[key_index].status = KEYBOARD_KEY_DETECT;
					_key_status[key_index].detect_tick = _keyboard_timer_tick;
				}
				else if(_key_status[key_index].status == KEYBOARD_KEY_DETECT)
				{
				    // check to see if the key is entirely pressed, if so, calculate total timer time and notify of a note on
					if((keys & press_pin_mask) == 0x0000)
					{
						_key_status[key_index].status = KEYBOARD_KEY_PRESS;
						_key_status[key_index].velocity = _keyboard_timer_tick - _key_status[key_index].detect_tick;

						keyboard_note_on(key_index, _key_status[key_index].velocity);
					}
				}
			}
			// is released
			else
			{
			    // if we are currently pressed, send the note off
				if(_key_status[key_index].status == KEYBOARD_KEY_PRESS)
				{
				    keyboard_note_off(key_index);
				}

				_key_status[key_index].status = KEYBOARD_KEY_RELEASE;
			}

			// delay is important to filter out cross talk
			KEYBOARD_PORT->ODR |= group_mask;
	        delay = KEYBOARD_DELAY_LOOP_COUNT;
	        while(--delay){}

	        // shift to the next group
			group_mask = group_mask << 1;
			group += 4;
		}

		KEYBOARD_PORT->ODR &= ~0xFF;
	}
	// no detect/press switches are pressed
	else
	{
		group = 0;
		group_mask = 0x01;
		while(group_mask)
		{
			key_index = key + group;

			// if we are currently pressed, send the note off
			if(_key_status[key_index].status == KEYBOARD_KEY_PRESS)
			{
			    keyboard_note_off(key_index);
			}

			_key_status[key_index].status = KEYBOARD_KEY_RELEASE;

			group_mask = group_mask << 1;
			group += 4;
		}
	}
}

/**
 * @brief Keyboard scan as fast as possible
 *
 * @param param
 */
void keyboard_task(void const *param)
{
	while(1)
	{
	    // scan key 1
		keyboard_scan_key(0x0100, 0x0200, 0);
		// scan key 2
		keyboard_scan_key(0x0400, 0x0800, 1);
		// scan key 3
		keyboard_scan_key(0x1000, 0x2000, 2);
		// scan key 4
		keyboard_scan_key(0x4000, 0x8000, 3);

		osDelay(1);
	}
}

/**
 * @brief Initialize keyboard module
 *
 */
void keyboard_init()
{
	GPIO_InitTypeDef gpio = {0U};

	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_NOPULL;
	gpio.Pin = 0xFF00;
	HAL_GPIO_Init(KEYBOARD_PORT, &gpio);

	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_NOPULL;
	gpio.Pin = 0x00FF;
	HAL_GPIO_Init(KEYBOARD_PORT, &gpio);

	HAL_GPIO_WritePin(KEYBOARD_PORT, 0x00FF, GPIO_PIN_RESET);

	memset(_key_status, 0, sizeof(_key_status));

	keyboard_setup_timer();

	osThreadDef(defaultTask, keyboard_task, osPriorityHigh, 0, 256);
	_keyboard_task_handle = osThreadCreate(osThread(defaultTask), NULL);
}

/**
 * @brief Get start note
 *
 * @return Start note (first key on keyboard)
 */
uint8_t keyboard_get_start_note()
{
    return _keyboard_start_note;
}

/**
 * @brief Set start note
 *
 * @param note Start note (first key on keyboard)
 */
void keyboard_set_start_note(uint8_t note)
{
    _keyboard_start_note = note;
}
