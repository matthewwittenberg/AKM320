#ifndef SRC_KEYPAD_H_
#define SRC_KEYPAD_H_


#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	KEY_NONE = 0x00,
	KEY_OCTAVE_PLUS = 0x01,
	KEY_OCTAVE_MINUS = 0x02,
	KEY_TRANSPOSE_PLUS = 0x04,
	KEY_TRANSPOSE_MINUS = 0x08,
} KEY_T;

typedef enum
{
	KEY_PRESS,
	KEY_RELEASE,
	KEY_DETECT,
} KEY_STATE_T;

typedef void (*key_callback)(KEY_T key, KEY_STATE_T state, void *param);

void keypad_init();
void keypad_register_callback(key_callback callback, void *param);
void keypad_deregister_callback(key_callback callback);
KEY_T keypad_get_key();

#endif
