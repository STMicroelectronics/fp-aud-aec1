/**
  ******************************************************************************
  * @file    stm32f7xx_it.h 
  * @author  SRA
  * 
  * 
  * @brief   This file contains the headers of the interrupt handlers.
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
#ifndef __STM32xx_IT_H
#define __STM32xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#include "stm32f7xx_nucleo_144.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_audio_in_out.h"
#include "usbd_audio_if.h"    
#include "audio_application.h"
#include "cca02m2_audio.h"
#include "cca01m1_audio.h"
   
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
   
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void USB_IRQHandler(void);

void AUDIO_IN_I2S_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void EXTI4_15_IRQHandler(void);
void AUDIO_OUT1_I2S_IRQHandler(void);
void AUDIO_OUT2_I2S_IRQHandler(void);
void EXTI3_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32xx_IT_H */


