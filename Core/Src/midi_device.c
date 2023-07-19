#include "midi_device.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_midi10_device.h"
#include "usbd_midi10_app.h"
#include "usbd_midi20_device.h"
#include "usbd_midi20_app.h"

USBD_MIDI_DEVICE_T MIDI_DEVICE;

void midi_device_init(USBD_HandleTypeDef *pdev)
{
#if MIDI_VERSION == 1
    if (USBD_RegisterClass(pdev, &USBD_MIDI10) != USBD_OK)
    {
      Error_Handler();
    }
    if (USBD_MIDI10_RegisterInterface(pdev, &MIDI10_CALLBACKS) != USBD_OK)
    {
      Error_Handler();
    }

    MIDI_DEVICE.init = usbd_midi10_init;
    MIDI_DEVICE.note_on = usbd_midi10_note_on;
    MIDI_DEVICE.note_off = usbd_midi10_note_off;
    MIDI_DEVICE.pitch_wheel = usbd_midi10_pitch_wheel;
    MIDI_DEVICE.modulation_wheel = usbd_midi10_modulation_wheel;
    MIDI_DEVICE.volume = usbd_midi10_volume;
    MIDI_DEVICE.sense = usbd_midi10_sense;
    MIDI_DEVICE.sustain = usbd_midi10_sustain;
    MIDI_DEVICE.task = usbd_midi10_task;
    MIDI_DEVICE.version = MIDI_VERSION_1_0;
#else
    if (USBD_RegisterClass(pdev, &USBD_MIDI20) != USBD_OK)
    {
      Error_Handler();
    }
    if (USBD_MIDI20_RegisterInterface(pdev, &MIDI20_CALLBACKS) != USBD_OK)
    {
      Error_Handler();
    }

    MIDI_DEVICE.init = usbd_midi20_init;
    MIDI_DEVICE.note_on = usbd_midi20_note_on;
    MIDI_DEVICE.note_off = usbd_midi20_note_off;
    MIDI_DEVICE.pitch_wheel = usbd_midi20_pitch_wheel;
    MIDI_DEVICE.modulation_wheel = usbd_midi20_modulation_wheel;
    MIDI_DEVICE.volume = usbd_midi20_volume;
    MIDI_DEVICE.sense = usbd_midi20_sense;
    MIDI_DEVICE.sustain = usbd_midi20_sustain;
    MIDI_DEVICE.task = usbd_midi20_task;
    MIDI_DEVICE.version = MIDI_VERSION_2_0;
#endif

    MIDI_DEVICE.init();
}




