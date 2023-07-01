#include "usbd_midi10_device.h"
#include "usbd_ctlreq.h"

#define USBD_MIDI10_EP_OUT_ADDR 0x01
#define USBD_MIDI10_EP_IN_ADDR 0x82
#define USBD_MIDI10_EP_MAX_PACKET_SIZE 0x20

static uint8_t  USBD_MIDI10_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI10_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI10_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t  *USBD_MIDI10_GetFSCfgDesc(uint16_t *length);
static uint8_t  *USBD_MIDI10_GetHSCfgDesc(uint16_t *length);
static uint8_t  *USBD_MIDI10_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t  *USBD_MIDI10_GetDeviceQualifierDesc(uint16_t *length);
static uint8_t  USBD_MIDI10_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MIDI10_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MIDI10_EP0_RxReady(USBD_HandleTypeDef  *pdev);


USBD_ClassTypeDef  USBD_MIDI10 =
{
	USBD_MIDI10_Init,
	USBD_MIDI10_DeInit,
	USBD_MIDI10_Setup,
	NULL, /*EP0_TxSent*/
	USBD_MIDI10_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
	USBD_MIDI10_DataIn, /*DataIn*/
	USBD_MIDI10_DataOut,
	NULL, /*SOF */
	NULL,
	NULL,
	USBD_MIDI10_GetHSCfgDesc,
	USBD_MIDI10_GetFSCfgDesc,
	USBD_MIDI10_GetOtherSpeedCfgDesc,
	USBD_MIDI10_GetDeviceQualifierDesc,
};

/* USB CUSTOM_HID device FS Configuration Descriptor */
#define USBD_MIDI10_CONFIG_DESC_SIZE 97
__ALIGN_BEGIN static uint8_t USBD_MIDI10_FS_CONFIG_DESC[USBD_MIDI10_CONFIG_DESC_SIZE] __ALIGN_END =
{
    0x09,                               /* bLength */
    USB_DESC_TYPE_CONFIGURATION,        /* bDescriptorType */
    USBD_MIDI10_CONFIG_DESC_SIZE,       /* wTotalLength */
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
    0x01,
    54,                                 /* wTotalLength */
    0x00,

    /* MIDI IN JACK */
    0x06,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x02,                               /* bDescriptorSubtype */
    0x01,                               /* bJackType */
    0x01,                               /* bJackID */
    0x00,                               /* iJack */

    /* MIDI IN JACK */
    0x06,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x02,                               /* bDescriptorSubtype */
    0x02,                               /* bJackType */
    0x02,                               /* bJackID */
    0x00,                               /* iJack */

    /* MIDI OUT JACK */
    0x09,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x03,                               /* bDescriptorSubtype */
    0x01,                               /* bJackType */
    0x03,                               /* bJackID */
    0x01,                               /* bNrInputPins */
    0x02,                               /* baSourceID1 */
    0x01,                               /* BaSourcePin1 */
    0x00,                               /* iJack */

    /* MIDI OUT JACK */
    0x09,                               /* bLength */
    0x24,                               /* bDescriptorType */
    0x03,                               /* bDescriptorSubtype */
    0x02,                               /* bJackType */
    0x04,                               /* bJackID */
    0x01,                               /* bNrInputPins */
    0x01,                               /* baSourceID1 */
    0x01,                               /* BaSourcePin1 */
    0x00,                               /* iJack */

    /* OUT ENDPOINT */
    0x07,                               /* bLength */
    USB_DESC_TYPE_ENDPOINT,             /* bDescriptorType */
    USBD_MIDI10_EP_OUT_ADDR,            /* bEndpointAddress */
    0x02,                               /* bmAttributes */
    USBD_MIDI10_EP_MAX_PACKET_SIZE,     /* wMaxPacketSize */
    0x00,
    0x00,                               /* bInterval */

    /* OUT ENDPOINT MS */
    0x05,                               /* bLength */
    0x25,                               /* bDescriptorType */
    0x01,                               /* bDescriptorSubtype */
    0x01,                               /* bNumEmbMIDIJack */
    0x01,                               /* baAssocJackID */

    /* IN ENDPOINT */
    0x07,                               /* bLength */
    USB_DESC_TYPE_ENDPOINT,             /* bDescriptorType */
    USBD_MIDI10_EP_IN_ADDR,             /* bEndpointAddress */
    0x02,                               /* bmAttributes */
    USBD_MIDI10_EP_MAX_PACKET_SIZE,     /* wMaxPacketSize */
    0x00,
    0x00,                               /* bInterval */

    /* IN ENDPOINT MS */
    0x05,                               /* bLength */
    0x25,                               /* bDescriptorType */
    0x01,                               /* bDescriptorSubtype */
    0x01,                               /* bNumEmbMIDIJack */
    0x03,                               /* baAssocJackID */
};

/**
  * @brief  USBD_CUSTOM_HID_Init
  *         Initialize the CUSTOM_HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI10_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    uint8_t ret = 0U;
    USBD_MIDI10_ClassData *pclass_data;

    /* Open EP IN */
    USBD_LL_OpenEP(pdev, USBD_MIDI10_EP_IN_ADDR, USBD_EP_TYPE_BULK, USBD_MIDI10_EP_MAX_PACKET_SIZE);
    pdev->ep_in[USBD_MIDI10_EP_IN_ADDR & 0xFU].is_used = 1U;

    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, USBD_MIDI10_EP_OUT_ADDR, USBD_EP_TYPE_BULK, USBD_MIDI10_EP_MAX_PACKET_SIZE);
    pdev->ep_out[USBD_MIDI10_EP_OUT_ADDR & 0xFU].is_used = 1U;

    pdev->pClassData = USBD_malloc(sizeof(USBD_MIDI10_ClassData));

    if (pdev->pClassData == NULL)
    {
        ret = 1U;
    }
    else
    {
        pclass_data = (USBD_MIDI10_ClassData*)pdev->pClassData;

        /* Prepare Out endpoint to receive 1st packet */
        USBD_LL_PrepareReceive(pdev, USBD_MIDI10_EP_OUT_ADDR, pclass_data->rx_buf, RX_BUFFER_SIZE);
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
static uint8_t USBD_MIDI10_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Close CUSTOM_HID EP IN */
    USBD_LL_CloseEP(pdev, USBD_MIDI10_EP_IN_ADDR);
    pdev->ep_in[USBD_MIDI10_EP_IN_ADDR & 0xFU].is_used = 0U;

    /* Close CUSTOM_HID EP OUT */
    USBD_LL_CloseEP(pdev, USBD_MIDI10_EP_OUT_ADDR);
    pdev->ep_out[USBD_MIDI10_EP_OUT_ADDR & 0xFU].is_used = 0U;

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
static uint8_t USBD_MIDI10_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint16_t status_info = 0U;
    uint8_t ret = USBD_OK;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            break;

        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest)
            {
                case USB_REQ_GET_STATUS:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED)
                    {
                        USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
                    }
                    else
                    {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_GET_DESCRIPTOR:
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;

                case USB_REQ_GET_INTERFACE:
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;

                case USB_REQ_SET_INTERFACE:
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;

                default:
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;
            }
            break;

        default:
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
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
static uint8_t *USBD_MIDI10_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI10_FS_CONFIG_DESC);
  return USBD_MIDI10_FS_CONFIG_DESC;
}

/**
  * @brief  USBD_CUSTOM_HID_GetHSCfgDesc
  *         return HS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_MIDI10_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI10_FS_CONFIG_DESC);
  return USBD_MIDI10_FS_CONFIG_DESC;
}

/**
  * @brief  USBD_CUSTOM_HID_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_MIDI10_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI10_FS_CONFIG_DESC);
  return USBD_MIDI10_FS_CONFIG_DESC;
}

/**
  * @brief  USBD_CUSTOM_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_MIDI10_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_MIDI10_Callbacks *pcallbacks = (USBD_MIDI10_Callbacks*)pdev->pUserData;
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
static uint8_t  USBD_MIDI10_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_MIDI10_ClassData *pclass_data = (USBD_MIDI10_ClassData*)pdev->pClassData;

//    pdev->
//
//    ((USBD_MIDI10_Callbacks *)pdev->pUserData)->message_rx(pclass_data->rx_buf->Report_buf[0], hhid->Report_buf[1]);

    USBD_LL_PrepareReceive(pdev, USBD_MIDI10_EP_OUT_ADDR, pclass_data->rx_buf, RX_BUFFER_SIZE);

    return USBD_OK;
}

/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_MIDI10_EP0_RxReady(USBD_HandleTypeDef *pdev)
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
static uint8_t  *USBD_MIDI10_GetDeviceQualifierDesc(uint16_t *length)
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
uint8_t USBD_MIDI10_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI10_Callbacks *pcallbacks)
{
  uint8_t  ret = USBD_FAIL;

  if (pcallbacks != NULL)
  {
    pdev->pUserData = pcallbacks;
    ret = USBD_OK;
  }

  return ret;
}

static uint32_t _message;
uint8_t USBD_MIDI10_SendMessage(USBD_HandleTypeDef *pdev, uint32_t message)
{
    _message = message;

    return USBD_LL_Transmit(pdev, USBD_MIDI10_EP_IN_ADDR, (uint8_t*)&_message, 4);
}

