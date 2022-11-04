/**
  ******************************************************************************
  * @file    usbd_audio_if.h
  * @author  SRA
  * 
  * 
  * @brief   Header for usbd_audio_if.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *                             
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_AUDIO_IF_H
#define __USBD_AUDIO_IF_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_in_out.h"
#include "usbd_core.h"
#include "usbd_desc.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Send_Audio_to_USB(int16_t * audioData, uint16_t PCMSamples);
//void Audio_OUT_IncTick(void);
uint8_t Audio_OUT_GetState(void);
uint8_t Audio_OUT_SetState(uint8_t state);


#ifdef __cplusplus
}
#endif

#endif /* __USBD_AUDIO_IF_H */


