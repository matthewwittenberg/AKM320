#ifndef SRC_KEYBOARD_H_
#define SRC_KEYBOARD_H_

#include <stdint.h>
#include <stdbool.h>

#define KEYBOARD_TOTAL_KEYS 32
#define KEYBOARD_NOTES_PER_OCTAVE 12
#define KEYBOARD_START_NOTE 53
#define KEYBOARD_CHANNEL 0

void keyboard_init();
uint8_t keyboard_get_start_note();
void keyboard_set_start_note(uint8_t note);

#endif
