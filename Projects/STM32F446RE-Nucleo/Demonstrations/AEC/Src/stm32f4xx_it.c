/**
  ******************************************************************************
  * @file    stm32xx_it.c 
  * @author  SRA
  * 
  * 
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"

/** @addtogroup X_NUCLEO_CCA02M1_Applications
* @{
*/

/** @addtogroup Microphones_Acquisition
* @{
*/  

/** @defgroup stm32xx_it 
* @{
*/
    
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define USB_IRQHandler OTG_FS_IRQHandler


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/******************************************************************************/
/*             Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
//  Audio_OUT_IncTick();
}

/**
  * @brief  This function handles USB Handler.
  * @param  None
  * @retval None
  */
void USB_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd);
}

/**
  * @brief  This function handles DMA Stream interrupt request.
  * @param  None
  * @retval None
  */
void AUDIO_IN_I2S_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hAudioInI2s.hdmarx);
}

/**
  * @brief  This function handles DMA Stream interrupt request for First Device.
  * @param  None
  * @retval None
  */
void AUDIO_OUT1_I2S_IRQHandler(void)
{

  HAL_DMA_IRQHandler(hAudioOut[0].hdmatx);
}

/**
  * @brief  This function handles DMA Stream interrupt request for Second Device.
  * @param  None
  * @retval None
  */
void DMA1_Stream7_IRQHandler(void)
{

  HAL_DMA_IRQHandler(hAudioOut[1].hdmatx);
}

/**
  * @brief  This function handles External lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
}

/**
  * @brief  This function handles External lines 4 to 15 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI4_15_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(KEY_BUTTON_PIN);
}


/**
  * @brief  Interrupt request.
  * @param  None
  * @retval None
  */
void EXTI3_IRQHandler(void)
{
   HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
   SW_Task3_Callback();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/


