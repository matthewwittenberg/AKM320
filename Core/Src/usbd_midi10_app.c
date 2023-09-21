#include "usbd_midi10_app.h"
#include "midi_spec.h"
#include "cmsis_os.h"
#include <stdbool.h>

#define TX_MESSAGE_QUEUE_DEPTH 32

extern USBD_HandleTypeDef hUsbDeviceFS;
bool _midi10_tx_busy = false;
uint32_t _midi10_tx_message_queue[TX_MESSAGE_QUEUE_DEPTH];
int32_t _midi10_tx_message_head = 0;
int32_t _midi10_tx_message_tail = 0;

osSemaphoreDef(_midi10_semaphore);
osSemaphoreId(_midi10_semaphore_id);

void midi10_setup_rx(uint8_t *psetup, uint16_t length)
{
}

void midi10_message_rx(uint32_t message)
{
}

void midi10_message_tx_complete()
{
    if(_midi10_tx_message_head != _midi10_tx_message_tail)
    {
        _midi10_tx_busy = true;
        USBD_MIDI10_SendMessage(&hUsbDeviceFS, _midi10_tx_message_queue[_midi10_tx_message_tail]);
        _midi10_tx_message_tail++;
        if(_midi10_tx_message_tail >= TX_MESSAGE_QUEUE_DEPTH)
            _midi10_tx_message_tail = 0;
    }
    else
    {
        _midi10_tx_busy = false;
    }
}

void usbd_midi10_send_message(uint32_t message)
{
    osSemaphoreWait(_midi10_semaphore_id, osWaitForever);

    _midi10_tx_message_queue[_midi10_tx_message_head] = message;
    _midi10_tx_message_head++;
    if(_midi10_tx_message_head >= TX_MESSAGE_QUEUE_DEPTH)
        _midi10_tx_message_head = 0;

    osSemaphoreRelease(_midi10_semaphore_id);

    if(_midi10_tx_busy == false)
    {
        midi10_message_tx_complete();
    }
}

void usbd_midi10_init()
{
    _midi10_semaphore_id = osSemaphoreCreate(osSemaphore(_midi10_semaphore), 1);
}

void usbd_midi10_note_on(uint8_t note, uint8_t channel, uint16_t velocity)
{
    uint32_t message = CIN_NOTE_ON | ((MIDI_NOTE_ON | channel) << 8) | (note << 16) | (velocity << 24);
    usbd_midi10_send_message(message);
}

void usbd_midi10_note_off(uint8_t note, uint8_t channel, uint16_t velocity)
{
    uint32_t message = CIN_NOTE_OFF | ((MIDI_NOTE_OFF | channel) << 8) | (note << 16);
    usbd_midi10_send_message(message);
}

void usbd_midi10_pitch_wheel(uint8_t channel, int32_t pitch)
{
    pitch = (pitch + 8192) & 0x3FFF;
    uint32_t pitch_new = (pitch & 0x7F) | ((pitch << 1) & 0x7F00);
    uint32_t message = CIN_PITCH_BEND | ((MIDI_PITCH_WHEEL | channel) << 8) | (pitch_new << 16);
    usbd_midi10_send_message(message);
}

void usbd_midi10_modulation_wheel(uint8_t channel, uint16_t modulation)
{
    modulation = modulation & 0x3FFF;

    uint32_t fine = modulation & 0x7F;
    uint32_t message = CIN_CONTROL_CHANGE | ((MIDI_CONTROLLER | channel) << 8) | (MIDI_CONT_MOD_WHEEL_FINE << 16) | (fine << 24);
    usbd_midi10_send_message(message);

    uint32_t course = (modulation >> 7) & 0x7F;
    message = CIN_CONTROL_CHANGE | ((MIDI_CONTROLLER | channel) << 8) | (MIDI_CONT_MOD_WHEEL_COARSE << 16) | (course << 24);
    usbd_midi10_send_message(message);
}

void usbd_midi10_volume(uint8_t channel, uint16_t volume)
{
    volume = volume & 0x3FFF;

    uint32_t fine = volume & 0x7F;
    uint32_t message = CIN_CONTROL_CHANGE | ((MIDI_CONTROLLER | channel) << 8) | (MIDI_CONT_VOLUME_FINE << 16) | (fine << 24);
    usbd_midi10_send_message(message);

    uint32_t course = (volume >> 7) & 0x7F;
    message = CIN_CONTROL_CHANGE | ((MIDI_CONTROLLER | channel) << 8) | (MIDI_CONT_VOLUME_COARSE << 16) | (course << 24);
    usbd_midi10_send_message(message);
}

void usbd_midi10_sense()
{
    uint32_t message = CIN_SINGLE_BYTE | (MIDI_SENSE << 8);
    usbd_midi10_send_message(message);
}

void usbd_midi10_sustain(uint8_t channel, bool on)
{
    uint32_t hold_value = on ? 0x7F : 0;
    uint32_t message = CIN_CONTROL_CHANGE | ((MIDI_CONTROLLER | channel) << 8) | (MIDI_CONT_HOLD_PEDAL << 16) | (hold_value << 24);
    usbd_midi10_send_message(message);
}

void usbd_midi10_task()
{
}

USBD_MIDI10_Callbacks MIDI10_CALLBACKS =
{
    midi10_setup_rx,
    midi10_message_rx,
    midi10_message_tx_complete
};


