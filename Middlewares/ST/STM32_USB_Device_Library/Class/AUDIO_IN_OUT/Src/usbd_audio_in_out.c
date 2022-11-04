/**
  ******************************************************************************
  * @file    usbd_audio_in_out.c
  * @author  SRA
  * @brief   This file provides the Audio Input/Output core functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#include "usbd_audio_in_out.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
* @{
*/

/** @defgroup USBD_AUDIO_IN 
* 		
* @{
*/

/** @defgroup USBD_AUDIO_IN_Private_TypesDefinitions
* @{
*/
/**
* @}
*/

/** @defgroup USBD_AUDIO_IN_Private_Defines
* @{
*/

/**
* @}
*/

/** @defgroup USBD_AUDIO_IN_Private_Macros
* @{
*/
/**
* @}
*/

/** @defgroup USBD_AUDIO_IN_Private_FunctionPrototypes
* @{
*/
static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void handleTypeStandard(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void handleTypeClass(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length);
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length);
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetMaximum(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetMinimum(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetResolution(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
/**
* @}
*/

/** @defgroup USBD_AUDIO_Private_Variables
* @{
*/
/* This dummy buffer with 0 values will be sent when there is no availble data */
static uint8_t IsocInBuffDummy[AUDIO_MAX_PACKET_SIZE_IN];
static uint8_t SinglePacketBuffer[AUDIO_MAX_PACKET_SIZE_OUT];

static uint8_t testSynch[]={0,0,0x0c,0};
static uint16_t diff = 0;

/* Macros to convert the samples number into 10.10 format (extended into 10.14 format) */
#define FEEDBACK_EXTENDED_PRECISION_10_14

#define FEEDBACK_FF_FS(buf,a,b) buf[0]= FEEDBACK_FF_FS_B2(a,b);\
                                buf[1]= FEEDBACK_FF_FS_B1(a,b);\
                                buf[2]= FEEDBACK_FF_FS_B0(a,b)
                                  
#define FEEDBACK_FF_FS_B0(a,b)  (uint8_t)(((a) & 0x3FCU) >> 2U)
                                  
#define FEEDBACK_FF_FS_B1(a,b)  (uint8_t)(((((a) & 0x03U) << 6U) & 0xC0U) | (((b) >> 4U) & 0x3FU))
                                  
#ifndef FEEDBACK_EXTENDED_PRECISION_10_14
 #define FEEDBACK_FF_FS_B2(a,b) (uint8_t)(((b) << 4U) & 0xF0U)
#else
#define FEEDBACK_FF_FS_B2(a,b)  (uint8_t)((b) & 0xFFU)
#endif /* FEEDBACK_EXTENDED_PRECISION_10_14 */
                                  

USBD_ClassTypeDef USBD_AUDIO = 
{ 
  USBD_AUDIO_Init, 
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup, 
  USBD_AUDIO_EP0_TxReady, 
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn, 
  USBD_AUDIO_DataOut, 
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete, 
  USBD_AUDIO_IsoOutIncomplete,
  USBD_AUDIO_GetCfgDesc, 
  USBD_AUDIO_GetCfgDesc, 
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc
};


/* USB AUDIO device Configuration Descriptor */
/* NOTE: This descriptor has to be filled using the Descriptor Initialization function */
/* USB AUDIO device Configuration Descriptor */
#define AUDIO_SAMPLE_FREQ(frq)      (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))
#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * 2 * 2)/1000) & 0xFF), (uint8_t)((((frq * 2 * 2)/1000) >> 8) & 0xFF)

__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[] __ALIGN_END
= {
  /*CONFIGURATION DESCRIPTOR*/
  0x09,                                         /* bLength */
  0x02,                                         /* bDescriptorType */
  ((USB_AUDIO_CONFIG_DESC_SIZ ) & 0xff),        /* wTotalLength */
  ((USB_AUDIO_CONFIG_DESC_SIZ ) >> 8),
  0x03,                                         /* bNumInterfaces */
  0x01,                                         /* bConfigurationValue */
  0x00,                                         /* iConfiguration */
  0x80,                                         /* bmAttributes  BUS Powered*/
  0x32,                                         /* bMaxPower = 100 mA*/  
  
  /*AudioControl Interface Descriptor*/
  /* Standard AC Interface Descriptor */
  9,                                            /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,                /* bDescriptorType */
  0x00,                                         /* bInterfaceNumber */
  0x00,                                         /* bAlternateSetting */
  0x00,                                         /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,                       /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOCONTROL,                  /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,                     /* bInterfaceProtocol */
  0x00,                                         /* iInterface */  
  
  /* Class-specific Interface Descriptor - HEADER - */
  0x0A,                                         /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_CONTROL_HEADER,                         /* bDescriptorSubtype */
  0x00,                                         /* 1.00 */
  0x01,
  70 + USBD_AUDIO_IN_CH,                        /* wTotalLength */
  0x00,
  0x02,                                         /* bInCollection */
  0x01,                                         /* baInterfaceNr */
  0x02,                                         /* baInterfaceNr */  
  
  /* USB Microphone Input Terminal Descriptor */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,               /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_CONTROL_INPUT_TERMINAL,                 /* bDescriptorSubtype */
  0x01,                                         /* bTerminalID */
  0x01,                                         /* wTerminalType AUDIO_TERMINAL_USB_MICROPHONE   0x0201 */
  0x02,
  0x00,                                         /* bAssocTerminal not associated with anything*/
  USBD_AUDIO_IN_CH,                             /* bNrChannels*/
#if (USBD_AUDIO_IN_CH == 1) || (USBD_AUDIO_IN_CH == 4) || (USBD_AUDIO_IN_CH == 6) || (USBD_AUDIO_IN_CH == 8)
  0x00,                                         /* wChannelConfig 0x0003  Stereo */
#else
  0x03,                                         /* wChannelConfig 0x0003  Stereo */
#endif  
  0x00,
  0x00,                                         /* iChannelNames */
  0x00,                                         /* iTerminal */
  
  /* USB Microphone Audio Feature Unit Descriptor */
  0x07 + USBD_AUDIO_IN_CH +1,                   /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_CONTROL_FEATURE_UNIT,                   /* bDescriptorSubtype */
  0x02,                                         /* bUnitID */
  0x01,                                         /* bSourceID */ 
  0x01,                                         /* bControlSize */
  AUDIO_CONTROL_VOLUME,
  0x00,
#if (USBD_AUDIO_IN_CH == 2) ||(USBD_AUDIO_IN_CH == 4) || (USBD_AUDIO_IN_CH == 6) || (USBD_AUDIO_IN_CH == 8)
  0x00,
#endif
#if (USBD_AUDIO_IN_CH == 4) || (USBD_AUDIO_IN_CH == 6) || (USBD_AUDIO_IN_CH == 8)
  0X02,
  0X02,
#endif
#if (USBD_AUDIO_IN_CH == 6) || (USBD_AUDIO_IN_CH == 8)
  0X02,
  0X02,
#endif
#if USBD_AUDIO_IN_CH == 8
  0X02,
  0X02,
#endif
  0x00,                                         /* iFeature */
  
  /*USB Microphone Output Terminal Descriptor */
  0x09,                                         /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_CONTROL_OUTPUT_TERMINAL,                /* bDescriptorSubtype */
  0x03,                                         /* bTerminalID */
  0x01,                                         /* wTerminalType AUDIO_TERMINAL_USB_STREAMING 0x0101*/
  0x01,
  0x00,                                         /*Unused*/
  0x02,                                         /*bSourceID: from Audio Feature Unit Descriptor*/
  0x00,                                         /*Unused*/
  
  /* USB Speaker Input Terminal Descriptor */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,               /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_CONTROL_INPUT_TERMINAL,                 /* bDescriptorSubtype */
  0x04,                                         /* bTerminalID */
  0x01,                                         /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
  0x01, 
  0x00,                                         /* bAssocTerminal */
  0x02,                                         /* bNrChannels */
  0x00,                                         /* wChannelConfig 0x0300  Stereo */
  0x03, 
  0x00,                                         /* iChannelNames */
  0x00,                                         /* iTerminal */
  /* 12 byte*/
  
  /* USB Speaker Audio Feature Unit Descriptor */
  0x0A,                                         /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_CONTROL_FEATURE_UNIT,                   /* bDescriptorSubtype */
  0x05,                                         /* bUnitID */
  0x04,                                         /* bSourceID */
  0x01,                                         /* bControlSize */
  AUDIO_CONTROL_VOLUME, 
  0x00,                                         /* bmaControls(0) */
  0x00,                                         /* bmaControls(1) */
  0x00,                                         /* iTerminal */
  /* 09 byte*/
  
  /*USB Speaker Output Terminal Descriptor */
  0x09,      /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_CONTROL_OUTPUT_TERMINAL,                /* bDescriptorSubtype */
  0x06,                                         /* bTerminalID */
  0x01,                                         /* wTerminalType  0x0301= speaker*/
  0x03, 
  0x00,                                         /* bAssocTerminal */
  0x05,                                         /* bSourceID */
  0x00,                                         /* iTerminal */
  /*******end of audio control interface*/  
  
  /* USB Microphone Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
  /* Interface 1, Alternate Setting 0      */
  9, /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,                /* bDescriptorType */
  0x01,                                         /* bInterfaceNumber */
  0x00,                                         /* bAlternateSetting */
  0x00,                                         /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,                       /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,                /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,                     /* bInterfaceProtocol */
  0x00,
  
  /* USB Microphone Standard AS Interface Descriptor - Audio Streaming Operational */
  /* Interface 1, Alternate Setting 1                                           */
  9,                                            /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,                /* bDescriptorType */
  0x01,                                         /* bInterfaceNumber */
  0x01,                                         /* bAlternateSetting */
  0x01,                                         /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,                       /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,                /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,                     /* bInterfaceProtocol */
  0x00,                                         /* iInterface */
  
  /* USB Microphone Audio Streaming Interface Descriptor GENERAL*/
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,          /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_STREAMING_GENERAL,                      /* bDescriptorSubtype */
  0x03,                                         /* bTerminalLink */
  0x01,                                         /* bDelay */
  0x01,                                         /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
  0x00,
  
  /* USB Microphone Audio Type I Format Interface Descriptor */
  0x0B,                                         /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_STREAMING_FORMAT_TYPE,                  /* bDescriptorSubtype */
  AUDIO_FORMAT_TYPE_I,                          /* bFormatType */
  USBD_AUDIO_IN_CH,                             /* bNrChannels */
  0x02,                                         /* bSubFrameSize */
  16,                                           /* bBitResolution */
  0x01,                                         /* bSamFreqType */
  USBD_AUDIO_IN_FREQ & 0xff,                    /* tSamFreq */
  (USBD_AUDIO_IN_FREQ >> 8) & 0xff, 
  USBD_AUDIO_IN_FREQ >> 16,  
  
  /*ENDPOINT DESCROPTORS */
  /* Endpoint 1 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,            /* bLength */
  0x05,                                         /* bDescriptorType */
  AUDIO_IN_EP,                                  /* bEndpointAddress 1 in endpoint*/
  0x05,                                         /* bmAttributes */
  LOBYTE(AUDIO_MAX_PACKET_SIZE_IN),       /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  HIBYTE(AUDIO_MAX_PACKET_SIZE_IN),   
  0x01,                                         /* bInterval */
  0x00,                                         /* bRefresh */
  0x00,                                         /* bSynchAddress */
  /* Endpoint - Audio Streaming Descriptor*/
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,           /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,               /* bDescriptorType */
  AUDIO_ENDPOINT_GENERAL,                       /* bDescriptor */
  0x00,                                         /* bmAttributes */
  0x00,                                         /* bLockDelayUnits */
  0x00,                                         /* wLockDelay */
  0x00,  
  
  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
  /* Interface 1, Alternate Setting 0                                             */
  AUDIO_INTERFACE_DESC_SIZE,                    /* bLength */
  USB_DESC_TYPE_INTERFACE,                      /* bDescriptorType */
  0x02,                                         /* bInterfaceNumber */
  0x00,                                         /* bAlternateSetting */
  0x00,                                         /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,                       /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,                /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,                     /* bInterfaceProtocol */
  0x00,                                         /* iInterface */
  
  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
  /* Interface 1, Alternate Setting 1                                           */
  AUDIO_INTERFACE_DESC_SIZE,                    /* bLength */
  USB_DESC_TYPE_INTERFACE,                      /* bDescriptorType */
  0x02,                                         /* bInterfaceNumber */
  0x01,                                         /* bAlternateSetting */
  0x02,                                         /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,                       /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,                /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,                     /* bInterfaceProtocol */
  0x00,                                         /* iInterface */
  
  /* USB Speaker Audio Streaming Interface Descriptor */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,          /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_STREAMING_GENERAL,                      /* bDescriptorSubtype */
  0x04,                                         /* bTerminalLink */
  0x01,                                         /* bDelay */
  0x01,                                         /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
  0x00,
  
  /* USB Speaker Audio Type III Format Interface Descriptor */
  0x0B,                                         /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,              /* bDescriptorType */
  AUDIO_STREAMING_FORMAT_TYPE,                  /* bDescriptorSubtype */
  AUDIO_FORMAT_TYPE_I,                        /* bFormatType */
  0x02,                                         /* bNrChannels */
  0x02,                                         /* bSubFrameSize :  2 Bytes per frame (16bits) */
  16,                                           /* bBitResolution (16-bits per sample) */
  0x01,                                         /* bSamFreqType only one frequency supported */
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_OUT_FREQ),       /* Audio sampling frequency coded on 3 bytes */
  
  /* Endpoint 1 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_ENDPOINT,                       /* bDescriptorType */
  AUDIO_OUT_EP,                                 /* bEndpointAddress 1 out endpoint*/
  USBD_EP_TYPE_ISOC + 4,                            /* bmAttributes */
  LOBYTE(AUDIO_MAX_PACKET_SIZE_OUT),       /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  HIBYTE(AUDIO_MAX_PACKET_SIZE_OUT),        /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
  0x01,                                         /* bInterval */
  0x00,                                         /* bRefresh */
  AUDIO_OUT_SYNCH_EP,                            /* bSynchAddress */
  
  /* Endpoint - Audio Streaming Descriptor*/
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,           /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,               /* bDescriptorType */
  AUDIO_ENDPOINT_GENERAL,                       /* bDescriptor */
  0x00,                                         /* bmAttributes */
  0x00,                                         /* bLockDelayUnits */
  0x00,                                         /* wLockDelay */
  0x00,
  
  /* Endpoint 1 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_ENDPOINT,                       /* bDescriptorType */
  AUDIO_OUT_SYNCH_EP,                           /* bEndpointAddress 1 out endpoint*/
  USBD_EP_TYPE_ISOC,                            /* bmAttributes */
  AUDIO_OUT_SYNCH_EP_DATA_SIZE,                 /* wMaxPacketSize in Bytes */
  0x00,
  0x01,                                         /* bInterval */
  5,  /*3 = 8 ms*/                              /* bRefresh */
  0x00,                                         /* bSynchAddress */
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END
= {
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER, 
  0x00, 
  0x02, 
  0x00, 
  0x00, 
  0x00, 
  0x40,
  0x01, 
  0x00, 
};
/**
* @}
*/

/** @defgroup USBD_AUDIO_IN_Private_Functions
* @{
*/

/**
* @brief  USBD_AUDIO_Init
*         Initialize the AUDIO interface
* @param  pdev: device instance
* @param  cfgidx: Configuration index
* @retval status
*/
static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx) 
{ 
  UNUSED(cfgidx);
  
  uint8_t ret = (uint8_t)USBD_OK;
  
  pdev->pClassData = USBD_malloc(sizeof(USBD_AUDIO_HandleTypeDef));
  if (pdev->pClassData == NULL) 
  {    
    ret = (uint8_t)USBD_FAIL;
  }
  else
  {  
    (void)USBD_LL_OpenEP(pdev, AUDIO_IN_EP, USBD_EP_TYPE_ISOC, (uint16_t)AUDIO_MAX_PACKET_SIZE_IN);  
    (void)USBD_LL_OpenEP(pdev, AUDIO_OUT_SYNCH_EP, USBD_EP_TYPE_ISOC, (uint16_t)AUDIO_OUT_SYNCH_EP_DATA_SIZE);  
    (void)USBD_LL_OpenEP(pdev, AUDIO_OUT_EP, USBD_EP_TYPE_ISOC, (uint16_t)AUDIO_MAX_PACKET_SIZE_OUT);  
    
    USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *) pdev->pClassData;
    
    haudio->IN_paketDimension = (USBD_AUDIO_IN_FREQ / 1000U * USBD_AUDIO_IN_CH * 2U);
    haudio->IN_frequency = USBD_AUDIO_IN_FREQ;
    haudio->IN_buffer_length =  haudio->IN_paketDimension * AUDIO_IN_PACKET_NUM;
    haudio->IN_channels = USBD_AUDIO_IN_CH;
    haudio->IN_upper_treshold = 5;
    haudio->IN_lower_treshold = 2;
    haudio->IN_state = (uint8_t)STATE_USB_WAITING_FOR_INIT;
    haudio->IN_wr_ptr = 3U * haudio->IN_paketDimension;
    haudio->IN_rd_ptr = 0;
    haudio->IN_dataAmount = 0;
    
    /*Audio In buffer is allocated when data is passed to the driver by the application for the first time*/
    haudio->IN_buffer = 0;  
    
    uint16_t wr_rd_offset = (AUDIO_IN_PACKET_NUM / 2U) * haudio->IN_dataAmount / haudio->IN_paketDimension;
    haudio->IN_wr_ptr = wr_rd_offset * haudio->IN_paketDimension;
    haudio->IN_rd_ptr = 0;
    haudio->IN_timeout = 0;
    
    haudio->OUT_paketDimension = (USBD_AUDIO_OUT_FREQ / 1000U * USBD_AUDIO_OUT_CH * 2U);
    haudio->OUT_frequency = USBD_AUDIO_OUT_FREQ;
    haudio->OUT_buffer_length = haudio->OUT_paketDimension * AUDIO_OUT_PACKET_NUM;    
    haudio->OUT_channels = USBD_AUDIO_OUT_CH;
    haudio->OUT_upper_treshold = 5;
    haudio->OUT_lower_treshold = 2;
    haudio->OUT_state = (uint8_t)STATE_USB_WAITING_FOR_INIT;
    haudio->OUT_wr_ptr = 0;
    haudio->OUT_rd_ptr = 0;
    haudio->OUT_dataAmount = 0;  /*Not Used*/
    
    haudio->OUT_buffer = USBD_malloc(haudio->OUT_buffer_length); 
    if (haudio->OUT_buffer == NULL) 
    {
      ret = (uint8_t)USBD_FAIL;
    }
    else
    {    
      /*Call interface functions to init application specific audio hardware*/
      ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->IN_Init(haudio->IN_frequency, 0, haudio->IN_channels);
      ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->OUT_Init(0x10, haudio->OUT_frequency);
      ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->OUT_VolumeCtl(haudio->OUT_VOL_CUR);
      
      FEEDBACK_FF_FS(testSynch, haudio->OUT_frequency/1000U, 0U);  
      
      (void)USBD_LL_FlushEP(pdev, AUDIO_IN_EP);  
      (void)USBD_LL_FlushEP(pdev, AUDIO_OUT_EP);  
      (void)USBD_LL_FlushEP(pdev, AUDIO_OUT_SYNCH_EP);  
      
      (void)USBD_LL_Transmit(pdev, AUDIO_IN_EP, IsocInBuffDummy, AUDIO_MAX_PACKET_SIZE_IN);  
      (void)USBD_LL_Transmit(pdev, AUDIO_OUT_SYNCH_EP, (uint8_t *)testSynch, AUDIO_OUT_SYNCH_EP_DATA_SIZE);  
      (void)USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, (uint8_t *)SinglePacketBuffer, AUDIO_MAX_PACKET_SIZE_OUT);  
      
      haudio->IN_state = (uint8_t)STATE_USB_IDLE;
      haudio->OUT_state = (uint8_t)STATE_USB_IDLE;
      
      haudio->alt_setting_if0 = 0;
      haudio->alt_setting_if1 = 0;
      haudio->alt_setting_if2 = 0;
    }
  }
  return ret;
}

/**
* @brief  USBD_AUDIO_Init
*         DeInitialize the AUDIO layer
* @param  pdev: device instance
* @param  cfgidx: Configuration index
* @retval status
*/
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx) 
{
  UNUSED(cfgidx);
  
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *) pdev->pClassData;
  
  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, AUDIO_IN_EP);
  /* Close EP OUT */
  (void)USBD_LL_CloseEP(pdev, AUDIO_OUT_EP);  
  (void)USBD_LL_CloseEP(pdev, AUDIO_OUT_SYNCH_EP);
  
  /* DeInit  physical Interface components */
  if (pdev->pClassData != NULL) 
  {
    ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->IN_DeInit(0);
    haudio->IN_state = (uint8_t)STATE_USB_WAITING_FOR_INIT;
    
    ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->OUT_DeInit(0);
    haudio->OUT_state = (uint8_t)STATE_USB_WAITING_FOR_INIT;    
  }
  return (uint8_t)USBD_OK;
}

/**
* @brief  USBD_AUDIO_Setup
*         Handle the AUDIO specific requests
* @param  pdev: instance
* @param  req: usb requests
* @retval status
*/
static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) 
{  
  switch (req->bmRequest & USB_REQ_TYPE_MASK) 
  {
  case USB_REQ_TYPE_CLASS:    /* AUDIO Class Requests */
    handleTypeClass(pdev, req);
    break;
    
  case USB_REQ_TYPE_STANDARD:   /* Standard Requests */   
    handleTypeStandard(pdev, req);
    break;
    
  default:
    break;
  }
  return (uint8_t)USBD_OK;
}

static void handleTypeStandard(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{   
  USBD_AUDIO_HandleTypeDef *haudio;
  uint16_t len;
  uint8_t *pbuf;
  haudio = pdev->pClassData;
  
  switch (req->bRequest) 
  {
  case USB_REQ_GET_DESCRIPTOR:
    if ((req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE) 
    {        
      pbuf = USBD_AUDIO_CfgDesc + 18;
      len = MIN(USB_AUDIO_DESC_SIZ, req->wLength);        
      (void)USBD_CtlSendData(pdev, pbuf, len);
    }
    break;
    
  case USB_REQ_GET_INTERFACE:
    if (req->wIndex == 0x000U)
    {
      (void)USBD_CtlSendData(pdev, (uint8_t *) &haudio->alt_setting_if0, 1);        
    }
    else if (req->wIndex == 0x001U)
    {
      (void)USBD_CtlSendData(pdev, (uint8_t *) &haudio->alt_setting_if1, 1);        
    }
    else if (req->wIndex == 0x002U)
    {
      (void)USBD_CtlSendData(pdev, (uint8_t *) &haudio->alt_setting_if2, 1);
    }
    else
    {
      /**/
    }
    break;
    
  case USB_REQ_SET_INTERFACE:
    if ((uint8_t) (req->wValue) < USBD_MAX_NUM_INTERFACES) 
    {
      if (req->wIndex == 0x000U)
      {
        haudio->alt_setting_if0 = (uint8_t) (req->wValue);          
      }
      else if (req->wIndex == 0x001U)
      {
        haudio->alt_setting_if1 = (uint8_t) (req->wValue);
      }
      else if (req->wIndex == 0x002U)
      {
        haudio->alt_setting_if2 = (uint8_t) (req->wValue);
      }
      else
      {
        
      }
    } 
    else 
    {
      /* Call the error management function (command will be nacked */
      USBD_CtlError(pdev, req);
    }
    break;
    
  default:
    break;
  }  
}

static void handleTypeClass(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{  
  switch (req->bRequest) 
  {
  case AUDIO_REQ_GET_CUR:
    AUDIO_REQ_GetCurrent(pdev, req);
    break;
    
  case AUDIO_REQ_SET_CUR:
    AUDIO_REQ_SetCurrent(pdev, req);
    break;
    
  case AUDIO_REQ_GET_MIN:
    AUDIO_REQ_GetMinimum(pdev, req);
    break;
    
  case AUDIO_REQ_GET_MAX:
    AUDIO_REQ_GetMaximum(pdev, req);
    break;
    
  case AUDIO_REQ_GET_RES:
    AUDIO_REQ_GetResolution(pdev, req);
    break;
    
  default:
    USBD_CtlError(pdev, req);
    break;
  }
}

/**
* @brief  USBD_AUDIO_GetCfgDesc
*         return configuration descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length) 
{
  *length = (uint16_t)sizeof(USBD_AUDIO_CfgDesc);
  return USBD_AUDIO_CfgDesc;
}

/**
* @brief  USBD_AUDIO_DataIn
*         handle data IN Stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) 
{  
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = pdev->pClassData;
  uint16_t length_usb_pck;
  uint16_t app;
  uint16_t IsocInWr_app = haudio->IN_wr_ptr;
  uint16_t true_dim = haudio->IN_buffer_length;
  uint16_t packet_dim = haudio->IN_paketDimension;
  uint16_t channels = haudio->IN_channels;
  length_usb_pck = packet_dim;
  haudio->IN_timeout = 0;
  
  if (true_dim != 0U)
  {
    if (epnum == (AUDIO_IN_EP & 0x7FU)) 
    {
      if (haudio->IN_state == (uint8_t)STATE_USB_IDLE) 
      {
        haudio->IN_state = (uint8_t)STATE_USB_REQUESTS_STARTED;
        ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->IN_Record();
      }
      if (haudio->IN_state == (uint8_t)STATE_USB_BUFFER_WRITE_STARTED) 
      {
        haudio->IN_rd_ptr = haudio->IN_rd_ptr % (true_dim);
        
        if (IsocInWr_app < haudio->IN_rd_ptr) 
        {
          app = ((true_dim) - haudio->IN_rd_ptr) + IsocInWr_app;
        } 
        else 
        {
          app = IsocInWr_app - haudio->IN_rd_ptr;
        }
        
        if (app >= (packet_dim * haudio->IN_upper_treshold)) 
        {
          length_usb_pck += channels * 2U;
        } 
        else if (app <= (packet_dim * haudio->IN_lower_treshold)) 
        {
          length_usb_pck -= channels * 2U;
        }
        else
        {
          /**/
        }
        
        (void)USBD_LL_Transmit(pdev, AUDIO_IN_EP, (uint8_t*) (&haudio->IN_buffer[haudio->IN_rd_ptr]), length_usb_pck);
        
        haudio->IN_rd_ptr += length_usb_pck;
        
        if (app < (haudio->IN_buffer_length/10U)) 
        {
          ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->IN_Stop();
          haudio->IN_state = (uint8_t)STATE_USB_IDLE;
          haudio->IN_timeout = 0;
          (void)memset(haudio->IN_buffer, 0, ((uint32_t)haudio->IN_buffer_length + (uint32_t)haudio->IN_dataAmount));
        }
      } 
      else 
      {
        (void)USBD_LL_Transmit(pdev, AUDIO_IN_EP, IsocInBuffDummy, length_usb_pck);
      }
    }
    else if (epnum == (AUDIO_OUT_SYNCH_EP & 0x7FU)) 
    {
      (void)USBD_LL_Transmit(pdev, AUDIO_OUT_SYNCH_EP, testSynch, AUDIO_OUT_SYNCH_EP_DATA_SIZE);
    } 
    else
    {
      /**/
    }
  }
  return (uint8_t)USBD_OK;
}

/**
* @brief  USBD_AUDIO_EP0_RxReady
*         handle EP0 Rx Ready event
* @param  pdev: device instance
* @retval status
*/

static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev) 
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = pdev->pClassData;
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR) 
  {
    if (haudio->control.unit == AUDIO_IN_STREAMING_CTRL) 
    {      
      haudio->IN_VOL_CUR = *((int16_t *)haudio->control.data);
      /*Call interface function*/
      ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->IN_VolumeCtl(haudio->IN_VOL_CUR);
      
      haudio->control.cmd = 0;
      haudio->control.len = 0;
      haudio->control.unit = 0;
      haudio->control.data[0] = 0;
      haudio->control.data[0] = 0;
    }
    else if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL) 
    {     
      haudio->OUT_VOL_CUR = *((int16_t *)haudio->control.data);
      /*Call interface function*/
      ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->OUT_VolumeCtl(haudio->OUT_VOL_CUR);
      
      haudio->control.cmd = 0;
      haudio->control.len = 0;
      haudio->control.unit = 0;
      haudio->control.data[0] = 0;
      haudio->control.data[0] = 0;
    }
    else
    {
      /**/
    }
  }
  return (uint8_t)USBD_OK;
}
/**
* @brief  USBD_AUDIO_EP0_TxReady
*         handle EP0 TRx Ready event
* @param  pdev: device instance
* @retval status
*/
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev) 
{
  UNUSED(pdev);
  /* Only OUT control data are processed */
  return (uint8_t)USBD_OK;
}

/**
* @brief  USBD_AUDIO_SOF
*         handle SOF event
* @param  pdev: device instance
* @retval status
*/
static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev) 
{
  /*This is a very simple control loop for the feedback ep: it works, but it could be enanched to reduce
  PC-side resampling intervention*/
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = pdev->pClassData;
  uint16_t packet_dim = haudio->OUT_paketDimension;
  uint16_t integer, fractional;
  if( (haudio->OUT_state != (uint8_t)STATE_USB_WAITING_FOR_INIT) || (haudio->OUT_state != (uint8_t)STATE_USB_IDLE) )
  { 
    haudio->OUT_rd_ptr =  ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->OUT_GetStreamingPos(haudio->OUT_buffer_length);
    
    if (haudio->OUT_wr_ptr >= haudio->OUT_rd_ptr)
    {
      diff = (haudio->OUT_wr_ptr - haudio->OUT_rd_ptr);
    }
    else
    {
      diff = haudio->OUT_buffer_length - haudio->OUT_rd_ptr  + haudio->OUT_wr_ptr;
    } 
    
    if (diff <= (packet_dim * haudio->OUT_lower_treshold)) 
    {
      integer = (haudio->OUT_frequency/1000U) ;
      fractional = 51;      
      /*This results in sending feedback for Fs/1000.(fractional/2^10) samples
      For example, @ 48 KHZ with fractional = 51, you ask for ~48.05 samples*/
    }
    else if (diff >= (packet_dim * haudio->OUT_upper_treshold)) 
    {
      integer = (haudio->OUT_frequency/1000U) - 1U;
      fractional = 972;
      /*This results in sending feedback for (Fs/1000 -1).(fractional/2^10) samples
      For example, @ 48 KHZ with fractional = 972, you ask for ~47.95 samples*/
    }
    else
    {
      integer = (haudio->OUT_frequency/1000U);
      fractional = 0;      
    }
    
    FEEDBACK_FF_FS(testSynch,integer,fractional); 
  }
  
  else
  {
    FEEDBACK_FF_FS(testSynch, haudio->OUT_frequency/1000U, 0U);    
  }
  
  if(haudio->OUT_state == (uint8_t)STATE_USB_BUFFER_READ_STARTED)
  {   
    haudio->OUT_timeout++;
  }
  
  if ( (haudio->OUT_timeout >= 2) && (haudio->OUT_state == (uint8_t)STATE_USB_BUFFER_READ_STARTED) )
  {
    ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->OUT_Stop();
    haudio->OUT_state = (uint8_t)STATE_USB_IDLE;    
  }  
  
  return (uint8_t)USBD_OK;
}
/**
* @brief  USBD_AUDIO_IsoINIncomplete
*         handle data ISO IN Incomplete event
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum) 
{
  UNUSED(epnum);
  
  uint32_t dsts, depctl;
  
  PCD_HandleTypeDef *localHpcd = pdev->pData;
  USB_OTG_GlobalTypeDef *USBx =  localHpcd ->Instance;  
  uint32_t USBx_BASE = (uint32_t)USBx;
  
  dsts = USBx_DEVICE->DSTS;
  depctl = USBx_INEP(AUDIO_OUT_SYNCH_EP & 0x7FU)->DIEPCTL;
  
  /* Check if this is the first packet */
  if(((dsts & USB_OTG_DSTS_FNSOF) & 1U) == (depctl & USB_OTG_DIEPCTL_SD0PID_SEVNFRM))
  {
    /* EP disable */
    USBx_INEP(AUDIO_OUT_SYNCH_EP & 0x7FU)->DIEPCTL |= (USB_OTG_DIEPCTL_EPDIS | USB_OTG_DIEPCTL_SNAK);
    /*flush endpoint*/
    (void)USB_FlushTxFifo(USBx, AUDIO_OUT_SYNCH_EP & 0x7FU);
    /*Restart transfer  (EVEN/ODD frame scheduling will be done here when preparing packet) */
    (void)USBD_LL_Transmit(pdev, AUDIO_OUT_SYNCH_EP, (uint8_t*)testSynch, AUDIO_OUT_SYNCH_EP_DATA_SIZE);
  }  
  return (uint8_t)USBD_OK;
}
/**
* @brief  USBD_AUDIO_IsoOutIncomplete
*         handle data ISO OUT Incomplete event
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum) 
{
  UNUSED(pdev);
  UNUSED(epnum);
  return (uint8_t)USBD_OK;
}
/**
* @brief  USBD_AUDIO_DataOut
*         handle data OUT Stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
/**
* @brief  USBD_AUDIO_DataOut
*         handle data OUT Stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
static uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  uint16_t packet_length;
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  uint16_t i;
  
  if (epnum == AUDIO_OUT_EP)
  {    
    if(haudio->OUT_state == (uint8_t)STATE_USB_IDLE)
    {
      haudio->OUT_state = (uint8_t)STATE_USB_REQUESTS_STARTED;
    }    
    
    /*Set Timeout = 0*/
    haudio->OUT_timeout = 0;        
    /*Get received length */
    packet_length = (uint16_t)USBD_LL_GetRxDataSize(pdev, epnum);    
    ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->OUT_PeriodicTC(SinglePacketBuffer, packet_length);
    
    for (i = 0; i < packet_length; i ++)
    {          
      ((uint8_t *)haudio->OUT_buffer)[haudio->OUT_wr_ptr] = SinglePacketBuffer[i];
      haudio->OUT_wr_ptr += 1U;  
      haudio->OUT_wr_ptr = haudio->OUT_wr_ptr % haudio->OUT_buffer_length; 
    }       
    
    /*Start Audio output by calling the relevant interface function when half of the buffer is full*/
    if( (haudio->OUT_wr_ptr >= (haudio->OUT_buffer_length/2U) ) && (haudio->OUT_state != (uint8_t)STATE_USB_BUFFER_READ_STARTED) )
    {      
      ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->OUT_Play((uint8_t *)haudio->OUT_buffer,haudio->OUT_buffer_length);
      haudio->OUT_state = (uint8_t)STATE_USB_BUFFER_READ_STARTED;     
    }    
    
    /* Prepare Out endpoint to receive next audio packet */
    (void)USBD_LL_PrepareReceive(pdev, AUDIO_OUT_EP, ((uint8_t *)SinglePacketBuffer), AUDIO_MAX_PACKET_SIZE_OUT);    
  }  
  return (uint8_t)USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length) 
{
  *length = (uint16_t)sizeof(USBD_AUDIO_DeviceQualifierDesc);
  return USBD_AUDIO_DeviceQualifierDesc;
}

/**
* @brief  AUDIO_REQ_GetMaximum
*         Handles the VOL_MAX Audio control request.
* @param  pdev: instance
* @param  req: setup class request
* @retval status
*/
static void AUDIO_REQ_GetMaximum(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) 
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = pdev->pClassData;
  
  (haudio->control.data)[0] = (uint8_t)((uint16_t) VOL_MAX & 0xFFU);
  (haudio->control.data)[1] = (uint8_t)(((uint16_t) VOL_MAX & 0xFF00U) >> 8U);
  
  (void)USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
}

/**
* @brief  AUDIO_REQ_GetMinimum
*         Handles the IN_VOL_MIN Audio control request.
* @param  pdev: instance
* @param  req: setup class request
* @retval status
*/
static void AUDIO_REQ_GetMinimum(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) 
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = pdev->pClassData;
  
  if((req->wIndex >> 8) == AUDIO_IN_STREAMING_CTRL)
  {
    (haudio->control.data)[0] = (uint8_t)((uint16_t) IN_VOL_MIN & 0xFFU);
    (haudio->control.data)[1] = (uint8_t)(((uint16_t) IN_VOL_MIN & 0xFF00U) >> 8U);
  }
  else if((req->wIndex >> 8) == AUDIO_OUT_STREAMING_CTRL)
  {
    (haudio->control.data)[0] = (uint8_t)((uint16_t) OUT_VOL_MIN & 0xFFU);
    (haudio->control.data)[1] = (uint8_t)(((uint16_t)OUT_VOL_MIN & 0xFF00U) >> 8U);    
  }
  else
  {
    /**/
  }
  
  /* Send the current mute IN_state */
  (void)USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
}

/**
* @brief  AUDIO_Req_GetResolution
*         Handles the IN_VOL_RES Audio control request.
* @param  pdev: instance
* @param  req: setup class request
* @retval status
*/
static void AUDIO_REQ_GetResolution(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) 
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = pdev->pClassData;
  if((req->wIndex >> 8) == AUDIO_IN_STREAMING_CTRL)
  {
    (haudio->control.data)[0] = (uint8_t)((uint16_t) IN_VOL_RES & 0xFFU);
    (haudio->control.data)[1] = (uint8_t)(((uint16_t) IN_VOL_RES & 0xFF00U) >> 8U);
  }
  else if((req->wIndex >> 8) == AUDIO_OUT_STREAMING_CTRL)
  {
    (haudio->control.data)[0] = (uint8_t)((uint16_t) OUT_VOL_RES & 0xFFU);
    (haudio->control.data)[1] = (uint8_t)(((uint16_t) OUT_VOL_RES & 0xFF00U) >> 8U);    
  }
  else
  {
    /**/
  }
  (void)USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
}

/**
* @brief  AUDIO_Req_GetCurrent
*         Handles the GET_CUR Audio control request.
* @param  pdev: instance
* @param  req: setup class request
* @retval status
*/
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) 
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = pdev->pClassData;
  
  if((req->wIndex >> 8) == AUDIO_IN_STREAMING_CTRL)
  {
    (haudio->control.data)[0] = (uint8_t)((uint16_t) haudio->IN_VOL_CUR & 0xFFU);
    (haudio->control.data)[1] = (uint8_t)(((uint16_t) haudio->IN_VOL_CUR & 0xFF00U) >> 8U);
  }
  else if((req->wIndex >> 8) == AUDIO_OUT_STREAMING_CTRL)
  {
    (haudio->control.data)[0] = (uint8_t)((uint16_t) haudio->OUT_VOL_CUR & 0xFFU);
    (haudio->control.data)[1] = (uint8_t)(((uint16_t) haudio->OUT_VOL_CUR & 0xFF00U) >> 8U);    
  }
  else
  {
    /**/
  }
  
  (void)USBD_CtlSendData(pdev, haudio->control.data, req->wLength);
}

/**
* @brief  AUDIO_Req_SetCurrent
*         Handles the SET_CUR Audio control request.
* @param  pdev: instance
* @param  req: setup class request
* @retval status
*/
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) 
{
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = pdev->pClassData;
  if (req->wLength != 0U) 
  {
    /* Prepare the reception of the buffer over EP0 */
    (void)USBD_CtlPrepareRx(pdev, (uint8_t *) haudio->control.data, req->wLength);
    
    haudio->control.cmd = AUDIO_REQ_SET_CUR; /* Set the request value */
    haudio->control.len = (uint8_t)req->wLength; /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex); /* Set the request target unit */
  }
}

/**
* @}
*/

/** @defgroup USBD_AUDIO_IN_Exported_Functions
* @{
*/

/**
* @brief  USBD_AUDIO_Data_Transfer
*         Fills the USB internal buffer with audio data from user
* @param pdev: device instance
* @param audioData: audio data to be sent via USB
* @param IN_dataAmount: number of PCM samples to be copyed
* @note Depending on the calling frequency, a coherent amount of samples must be passed to
*       the function. E.g.: assuming a Sampling frequency of 16 KHz and 1 channel,
*       you can pass 16 PCM samples if the function is called each millisecond,
*       32 samples if called every 2 milliseconds and so on.
* @retval status
*/
uint8_t USBD_AUDIO_Data_Transfer(USBD_HandleTypeDef *pdev, int16_t * audioData, uint16_t PCMSamples) 
{
  uint8_t ret = (uint8_t)USBD_OK;
  USBD_AUDIO_HandleTypeDef *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef *) pdev->pClassData;
  
  if ( (haudio->IN_state == (uint8_t)STATE_USB_WAITING_FOR_INIT) || (pdev->pClassData == NULL) )
  {
    ret = (uint8_t)USBD_BUSY;
  }
  else 
  {
    uint16_t IN_dataAmount = PCMSamples * 2U; /*Bytes*/
    uint16_t true_dim = haudio->IN_buffer_length;
    uint16_t current_data_Amount = haudio->IN_dataAmount;
    uint16_t packet_dim = haudio->IN_paketDimension;
    
    if (packet_dim != 0U)
    {    
      if ( (haudio->IN_state == (uint8_t)STATE_USB_REQUESTS_STARTED) || (current_data_Amount != IN_dataAmount) ) 
      {        
        /*USB parameters definition, based on the amount of data passed*/
        haudio->IN_dataAmount = IN_dataAmount;
        uint8_t wr_rd_offset = (AUDIO_IN_PACKET_NUM / 2U) * (uint8_t)(IN_dataAmount / packet_dim);
        haudio->IN_wr_ptr = wr_rd_offset * packet_dim;
        haudio->IN_rd_ptr = 0;
        haudio->IN_upper_treshold = wr_rd_offset + 1U;
        haudio->IN_lower_treshold = wr_rd_offset - 1U;
        haudio->IN_buffer_length = (packet_dim * (IN_dataAmount / packet_dim) * AUDIO_IN_PACKET_NUM);
        
        /*Memory allocation for data buffer, depending (also) on data amount passed to the transfer function*/
        if (haudio->IN_buffer != NULL) 
        {
          USBD_free(haudio->IN_buffer);
        }
        
        haudio->IN_buffer = USBD_malloc((uint32_t)haudio->IN_buffer_length + (uint32_t)haudio->IN_dataAmount);    
        if (haudio->IN_buffer == NULL) 
        {
          ret = (uint8_t)USBD_FAIL;
        }
        else
        {      
          (void)memset(haudio->IN_buffer, 0, (uint32_t)haudio->IN_buffer_length + (uint32_t)haudio->IN_dataAmount);
          haudio->IN_state = (uint8_t)STATE_USB_BUFFER_WRITE_STARTED;      
        }
      } 
      else if (haudio->IN_state == (uint8_t)STATE_USB_BUFFER_WRITE_STARTED) 
      {
        if (haudio->IN_timeout == AUDIO_IN_TIMEOUT_VALUE)
        {
          haudio->IN_state = (uint8_t)STATE_USB_IDLE;
          ((USBD_AUDIO_ItfTypeDef *) pdev->pUserData[pdev->classId])->IN_Stop();
          haudio->IN_timeout = 0;
        }
        haudio->IN_timeout++;
        
        (void)memcpy((uint8_t *) &haudio->IN_buffer[haudio->IN_wr_ptr],
                     (uint8_t *) (audioData), IN_dataAmount);
         haudio->IN_wr_ptr += IN_dataAmount;
         if (true_dim != 0U)
         {
           haudio->IN_wr_ptr = haudio->IN_wr_ptr % (true_dim);
         }
         if ((haudio->IN_wr_ptr - IN_dataAmount) == 0U) 
         {
           (void)memcpy((uint8_t *) (((uint8_t *) haudio->IN_buffer) + true_dim),
                        (uint8_t *) haudio->IN_buffer, IN_dataAmount);
         }
      }
      else
      {
        /**/
      }
    }
  }
  return ret;
}


/**
* @brief  USBD_AUDIO_RegisterInterface
* @param  fops: Audio interface callback
* @retval status
*/
uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_AUDIO_ItfTypeDef *fops) 
{
  if (fops != NULL) 
  {
    pdev->pUserData[0] = fops;
  }
  return 0;
}

uint8_t USBD_AUDIO_Get_OUT_State(USBD_HandleTypeDef *pdev)
{  
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *) pdev->pClassData;
  return haudio->OUT_state;  
}

uint8_t USBD_AUDIO_Set_OUT_State(USBD_HandleTypeDef *pdev, uint8_t state)
{
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *) pdev->pClassData;
  haudio->OUT_state = state;
  if(haudio->OUT_state == (uint8_t)STATE_USB_IDLE)
  {
    haudio->OUT_rd_ptr = 0;
    haudio->OUT_wr_ptr = 0;
  }
  return 0;  
}

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

