#ifndef SRC_USBD_MIDI10_DEVICE_H_
#define SRC_USBD_MIDI10_DEVICE_H_

#include <stdint.h>
#include "usbd_ioreq.h"

#define RX_BUFFER_SIZE 16

typedef struct
{
    void (*setup_rx)(uint8_t *psetup, uint16_t length);
    void (*message_rx)(uint32_t message);
    void (*message_tx_complete)(void);
} USBD_MIDI10_Callbacks;

typedef struct
{
    uint8_t rx_buf[RX_BUFFER_SIZE];
} USBD_MIDI10_ClassData;

uint8_t USBD_MIDI10_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI10_Callbacks *pcallbacks);
uint8_t USBD_MIDI10_SendMessage(USBD_HandleTypeDef *pdev, uint32_t message);

extern USBD_ClassTypeDef USBD_MIDI10;

#endif
