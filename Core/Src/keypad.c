#include "keypad.h"
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#define KEYPAD_MAX_CALLBACKS 5
#define KEY_TOTAL 4
#define KEY_DEBOUNCE_TIME 100

#define KEY_OCTAVE_PLUS_PORT GPIOC
#define KEY_OCTAVE_PLUS_PIN GPIO_PIN_13
#define KEY_OCTAVE_MINUS_PORT GPIOC
#define KEY_OCTAVE_MINUS_PIN GPIO_PIN_14
#define KEY_TRANSPOSE_PLUS_PORT GPIOC
#define KEY_TRANSPOSE_PLUS_PIN GPIO_PIN_15
#define KEY_TRANSPOSE_MINUS_PORT GPIOA
#define KEY_TRANSPOSE_MINUS_PIN GPIO_PIN_15

typedef struct
{
	key_callback callback;
	void *param;
} KEY_CALLBACK_T;

typedef struct
{
	KEY_T key;
	GPIO_TypeDef *port;
	uint16_t pin;
	KEY_STATE_T state;
	uint32_t press_tick;
} KEY_INFO_T;

static KEY_CALLBACK_T _callbacks[KEYPAD_MAX_CALLBACKS];
static osThreadId _keypad_task_handle = NULL;
static KEY_INFO_T _key_states[KEY_TOTAL];

void keypad_init_pin(GPIO_TypeDef *port, uint16_t pin)
{
	GPIO_InitTypeDef gpio = {0U};

	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio.Pull = GPIO_PULLUP;
	gpio.Pin = pin;

	HAL_GPIO_Init(port, &gpio);
}

void keypad_task(void const *param)
{
	for(;;)
	{
		for(uint32_t i=0; i<KEY_TOTAL; i++)
		{
			if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(_key_states[i].port, _key_states[i].pin))
			{
				if(_key_states[i].state == KEY_RELEASE)
				{
					_key_states[i].state = KEY_DETECT;
					_key_states[i].press_tick = osKernelSysTick();
				}
				else if(_key_states[i].state == KEY_DETECT)
				{
					if(_key_states[i].press_tick + KEY_DEBOUNCE_TIME < osKernelSysTick())
					{
						_key_states[i].state = KEY_PRESS;

						for(uint32_t j=0; j<KEYPAD_MAX_CALLBACKS; j++)
						{
							if(_callbacks[j].callback)
							{
								_callbacks[j].callback(_key_states[i].key, KEY_PRESS, _callbacks[j].param);
							}
						}
					}
				}
			}
			else
			{
				if(_key_states[i].state == KEY_PRESS)
				{
					for(uint32_t j=0; j<KEYPAD_MAX_CALLBACKS; j++)
					{
						if(_callbacks[j].callback)
						{
							_callbacks[j].callback(_key_states[i].key, KEY_RELEASE, _callbacks[j].param);
						}
					}
				}

				_key_states[i].state = KEY_RELEASE;
			}
		}

		osDelay(10);
	}
}

void keypad_init()
{
	memset(_callbacks, 0, sizeof(_callbacks));
	memset(_key_states, 0, sizeof(_key_states));

	_key_states[0].key = KEY_OCTAVE_PLUS;
	_key_states[0].port = KEY_OCTAVE_PLUS_PORT;
	_key_states[0].pin = KEY_OCTAVE_PLUS_PIN;
	_key_states[0].press_tick = 0;
	_key_states[0].state = KEY_RELEASE;

	_key_states[1].key = KEY_OCTAVE_MINUS;
	_key_states[1].port = KEY_OCTAVE_MINUS_PORT;
	_key_states[1].pin = KEY_OCTAVE_MINUS_PIN;
	_key_states[1].press_tick = 0;
	_key_states[1].state = KEY_RELEASE;

	_key_states[2].key = KEY_TRANSPOSE_PLUS;
	_key_states[2].port = KEY_TRANSPOSE_PLUS_PORT;
	_key_states[2].pin = KEY_TRANSPOSE_PLUS_PIN;
	_key_states[2].press_tick = 0;
	_key_states[2].state = KEY_RELEASE;

	_key_states[3].key = KEY_TRANSPOSE_MINUS;
	_key_states[3].port = KEY_TRANSPOSE_MINUS_PORT;
	_key_states[3].pin = KEY_TRANSPOSE_MINUS_PIN;
	_key_states[3].press_tick = 0;
	_key_states[3].state = KEY_RELEASE;

	for(uint32_t i=0; i<KEY_TOTAL; i++)
	{
		keypad_init_pin(_key_states[i].port, _key_states[i].pin);
	}

	osThreadDef(defaultTask, keypad_task, osPriorityNormal, 0, 128);
	_keypad_task_handle = osThreadCreate(osThread(defaultTask), NULL);
}

void keypad_register_callback(key_callback callback, void *param)
{
	// check if already registered
	for(uint32_t i=0; i<KEYPAD_MAX_CALLBACKS; i++)
	{
		if(callback == _callbacks[i].callback)
		{
			return;
		}
	}

	// find empty slot and register
	for(uint32_t i=0; i<KEYPAD_MAX_CALLBACKS; i++)
	{
		if(_callbacks[i].callback == NULL)
		{
			_callbacks[i].callback = callback;
			_callbacks[i].param = param;
			break;
		}
	}
}

void keypad_deregister_callback(key_callback callback)
{
	// check if already registered
	for(uint32_t i=0; i<KEYPAD_MAX_CALLBACKS; i++)
	{
		if(callback == _callbacks[i].callback)
		{
			_callbacks[i].callback = NULL;
			break;
		}
	}
}

KEY_T keypad_get_key()
{
	KEY_T key = KEY_NONE;

	for(uint32_t i=0; i<KEY_TOTAL; i++)
	{
		if(_key_states[i].state == KEY_PRESS)
		{
			key |= _key_states[i].key;
		}
	}

	return key;
}
