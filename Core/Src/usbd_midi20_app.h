/*
 * usbd_midi20_app.h
 *
 *  Created on: Jun 11, 2023
 *      Author: matt
 */

#ifndef SRC_USBD_MIDI20_APP_H_
#define SRC_USBD_MIDI20_APP_H_

#include "usbd_midi20_device.h"

extern USBD_MIDI20_Callbacks MIDI20_CALLBACKS;

void usbd_midi20_init();
void usbd_midi20_note_on(uint8_t note, uint8_t channel, uint16_t velocity);
void usbd_midi20_note_off(uint8_t note, uint8_t channel, uint16_t velocity);
void usbd_midi20_pitch_wheel(uint8_t channel, int32_t pitch);
void usbd_midi20_modulation_wheel(uint8_t channel, uint16_t modulation);
void usbd_midi20_volume(uint8_t channel, uint16_t volume);
void usbd_midi20_sense();
void usbd_midi20_sustain(uint8_t channel, bool on);
void usbd_midi20_task();

#endif
