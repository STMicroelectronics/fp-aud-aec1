/**
  ******************************************************************************
  * @file    usbd_audio_in_out.h
  * @author  SRA
  * @brief   header file for the usbd_audio_in_out.c file.
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

#ifndef __USBD_AUDIO_IN_OUT_H_
#define __USBD_AUDIO_IN_OUT_H_

#include "usbd_ioreq.h"
#include "usbd_conf.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
* @{
*/

/** @defgroup USBD_AUDIO_IN
* @{
*/ 

/** @defgroup USBD_AUDIO_IN_Exported_Defines
* @{
*/ 

/* Audio Data out endpoint */
#define AUDIO_OUT_EP                                  0x01U
/* Audio Data in endpoint */
#define AUDIO_IN_EP                                   0x81U 
  
/* Audio Data in endpoint */
#define AUDIO_OUT_SYNCH_EP                            0x82U

#define AUDIO_OUT_PACKET                              (uint32_t)((USBD_AUDIO_OUT_FREQ * 2U * 2U) /1000U)
#define AUDIO_MAX_PACKET_SIZE_OUT                     (uint32_t)(AUDIO_OUT_PACKET + 8U)

#define AUDIO_IN_PACKET                               (uint32_t)((USBD_AUDIO_IN_FREQ * USBD_AUDIO_IN_CH * 2U) /1000U)
#define AUDIO_MAX_PACKET_SIZE_IN                      (uint32_t)(AUDIO_IN_PACKET + 8U)


/* Audio Data in endpoint */
#define AUDIO_OUT_SYNCH_EP_DATA_SIZE                  3U
#define AUDIO_TERMINAL_USB_STREAMING                  0x0101U
#define AUDIO_OUTPUT_TERMINAL_SPEAKER                 0x0301U
#define AUDIO_INPUT_TERMINAL_MIC                      0x0201U

#define USB_AUDIO_CONFIG_DESC_SIZ                     (201 + USBD_AUDIO_IN_CH)
#define AUDIO_INTERFACE_DESC_SIZE                     9U
#define USB_AUDIO_DESC_SIZ                            0x09U
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09U
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07U
#define AUDIO_DESCRIPTOR_TYPE                         0x21U
#define USB_DEVICE_CLASS_AUDIO                        0x01U
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01U
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02U
#define AUDIO_PROTOCOL_UNDEFINED                      0x00U
#define AUDIO_STREAMING_GENERAL                       0x01U
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02U
/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24U
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25U
/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01U
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02U
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03U
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06U
#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0CU
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09U
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07U
#define AUDIO_CONTROL_MUTE                            0x01U
#define AUDIO_CONTROL_VOLUME                          0x02U
#define AUDIO_FORMAT_TYPE_I                           0x01U
#define AUDIO_FORMAT_TYPE_III                         0x03U
#define AUDIO_ENDPOINT_GENERAL                        0x01U
#define AUDIO_REQ_GET_CUR                             0x81U
#define AUDIO_REQ_GET_MIN                             0x82U
#define AUDIO_REQ_GET_MAX                             0x83U
#define AUDIO_REQ_GET_RES                             0x84U
#define AUDIO_REQ_SET_CUR                             0x01U
#define AUDIO_IN_STREAMING_CTRL                       0x02U
#define AUDIO_OUT_STREAMING_CTRL                      0x05U
#define USB_INTERFACE_DESCRIPTOR_TYPE                 0x04U

#define IN_VOL_MIN                                    0x0000U 
#define IN_VOL_RES                                    0x0001U
#define OUT_VOL_MIN                                   0x0000U 
#define OUT_VOL_RES                                   0x0001U
#define VOL_MAX                                       0x0065U

/* Buffering state definitions */
typedef enum
{
  STATE_USB_WAITING_FOR_INIT = 0,
  STATE_USB_IDLE = 1,
  STATE_USB_REQUESTS_STARTED = 2,  
  STATE_USB_BUFFER_WRITE_STARTED = 3,   
  STATE_USB_BUFFER_READ_STARTED = 4,
  STATE_USB_PAUSED = 5
}
AUDIO_StatesTypeDef;

/* Number of sub-packets in the audio in transfer buffer.*/
#define AUDIO_IN_PACKET_NUM     4U
/* Number of sub-packets in the audio out transfer buffer.*/
#define AUDIO_OUT_PACKET_NUM    6U

#define AUDIO_IN_TIMEOUT_VALUE  500

/* Audio Commands enmueration */
typedef enum
{
  AUDIO_CMD_START = 1,
  AUDIO_CMD_PLAY,
  AUDIO_CMD_STOP,
}AUDIO_CMD_TypeDef;

/**
* @}
*/ 


/** @defgroup USBD_AUDIO_IN_Exported_TypesDefinitions
* @{
*/
typedef struct
{
  
  uint8_t data[USB_MAX_EP0_SIZE];  
  uint8_t cmd;   
  uint8_t len;  
  uint8_t unit;    
}
USBD_AUDIO_ControlTypeDef; 

typedef enum
{
  HID_IDLE = 0,
  HID_BUSY,
}
HID_StateTypeDef; 

typedef struct
{
  uint8_t  *   IN_buffer;
  uint8_t  *   OUT_buffer;
  
  uint32_t     alt_setting_if0;
  uint32_t     alt_setting_if1;  
  uint32_t     alt_setting_if2;  
  
  uint8_t      IN_channels;
  uint16_t     IN_frequency;
  __IO int16_t IN_timeout;
  uint16_t     IN_buffer_length;    
  uint16_t     IN_dataAmount;
  uint16_t     IN_paketDimension;   
  uint8_t      IN_state;  
  uint16_t     IN_rd_ptr;  
  uint16_t     IN_wr_ptr;  
  uint8_t      IN_upper_treshold;
  uint8_t      IN_lower_treshold;
  int32_t      IN_VOL_CUR;
  
  uint8_t      OUT_channels;
  uint16_t     OUT_frequency;
  __IO int16_t OUT_timeout;
  uint16_t     OUT_buffer_length;    
  uint16_t     OUT_dataAmount;
  uint16_t     OUT_paketDimension;   
  uint8_t      OUT_state;  
  uint16_t     OUT_rd_ptr;  
  uint16_t     OUT_wr_ptr;  
  uint8_t      OUT_upper_treshold;
  uint8_t      OUT_lower_treshold;  
  int32_t      OUT_VOL_CUR;
     
  USBD_AUDIO_ControlTypeDef control;   
  
   /* HID */
  uint32_t         Protocol;   
  uint32_t         IdleState;  
  uint32_t         AltSetting;
  HID_StateTypeDef state;
}
USBD_AUDIO_HandleTypeDef; 


typedef struct
{
  int8_t  (*IN_Init)         	  (uint32_t  AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
  int8_t  (*IN_DeInit)       	  (uint32_t options);
  int8_t  (*IN_Record)     	  (void);
  int8_t  (*IN_VolumeCtl)    	  (int16_t Volume);
  int8_t  (*IN_MuteCtl)      	  (uint8_t cmd);
  int8_t  (*IN_Stop)   		  (void);
  int8_t  (*IN_Pause)   	  (void);
  int8_t  (*IN_Resume)   	  (void);
  int8_t  (*IN_CommandMgr)        (uint8_t cmd);
                                  
  int8_t  (*OUT_Init)         	  (uint32_t Volume, uint32_t  AudioFreq);
  int8_t  (*OUT_DeInit)       	  (uint32_t options);
  int8_t  (*OUT_Play)     	  (uint8_t* pData, uint32_t size);
  int8_t  (*OUT_VolumeCtl)    	  (int16_t Volume);
  int8_t  (*OUT_MuteCtl)      	  (uint8_t cmd);
  int8_t  (*OUT_Stop)   	  (void);
  int8_t  (*OUT_Pause)   	  (void);
  int8_t  (*OUT_Resume)   	  (void);
  uint32_t (*OUT_GetStreamingPos) (uint32_t totalBufferSize);
  int8_t  (*OUT_CommandMgr)       (uint8_t cmd);
  int32_t  (*OUT_PeriodicTC)      (uint8_t * data, uint32_t count);  
}
USBD_AUDIO_ItfTypeDef;
/**
* @}
*/ 

/** @defgroup USBD_AUDIO_IN_Exported_Macros
* @{
*/ 

/**
* @}
*/ 

/** @defgroup USBD_AUDIO_IN_Exported_Variables
* @{
*/ 

extern USBD_ClassTypeDef  USBD_AUDIO;
/**
* @}
*/ 

/** @defgroup USBD_AUDIO_IN_Exported_Functions
* @{
*/ 
uint8_t  USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_AUDIO_ItfTypeDef *fops);
void USBD_AUDIO_Init_Microphone_Descriptor(USBD_HandleTypeDef *pdev, uint32_t samplingFrequency, uint8_t Channels);
uint8_t  USBD_AUDIO_Data_Transfer(USBD_HandleTypeDef *pdev, int16_t * audioData, uint16_t dataAmount);

uint8_t USBD_AUDIO_Get_OUT_State(USBD_HandleTypeDef *pdev);
uint8_t USBD_AUDIO_Set_OUT_State(USBD_HandleTypeDef *pdev, uint8_t state);

uint8_t USBD_UAC_CDC_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);

/**
* @}
*/ 


/**
* @}
*/ 

/**
* @}
*/ 
#endif  // __USBD_AUDIO_IN_OUT_H_

