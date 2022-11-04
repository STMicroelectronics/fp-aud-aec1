/**
  ******************************************************************************
  * @file    audio_application.h 
  * @author  SRA
  * 
  * 
  * @brief   Header for audio_application.c module.
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
#ifndef __AUDIO_APPLICATION_H
#define __AUDIO_APPLICATION_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"    
#include "cca02m2_audio.h"
#include "cca01m1_audio.h"
#include "stdlib.h"
#include "arm_math.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
  
#define AUDIO_CHANNELS 				        USBD_AUDIO_IN_CH
#define AUDIO_SAMPLING_FREQUENCY 		        USBD_AUDIO_IN_FREQ
#define DEFAULT_VOLUME 0x5                    /* Default Volume */

   
/* Exported functions ------------------------------------------------------- */
void Init_Acquisition_Peripherals(uint32_t AudioFreq, uint32_t ChnlNbrIn, uint32_t ChnlNbrOut);
void Start_Acquisition(void);
void Error_Handler(void);
void AudioProcess(void);

uint32_t Init_Biquads_Filter(void);
uint32_t Init_AudioOut_Device(void);
uint32_t Start_AudioOut_Device(void);
uint32_t Stop_AudioOut_Device(void);
uint32_t Switch_Demo(void);

void AEC_Init(void);
void AEC_DeInit(void);

uint8_t Init_Libraries(void);

void Sensory_Init(void);
void Sensory_DeInit(void);
void Sensory_Run(void);
uint32_t Resume_AudioOut_Device(void);

void SW_IRQ_Tasks_Init(void);
void SW_Task1_Callback(void);
void SW_Task2_Callback(void);
void SW_Task1_Start(void);
void SW_Task2_Start(void);
void SW_Task3_Callback(void);
void SW_Task3_Start(void);
void SW_Task4_Callback(void);
void SW_Task4_Start(void);

void ledManager(void);


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_APPLICATION_H */


