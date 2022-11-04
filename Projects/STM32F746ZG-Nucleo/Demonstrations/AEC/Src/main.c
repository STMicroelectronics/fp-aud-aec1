/**
  ******************************************************************************
  * @file    main.c 
  * @author  SRA
  * 
  * 
  * @brief   Main program body
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
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#include "stm32f7xx_it.h"
#include "stm32f7xx_nucleo_144.h"
#include "usbd_audio_if.h"    
#include "audio_application.h"


/** @addtogroup X_NUCLEO_CCA02M1_Applications
* @{
*/ 

/** @addtogroup Microphones_Acquisition
* @{
*/

/** @defgroup main 
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

USBD_HandleTypeDef hUSBDDevice;
extern USBD_AUDIO_ItfTypeDef  USBD_AUDIO_fops;


/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void MPU_Config(void);


/* Functions Definition ------------------------------------------------------*/

/**
* @brief  Main program
* @param  None
* @retval None
*/

int main(void)
{
  /* STM32F7xx HAL library initialization:
  - Configure the Flash prefetch, instruction and Data caches
  - Configure the Systick to generate an interrupt each 1 msec
  - Global MSP (MCU Support Package) initialization
  */       
  MPU_Config();    
  CPU_CACHE_Enable(); 
  HAL_Init();  
  /* Initialize Push Button in interrupt mode */
  /* Configure the system clock */ 
  SystemClock_Config();
    /* Start audio acquisition and streaming */  
  Init_Acquisition_Peripherals(AUDIO_IN_SAMPLING_FREQUENCY, AUDIO_IN_CHANNELS, 0);
  /* Init Device Library */
  USBD_Init(&hUSBDDevice, &AUDIO_Desc, 0);
  /* Add Supported Class */
  USBD_RegisterClass(&hUSBDDevice, &USBD_AUDIO);
  /* Add Interface callbacks for AUDIO Class */  
  USBD_AUDIO_RegisterInterface(&hUSBDDevice, &USBD_AUDIO_fops);  
    /*Start USB*/
  USBD_Start(&hUSBDDevice);


  /*Initialize software interrupts*/
  SW_IRQ_Tasks_Init();
  /*Initialize libraries*/
  Init_Libraries();        
  HAL_Delay(500);
  /*Start MIC acquisition*/
  Start_Acquisition();
  
  while (1)
  {    
  }
}


/**
* @brief  System Clock Configuration
* @param  None
* @retval None
*/
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 288;  
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1) {};
  }
  
  /* Activate the OverDrive to reach the 216 Mhz Frequency */
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    while(1) {};
  }
  
  /* Select PLLSAI output as USB clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 7; 
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct)  != HAL_OK)
  {
    while(1) {};
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    while(1) {};
  }

}

/**
* @brief  Configure the MPU attributes as Write Through for SRAM1/2.
* @note   The Base Address is 0x20010000 since this memory interface is the AXI.
*         The Region Size is 256KB, it is related to SRAM1 and SRAM2  memory size.
* @param  None
* @retval None
*/
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();
  
  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x20010000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
/* Private functions ---------------------------------------------------------*/
/**
* @brief  CPU L1-Cache enable.
* @param  None
* @retval None
*/
static void CPU_CACHE_Enable(void)
{
  /* Enable branch prediction */
  SCB->CCR |= (1 <<18); 
  __DSB();
  /* Enable I-Cache */
  SCB_EnableICache();
  
  /* Enable D-Cache */
  SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

