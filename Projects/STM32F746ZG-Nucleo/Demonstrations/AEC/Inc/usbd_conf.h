/**
  ******************************************************************************
  * @file    usbd_conf.h
  * @author  SRA
  * 
  * 
  * @brief   General low level driver configuration
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
#ifndef __USBD_CONF_H
#define __USBD_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "cca01m1_conf.h"
#include "cca02m2_conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Common Config */
#define USBD_MAX_NUM_INTERFACES               3U
#define USBD_MAX_NUM_CONFIGURATION            1U
#define USBD_MAX_STR_DESC_SIZ                 0x100U
#define USBD_SUPPORT_USER_STRING              0U 
#define USBD_SELF_POWERED                     0U
#define USBD_DEBUG_LEVEL                      0U

/* AUDIO Class Config */
#define USBD_AUDIO_OUT_FREQ                       AUDIO_OUT_SAMPLING_FREQUENCY
#define USBD_AUDIO_OUT_CH                         2U

#define USBD_AUDIO_IN_FREQ                        AUDIO_IN_SAMPLING_FREQUENCY
#define USBD_AUDIO_IN_CH                          USBD_AUDIO_OUT_CH


/* External variables --------------------------------------------------------*/
   
extern PCD_HandleTypeDef hpcd;


/* Exported macro ------------------------------------------------------------*/

/* Memory management macros */
#define USBD_memset               memset
#define USBD_memcpy               memcpy

#ifdef USE_STATIC_ALLOCATION
#define USBD_free                 USBD_static_free
#define USBD_malloc               USBD_static_malloc
#define MAX_STATIC_ALLOC_SIZE     1600/4
#else
#define USBD_free                 free
#define USBD_malloc               malloc
#define MAX_STATIC_ALLOC_SIZE     4
#endif

/* DEBUG macros */
#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...)   printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define  USBD_ErrLog(...)   printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)   
#endif

#if (USBD_DEBUG_LEVEL > 2)                         
#define  USBD_DbgLog(...)   printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)                         
#endif
                            
                            
/* Exported functions ------------------------------------------------------- */
                            

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF_H */


