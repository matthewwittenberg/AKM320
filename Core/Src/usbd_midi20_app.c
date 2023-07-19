#include "usbd_midi20_app.h"
#include "midi_spec.h"
#include "cmsis_os.h"
#include "midi20_ci.h"
#include <stdbool.h>

#define TX_MESSAGE_QUEUE_DEPTH 512
#define RX_MESSAGE_QUEUE_DEPTH 512
#define SYSEX_BUFFER_LENGTH 128

typedef enum
{
    SYSEX_STATUS_NONE,
    SYSEX_STATUS_IN,
} SYSEX_STATUS_T;

extern USBD_HandleTypeDef hUsbDeviceFS;
bool _midi20_tx_busy = false;
uint8_t _midi20_tx_message_queue[TX_MESSAGE_QUEUE_DEPTH];
int32_t _midi20_tx_message_head = 0;
int32_t _midi20_tx_message_tail = 0;
uint8_t _midi20_rx_message_queue[RX_MESSAGE_QUEUE_DEPTH];
int32_t _midi20_rx_message_head = 0;
int32_t _midi20_rx_message_tail = 0;
uint8_t _midi20_sysex_buffer[SYSEX_BUFFER_LENGTH];
uint32_t _midi20_sysex_buffer_index = 0;

osSemaphoreDef(_midi20_semaphore);
osSemaphoreId(_midi20_semaphore_id);

void midi20_message_rx(uint8_t *pmessage, uint32_t length)
{
    int32_t temp_head = _midi20_rx_message_head;

    for(int32_t i=0; i<length; i++)
    {
        _midi20_rx_message_queue[temp_head] = pmessage[i];
        temp_head++;
        if(temp_head >= RX_MESSAGE_QUEUE_DEPTH)
            temp_head = 0;
    }

    // we are async, wait to update the head until the packet has been copied
    _midi20_rx_message_head = temp_head;
}

void midi20_message_tx_complete()
{
    if(_midi20_tx_message_head != _midi20_tx_message_tail)
    {
        _midi20_tx_busy = true;

        uint8_t message_length = 0;
        uint8_t message_type = _midi20_tx_message_queue[_midi20_tx_message_tail] & 0xF0;
        switch(message_type)
        {
            case MIDI20_MESSAGE_TYPE_UTILITY:
            case MIDI20_MESSAGE_TYPE_SYSTEM:
            case MIDI20_MESSAGE_TYPE_10_CHANNEL_VOICE:
                message_length = 4;
                break;
            case MIDI20_MESSAGE_TYPE_DATA64:
            case MIDI20_MESSAGE_TYPE_20_CHANNEL_VOICE:
                message_length = 8;
                break;
            case MIDI20_MESSAGE_TYPE_DATA128:
                message_length = 16;
                break;
        }

        uint8_t message[MIDI20_MESSAGE_LENGTH];
        for(int32_t i=0; i<message_length; i++)
        {
            message[i] = _midi20_tx_message_queue[_midi20_tx_message_tail];
            _midi20_tx_message_tail++;
            if(_midi20_tx_message_tail >= TX_MESSAGE_QUEUE_DEPTH)
                _midi20_tx_message_tail = 0;
        }

        if(message_length > 0)
            USBD_MIDI20_SendMessage(&hUsbDeviceFS, message, message_length);
    }
    else
    {
        _midi20_tx_busy = false;
    }
}

void usbd_midi20_send_message(uint8_t *pmessage, uint32_t length)
{
    osSemaphoreWait(_midi20_semaphore_id, osWaitForever);

    int32_t temp_head = _midi20_tx_message_head;

    for(int32_t i=0; i<length; i++)
    {
        _midi20_tx_message_queue[temp_head] = pmessage[i];
        temp_head++;
        if(temp_head >= TX_MESSAGE_QUEUE_DEPTH)
            temp_head = 0;
    }

    // we are async, wait to update the head until the packet has been copied
    _midi20_tx_message_head = temp_head;

    osSemaphoreRelease(_midi20_semaphore_id);

    if(_midi20_tx_busy == false)
    {
        midi20_message_tx_complete();
    }
}

void usbd_midi20_init()
{
    _midi20_semaphore_id = osSemaphoreCreate(osSemaphore(_midi20_semaphore), 1);
}

void usbd_midi20_note_on(uint8_t note, uint8_t channel, uint16_t velocity)
{
    uint8_t message[8];
    message[0] = MIDI20_MESSAGE_TYPE_20_CHANNEL_VOICE;
    message[1] = MIDI_NOTE_ON | channel;
    message[2] = note;
    message[3] = 0; // no attribute
    message[4] = velocity;
    message[5] = velocity >> 8;
    message[6] = 0; // attribute lsb
    message[7] = 0; // attribute msb
    usbd_midi20_send_message(message, sizeof(message));
}

void usbd_midi20_note_off(uint8_t note, uint8_t channel, uint16_t velocity)
{
    uint8_t message[8];
    message[0] = MIDI20_MESSAGE_TYPE_20_CHANNEL_VOICE;
    message[1] = MIDI_NOTE_OFF | channel;
    message[2] = note;
    message[3] = 0; // no attribute
    message[4] = velocity;
    message[5] = velocity >> 8;
    message[6] = 0; // attribute lsb
    message[7] = 0; // attribute msb
    usbd_midi20_send_message(message, sizeof(message));
}

void usbd_midi20_pitch_wheel(uint8_t channel, int32_t pitch)
{
    pitch += 0x80000000;

    uint8_t message[8];
    message[0] = MIDI20_MESSAGE_TYPE_20_CHANNEL_VOICE;
    message[1] = MIDI_PITCH_WHEEL | channel;
    message[2] = 0;
    message[3] = 0;
    message[4] = pitch;
    message[5] = pitch >> 8;
    message[6] = pitch >> 16;
    message[7] = pitch >> 24;
    usbd_midi20_send_message(message, sizeof(message));
}

void usbd_midi20_modulation_wheel(uint8_t channel, uint16_t modulation)
{
    modulation = modulation & 0x3FFF;

    uint8_t message[8];
    message[0] = MIDI20_MESSAGE_TYPE_20_CHANNEL_VOICE;
    message[1] = MIDI_CONTROLLER | channel;
    message[2] = MIDI_CONT_MOD_WHEEL_COARSE;
    message[3] = 0;
    message[4] = modulation;
    message[5] = modulation >> 8;
    message[6] = 0;
    message[7] = 0;
    usbd_midi20_send_message(message, sizeof(message));
}

void usbd_midi20_volume(uint8_t channel, uint16_t volume)
{
    volume = volume & 0x3FFF;

    uint8_t message[8];
    message[0] = MIDI20_MESSAGE_TYPE_20_CHANNEL_VOICE;
    message[1] = MIDI_CONTROLLER | channel;
    message[2] = MIDI_CONT_VOLUME_COARSE;
    message[3] = 0;
    message[4] = volume;
    message[5] = volume >> 8;
    message[6] = 0;
    message[7] = 0;
    usbd_midi20_send_message(message, sizeof(message));

    // just for testing a 128 bit message
//    uint8_t message[16];
//    message[0] = MIDI20_MESSAGE_TYPE_DATA128;
//    message[1] = MIDI_CONTROLLER | channel;
//    message[2] = MIDI_CONT_VOLUME_COARSE;
//    message[3] = 0;
//    message[4] = volume;
//    message[5] = volume >> 8;
//    message[6] = 0;
//    message[7] = 0;
//    message[8] = 0;
//    message[9] = 0;
//    message[10] = 0;
//    message[11] = 0;
//    message[12] = 0;
//    message[13] = 0;
//    message[14] = 0;
//    message[15] = 0;
//    usbd_midi20_send_message(message, sizeof(message));
}

void usbd_midi20_sense()
{
    uint8_t message[4];
    message[0] = MIDI20_MESSAGE_TYPE_SYSTEM;
    message[1] = MIDI_SENSE;
    message[2] = 0;
    message[3] = 0;
    usbd_midi20_send_message(message, sizeof(message));
}

void usbd_midi20_sustain(uint8_t channel, bool on)
{
    uint32_t hold_value = on ? 0x7F : 0;

    uint8_t message[8];
    message[0] = MIDI20_MESSAGE_TYPE_20_CHANNEL_VOICE;
    message[1] = MIDI_CONTROLLER | channel;
    message[2] = MIDI_CONT_HOLD_PEDAL;
    message[3] = 0;
    message[4] = hold_value;
    message[5] = hold_value >> 8;
    message[6] = 0;
    message[7] = 0;
    usbd_midi20_send_message(message, sizeof(message));
}

void usbd_midi20_ci_process_handler(uint8_t *pmessage, uint32_t length)
{
    uint32_t index = 0;
    uint8_t message[8];
    uint32_t total_messages = length / 6;
    uint32_t i, j;

    if(length % 6)
        total_messages++;

    // build usb packet chunks to send
    for(i=0; i<total_messages; i++)
    {
        message[0] = MIDI20_MESSAGE_TYPE_DATA64;

        if(total_messages == 1)
        {
            message[1] = MIDI20_SYSEX_STATUS_COMPLETE_IN_1;
        }
        else if(i == 0)
        {
            message[1] = MIDI20_SYSEX_STATUS_START;
        }
        else if(i < (total_messages - 1))
        {
            message[1] = MIDI20_SYSEX_STATUS_CONTINUE;
        }
        else
        {
            message[1] = MIDI20_SYSEX_STATUS_STOP;
        }

        for(j=0; j<6 && index<length; j++)
        {
            message[2+j] = pmessage[index++];
        }
        message[1] |= (j & 0x0F);

        usbd_midi20_send_message(message, sizeof(message));
    }
}

void usbd_midi20_process_sysex()
{
    // pass to ci manager
    midi20_ci_process(_midi20_sysex_buffer, _midi20_sysex_buffer_index, usbd_midi20_ci_process_handler);
}

void usbd_midi20_task()
{
    // process received messages
    while(_midi20_rx_message_head != _midi20_rx_message_tail)
    {
        uint8_t message_length = 0;
        uint8_t message_type = _midi20_rx_message_queue[_midi20_rx_message_tail] & 0xF0;
        switch(message_type)
        {
            case MIDI20_MESSAGE_TYPE_UTILITY:
            case MIDI20_MESSAGE_TYPE_SYSTEM:
            case MIDI20_MESSAGE_TYPE_10_CHANNEL_VOICE:
                message_length = 4;
                break;
            case MIDI20_MESSAGE_TYPE_DATA64:
            case MIDI20_MESSAGE_TYPE_20_CHANNEL_VOICE:
                message_length = 8;
                break;
            case MIDI20_MESSAGE_TYPE_DATA128:
                message_length = 16;
                break;
        }

        if(message_length == 0)
        {
            _midi20_rx_message_tail = _midi20_rx_message_head;
            break;
        }

        uint8_t message[MIDI20_MESSAGE_LENGTH];
        for(int32_t i=0; i<message_length; i++)
        {
            message[i] = _midi20_rx_message_queue[_midi20_rx_message_tail];
            _midi20_rx_message_tail++;
            if(_midi20_rx_message_tail >= RX_MESSAGE_QUEUE_DEPTH)
                _midi20_rx_message_tail = 0;
        }

        // check for sysex, otherwise eat the message
        if(message_type == MIDI20_MESSAGE_TYPE_DATA64)
        {
            uint8_t status = message[1] & 0xF0;
            uint8_t byte_count = message[1] & 0x0F;

            if(byte_count > 6)
                byte_count = 6;

            for(uint8_t i=0; i<byte_count; i++)
            {
                _midi20_sysex_buffer[_midi20_sysex_buffer_index] = message[i+2];
                _midi20_sysex_buffer_index++;
                if(_midi20_sysex_buffer_index >= SYSEX_BUFFER_LENGTH)
                    _midi20_sysex_buffer_index = 0;
            }

            if((status == MIDI20_SYSEX_STATUS_COMPLETE_IN_1) || (status == MIDI20_SYSEX_STATUS_STOP))
            {
                usbd_midi20_process_sysex();
                _midi20_sysex_buffer_index = 0;
            }
        }
    }
}

USBD_MIDI20_Callbacks MIDI20_CALLBACKS =
{
    midi20_message_rx,
    midi20_message_tx_complete
};
