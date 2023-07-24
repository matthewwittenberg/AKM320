#include "usbd_midi20_device.h"
#include "usbd_ctlreq.h"
#include "midi_spec.h"
#include <stdbool.h>

#define USBD_MIDI20_EP_OUT_ADDR 0x02
#define USBD_MIDI20_EP_IN_ADDR 0x81
#define USBD_MIDI20_EP_MAX_PACKET_SIZE 0x40

static uint8_t  USBD_MIDI20_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI20_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI20_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t  *USBD_MIDI20_GetFSCfgDesc(uint16_t *length);
static uint8_t  *USBD_MIDI20_GetHSCfgDesc(uint16_t *length);
static uint8_t  *USBD_MIDI20_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t  *USBD_MIDI20_GetDeviceQualifierDesc(uint16_t *length);
static uint8_t  USBD_MIDI20_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MIDI20_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MIDI20_EP0_RxReady(USBD_HandleTypeDef  *pdev);


USBD_ClassTypeDef  USBD_MIDI20 =
{
    USBD_MIDI20_Init,
    USBD_MIDI20_DeInit,
    USBD_MIDI20_Setup,
    NULL, /*EP0_TxSent*/
    USBD_MIDI20_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
    USBD_MIDI20_DataIn, /*DataIn*/
    USBD_MIDI20_DataOut,
    NULL, /*SOF */
    NULL,
    NULL,
    USBD_MIDI20_GetHSCfgDesc,
    USBD_MIDI20_GetFSCfgDesc,
    USBD_MIDI20_GetOtherSpeedCfgDesc,
    USBD_MIDI20_GetDeviceQualifierDesc,
};

/* USB CUSTOM_HID device FS Configuration Descriptor */
#define USBD_MIDI20_CONFIG_DESC_SIZE 67
__ALIGN_BEGIN static uint8_t USBD_MIDI20_FS_CONFIG_DESC[USBD_MIDI20_CONFIG_DESC_SIZE] __ALIGN_END =
{
    0x09,                               /* bLength */
    USB_DESC_TYPE_CONFIGURATION,        /* bDescriptorType */
    USBD_MIDI20_CONFIG_DESC_SIZE,       /* wTotalLength */
    0x00,
    0x02,                               /* bNumInterfaces */
    0x01,                               /* bConfigurationValue */
    0x00,                               /* iConfiguration */
    0x80,                               /* bmAttributes */
    0xFA,                               /* bMaxPower */

    /* AUDIO CONTROL INTERFACE */
    0x09,                               /* bLength */
    USB_DESC_TYPE_INTERFACE,            /* bDescriptorType */
    0x00,                               /* bInterfaceNumber */
    0x00,                               /* bAlternateSetting */
    0x00,                               /* bNumEndpoints */
    0x01,                               /* bInterfaceClass */
    0x01,                               /* bInterfaceSubClass */
    0x00,                               /* bInterfaceProtocol */
    0x00,                               /* iInterface */

    /* AUDIO CONTROL INTERFACE HEADER */
    0x09,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x01,                               /* bDescriptorSubtype */
    0x00,                               /* bcdADC */
    0x01,
    0x09,                               /* wTotalLength */
    0x00,
    0x01,                               /* bInCollection */
    0x01,                               /* bInterfaceNr */

    /* MIDI STREAM INTERFACE */
    0x09,                               /* bLength */
    USB_DESC_TYPE_INTERFACE,            /* bDescriptorType */
    0x01,                               /* bInterfaceNumber */
    0x00,                               /* bAlternateSetting */
    0x02,                               /* bNumEndpoints */
    0x01,                               /* bInterfaceClass */
    0x03,                               /* bInterfaceSubClass */
    0x00,                               /* bInterfaceProtocol */
    0x00,                               /* iInterface */

    /* MIDI STREAM INTERFACE HEADER */
    0x07,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x01,                               /* bDescriptorSubtype */
    0x00,                               /* bcdMSC */
    0x02,
    24,                                 /* wTotalLength */
    0x00,

    /* OUT ENDPOINT */
    0x07,                               /* bLength */
    USB_DESC_TYPE_ENDPOINT,             /* bDescriptorType */
    USBD_MIDI20_EP_OUT_ADDR,            /* bEndpointAddress */
    0x02,                               /* bmAttributes */
    USBD_MIDI20_EP_MAX_PACKET_SIZE,     /* wMaxPacketSize */
    0x00,
    0x00,                               /* bInterval */

    /* OUT ENDPOINT MS */
    0x05,                               /* bLength */
    0x25,                               /* bDescriptorType */
    0x02,                               /* bDescriptorSubtype */
    0x01,                               /* bNumGrpTrmBlock */
    0x01,                               /* baAssoGrpTrmBlkID */

    /* IN ENDPOINT */
    0x07,                               /* bLength */
    USB_DESC_TYPE_ENDPOINT,             /* bDescriptorType */
    USBD_MIDI20_EP_IN_ADDR,             /* bEndpointAddress */
    0x02,                               /* bmAttributes */
    USBD_MIDI20_EP_MAX_PACKET_SIZE,     /* wMaxPacketSize */
    0x00,
    0x00,                               /* bInterval */

    /* IN ENDPOINT MS */
    0x05,                               /* bLength */
    0x25,                               /* bDescriptorType */
    0x02,                               /* bDescriptorSubtype */
    0x01,                               /* bNumGrpTrmBlock */
    0x01,                               /* baAssoGrpTrmBlkID */
};

/* MS 2.0 terminal block descriptor */
#define USBD_MIDI20_TERM_BLOCK_DESC_SIZE 18
__ALIGN_BEGIN static uint8_t USBD_MIDI20_TERM_BLOCK_DESC[USBD_MIDI20_TERM_BLOCK_DESC_SIZE] __ALIGN_END =
{
    0x05,                               /* bLength */
    0x26,                               /* bDescriptorType */
    0x01,                               /* bDescriptorSubtype */
    USBD_MIDI20_TERM_BLOCK_DESC_SIZE,   /* wTotalLength */
    0x00,

    /* TERMINAL BLOCK */
    0x0D,                               /* bLength */
    0x26,                               /* bDescriptorType */
    0x02,                               /* bDescriptorSubtype */
    0x01,                               /* bGrpTrmBlkID */
    0x00,                               /* bGrpTrmBlkType */
    0x00,                               /* nGroupTrm */
    0x01,                               /* nNumGroupTrm */
    0x00,                               /* iBlockItem */
    0x11,                               /* bMIDIProtocol */
    0x01,                               /* wMaxInputBandwidth */
    0x00,
    0x00,                               /* wMaxOutputBandwidth */
    0x00,
};

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         Initialize the CUSTOM_HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI20_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    uint8_t ret = 0U;
    USBD_MIDI20_ClassData *pclass_data;

    /* Open EP IN */
    USBD_LL_OpenEP(pdev, USBD_MIDI20_EP_IN_ADDR, USBD_EP_TYPE_BULK, USBD_MIDI20_EP_MAX_PACKET_SIZE);
    pdev->ep_in[USBD_MIDI20_EP_IN_ADDR & 0xFU].is_used = 1U;
    pdev->ep_in[USBD_MIDI20_EP_IN_ADDR & 0xFU].maxpacket = USBD_MIDI20_EP_MAX_PACKET_SIZE;

    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, USBD_MIDI20_EP_OUT_ADDR, USBD_EP_TYPE_BULK, USBD_MIDI20_EP_MAX_PACKET_SIZE);
    pdev->ep_out[USBD_MIDI20_EP_OUT_ADDR & 0xFU].is_used = 1U;
    pdev->ep_out[USBD_MIDI20_EP_OUT_ADDR & 0xFU].maxpacket = USBD_MIDI20_EP_MAX_PACKET_SIZE;

    pdev->pClassData = USBD_malloc(sizeof(USBD_MIDI20_ClassData));

    if (pdev->pClassData == NULL)
    {
        ret = 1U;
    }
    else
    {
        pclass_data = (USBD_MIDI20_ClassData*)pdev->pClassData;

        /* Prepare Out endpoint to receive 1st packet */
        memset(pclass_data->rx_buf, 0, sizeof(USBD_MIDI20_ClassData));
        USBD_LL_PrepareReceive(pdev, USBD_MIDI20_EP_OUT_ADDR, pclass_data->rx_buf, MIDI20_RX_LENGTH);
    }

    return ret;
}

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         DeInitialize the CUSTOM_HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI20_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Close CUSTOM_HID EP IN */
    USBD_LL_CloseEP(pdev, USBD_MIDI20_EP_IN_ADDR);
    pdev->ep_in[USBD_MIDI20_EP_IN_ADDR & 0xFU].is_used = 0U;

    /* Close CUSTOM_HID EP OUT */
    USBD_LL_CloseEP(pdev, USBD_MIDI20_EP_OUT_ADDR);
    pdev->ep_out[USBD_MIDI20_EP_OUT_ADDR & 0xFU].is_used = 0U;

    /* FRee allocated memory */
    if (pdev->pClassData != NULL)
    {
        USBD_free(pdev->pClassData);
        pdev->pClassData = NULL;
    }

    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_Setup
  *         Handle the CUSTOM_HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_MIDI20_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint16_t status_info = 0U;
    uint8_t ret = USBD_OK;
    bool handled = false;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            break;

        case USB_REQ_TYPE_VENDOR:
            break;

        case USB_REQ_TYPE_STANDARD:
            if(req->bRequest == USB_REQ_GET_DESCRIPTOR)
            {
                if(req->wValue == 0x2601)
                {
                    USBD_CtlSendData(pdev, USBD_MIDI20_TERM_BLOCK_DESC, USBD_MIDI20_TERM_BLOCK_DESC_SIZE);
                    handled = true;
                }
            }

            if(req->bRequest == USB_REQ_GET_STATUS)
            {
                USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
            }

            break;

        default:
            break;
    }

    if(handled == false)
    {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
    }

    return ret;
}



/**
  * @brief  USBD_CUSTOM_HID_GetFSCfgDesc
  *         return FS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MIDI20_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI20_FS_CONFIG_DESC);
  return USBD_MIDI20_FS_CONFIG_DESC;
}

/**
  * @brief  USBD_CUSTOM_HID_GetHSCfgDesc
  *         return HS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_MIDI20_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI20_FS_CONFIG_DESC);
  return USBD_MIDI20_FS_CONFIG_DESC;
}

/**
  * @brief  USBD_CUSTOM_HID_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_MIDI20_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI20_FS_CONFIG_DESC);
  return USBD_MIDI20_FS_CONFIG_DESC;
}

/**
  * @brief  USBD_CUSTOM_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_MIDI20_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_MIDI20_Callbacks *pcallbacks = (USBD_MIDI20_Callbacks*)pdev->pUserData;
    pcallbacks->message_tx_complete();

    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_MIDI20_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_MIDI20_Callbacks *pcallbacks = (USBD_MIDI20_Callbacks*)pdev->pUserData;
    uint8_t *pdata = ((USBD_MIDI20_ClassData*)pdev->pClassData)->rx_buf;
    uint32_t index = 0;
    uint8_t message_type;
    uint8_t message_length;

    message_type = pdata[index] & 0xF0;
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
        default:
            message_length = 0;
            break;
    }

    if(message_length > 0)
        pcallbacks->message_rx(pdata, message_length);

    USBD_LL_PrepareReceive(pdev, USBD_MIDI20_EP_OUT_ADDR, ((USBD_MIDI20_ClassData*)pdev->pClassData)->rx_buf, MIDI20_RX_LENGTH);

    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_MIDI20_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
//  USBD_CUSTOM_HID_HandleTypeDef     *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)pdev->pClassData;
//
//  if (hhid->IsReportAvailable == 1U)
//  {
//    ((USBD_CUSTOM_HID_ItfTypeDef *)pdev->pUserData)->OutEvent(hhid->Report_buf[0],
//                                                              hhid->Report_buf[1]);
//    hhid->IsReportAvailable = 0U;
//  }

  return USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_MIDI20_GetDeviceQualifierDesc(uint16_t *length)
{
    *length = 0;
    return NULL;
}

/**
* @brief  USBD_CUSTOM_HID_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CUSTOMHID Interface callback
  * @retval status
  */
uint8_t USBD_MIDI20_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI20_Callbacks *pcallbacks)
{
  uint8_t  ret = USBD_FAIL;

  if (pcallbacks != NULL)
  {
    pdev->pUserData = pcallbacks;
    ret = USBD_OK;
  }

  return ret;
}

static uint8_t _message[MIDI20_MESSAGE_LENGTH];
uint8_t USBD_MIDI20_SendMessage(USBD_HandleTypeDef *pdev, uint8_t *pmessage, uint32_t length)
{
    memcpy(_message, pmessage, length);

    return USBD_LL_Transmit(pdev, USBD_MIDI20_EP_IN_ADDR, _message, length);
}
