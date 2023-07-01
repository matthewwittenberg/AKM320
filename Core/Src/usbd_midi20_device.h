/*
 * usbd_midi20_device.h
 *
 *  Created on: Jun 11, 2023
 *      Author: matt
 */

#ifndef SRC_USBD_MIDI20_DEVICE_H_
#define SRC_USBD_MIDI20_DEVICE_H_

#include <stdint.h>
#include "usbd_ioreq.h"

#define MIDI20_MESSAGE_LENGTH 16
#define MIDI20_RX_LENGTH 64

typedef struct
{
    void (*message_rx)(uint8_t *pmessage, uint32_t length);
    void (*message_tx_complete)(void);
} USBD_MIDI20_Callbacks;

typedef struct
{
    uint8_t rx_buf[MIDI20_RX_LENGTH];
} USBD_MIDI20_ClassData;

uint8_t USBD_MIDI20_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI20_Callbacks *pcallbacks);
uint8_t USBD_MIDI20_SendMessage(USBD_HandleTypeDef *pdev, uint8_t *pmessage, uint32_t length);

extern USBD_ClassTypeDef USBD_MIDI20;

#endif
