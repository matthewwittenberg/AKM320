#ifndef SRC_USBD_MIDI10_APP_H_
#define SRC_USBD_MIDI10_APP_H_

#include "usbd_midi10_device.h"

extern USBD_MIDI10_Callbacks MIDI10_CALLBACKS;

void usbd_midi10_init();
void usbd_midi10_note_on(uint8_t note, uint8_t channel, uint16_t velocity);
void usbd_midi10_note_off(uint8_t note, uint8_t channel, uint16_t velocity);
void usbd_midi10_pitch_wheel(uint8_t channel, int32_t pitch);
void usbd_midi10_modulation_wheel(uint8_t channel, uint16_t modulation);
void usbd_midi10_volume(uint8_t channel, uint16_t volume);
void usbd_midi10_sense();
void usbd_midi10_sustain(uint8_t channel, bool on);
void usbd_midi10_task();

#endif
