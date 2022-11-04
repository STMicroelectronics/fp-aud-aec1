/**
  ******************************************************************************
  * @file    audio_application.c 
  * @author  SRA
  * 
  * 
  * @brief   Audio application
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
#include "audio_application.h"
#include "acoustic_ec.h"
#include "audio_fw_glo.h"

/** @addtogroup X_NUCLEO_CCA02M1_Applications
* @{
*/ 

/** @addtogroup Microphones_Acquisition
* @{
*/

/** @defgroup audio_application 
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/*Handler and Config structures for AEC*/
AcousticEC_Handler_t   EchoHandlerInstance;
AcousticEC_Config_t    EchoConfigInstance;

extern USBD_HandleTypeDef hUSBDDevice;

/*Temporary buffers*/
int16_t USBOUT[AUDIO_CHANNELS*AUDIO_SAMPLING_FREQUENCY/1000];
int16_t EchoOut[AUDIO_SAMPLING_FREQUENCY/1000];
int16_t pAudio_out_16K[AUDIO_IN_SAMPLING_FREQUENCY/1000];
int16_t pAudio_out_48K[AUDIO_OUT_SAMPLING_FREQUENCY/1000];

CCA02M2_AUDIO_Init_t MicParams;
CCA01M1_AUDIO_Init_t OutParams;

uint16_t PDM_Buffer[AUDIO_CHANNELS*AUDIO_SAMPLING_FREQUENCY/1000*128/8*2];
uint16_t PCM_Buffer[AUDIO_CHANNELS*AUDIO_SAMPLING_FREQUENCY/1000];


/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/**
* @brief  Half Transfer user callback, called by BSP functions.
* @param  None
* @retval None
*/
void CCA02M2_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  AudioProcess();
}

/**
* @brief  Transfer Complete user callback, called by BSP functions.
* @param  None
* @retval None
*/
void CCA02M2_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  AudioProcess();
}

/**
* @brief  User function that is called when 1 ms of PDM data is available.
* 		  In this application only PDM to PCM conversion and USB streaming
*                  is performed.
* 		  User can add his own code here to perform some DSP or audio analysis.
* @param  none
* @retval None
*/

void AudioProcess(void)
{
  uint16_t i;
  static uint8_t process_state = 0; 
  
  static uint16_t audio_out_position;
  static uint16_t audio_out_size;
  static int16_t * pAudio_out;
  
  CCA02M2_AUDIO_IN_PDMToPCM(CCA02M2_AUDIO_IN_INSTANCE,(uint16_t * )PDM_Buffer,PCM_Buffer);  
  
  /*This processing part depends on the USB and output streaming:*/  
  if(Audio_OUT_GetState() != STATE_USB_BUFFER_READ_STARTED)
  {
    /*Audio out is not playing: a single mic is streamed to USB CH1*/
    for(i = 0; i < AUDIO_IN_SAMPLING_FREQUENCY/1000; i++)
    {  
      USBOUT[USBD_AUDIO_IN_CH * i + 0] = PCM_Buffer[i]; 
      USBOUT[USBD_AUDIO_IN_CH * i + 1] = 0;
    }
    
    process_state = 0;
    audio_out_position = 0;
  }  
  else if (Audio_OUT_GetState() == STATE_USB_BUFFER_READ_STARTED && process_state == 0)
  {    
    /*First time after Audio output starts*/
    /*Get audio output relevant parameters to be used for AEC*/
    USBD_AUDIO_HandleTypeDef * haudio = (USBD_AUDIO_HandleTypeDef*) (hUSBDDevice.pClassData);
    /*starts feeding the AEC with audio at this position:*/
    audio_out_position = ((USBD_AUDIO_ItfTypeDef *)hUSBDDevice.pUserData[0])->OUT_GetStreamingPos(haudio->OUT_buffer_length) + (haudio->OUT_buffer_length / 2) % (haudio->OUT_buffer_length / 2);  /*half output buffer (in int16_t)*/

    if(audio_out_position%2 != 0)
    {
      audio_out_position++;
    }
    
    audio_out_size = haudio->OUT_buffer_length / 2; /*(in int16_t)*/
    pAudio_out = (int16_t *)haudio->OUT_buffer;        
    
    /*Running phase*/
    for(i = 0; i < USBD_AUDIO_OUT_FREQ/1000; i++)
    {  
      /* Only R ch is used for this demo*/
      pAudio_out_48K[i] = pAudio_out[audio_out_position+1]; 
      audio_out_position=(audio_out_position+2)%(audio_out_size); 
    }      
    
    /*Sum L and R channel*/
    for(i = 0; i < AUDIO_IN_SAMPLING_FREQUENCY/1000; i++)
    {  
      /* Take 1 sample every 3 from the audio_out channel (FS_out = 48KHz, FS_in = 16KHz) */
      pAudio_out_16K[i] = pAudio_out_48K[3*i];
    }
    
    /*A single mic is streamed to USB CH1*/
    for(i = 0; i < AUDIO_IN_SAMPLING_FREQUENCY/1000; i++)
    {  
      USBOUT[USBD_AUDIO_IN_CH * i + 0] = PCM_Buffer[i]; 
      USBOUT[USBD_AUDIO_IN_CH * i + 1] = pAudio_out_16K[i];
    }    
    
    process_state = 1;
  }
  else
  {
    /*Running phase*/
    for(i = 0; i < USBD_AUDIO_OUT_FREQ/1000; i++)
    {  
      /* Only R ch is used for this demo*/
      pAudio_out_48K[i] = pAudio_out[audio_out_position+1]; 
      audio_out_position=(audio_out_position+2)%(audio_out_size);  
    }      
    
    /*Sum L and R channel*/
    for(i = 0; i < AUDIO_IN_SAMPLING_FREQUENCY/1000; i++)
    {  
      /* Take 1 sample every 3 from the audio_out channel (FS_out = 48KHz, FS_in = 16KHz) */
      pAudio_out_16K[i] = pAudio_out_48K[3*i];
    }
    
    /*AEC call*/    
    if(AcousticEC_Data_Input(&PCM_Buffer[0], pAudio_out_16K, &EchoOut[0], (AcousticEC_Handler_t *)&EchoHandlerInstance))    
    {            
      SW_Task3_Start();       
    }  
        
    /*AEC output and a single mic are streamed to USB */
    for(i = 0; i < AUDIO_IN_SAMPLING_FREQUENCY/1000; i++)
    {   
      USBOUT[USBD_AUDIO_IN_CH * i + 0] = PCM_Buffer[i]; 
      USBOUT[USBD_AUDIO_IN_CH * i + 1] = EchoOut[i];    
    }
  } 
 
  /*Stream data to USB microphone*/
  Send_Audio_to_USB((int16_t *)USBOUT, AUDIO_SAMPLING_FREQUENCY/1000*USBD_AUDIO_IN_CH);  
}

/**
* @brief  User function that is called when 1 ms of PDM data is available.
* 		  In this application only PDM to PCM conversion and USB streaming
*                  is performed.
* 		  User can add his own code here to perform some DSP or audio analysis.
* @param  none
* @retval None
*/
void Init_Acquisition_Peripherals(uint32_t AudioFreq, uint32_t ChnlNbrIn, uint32_t ChnlNbrOut)
{  
    /* Configure Audio Input peripheral*/  
  MicParams.BitsPerSample = 16;
  MicParams.ChannelsNbr = ChnlNbrIn;
  MicParams.Device = AUDIO_IN_DIGITAL_MIC;
  MicParams.SampleRate = AudioFreq;
  MicParams.Volume = AUDIO_VOLUME_INPUT;
  
  CCA02M2_AUDIO_IN_Init(CCA02M2_AUDIO_IN_INSTANCE, &MicParams);  
}

/**
* @brief  User function that is called when 1 ms of PDM data is available.
* 		  In this application only PDM to PCM conversion and USB streaming
*                  is performed.
* 		  User can add his own code here to perform some DSP or audio analysis.
* @param  none
* @retval None
*/
void Start_Acquisition(void)
{  
  CCA02M2_AUDIO_IN_Record(CCA02M2_AUDIO_IN_INSTANCE, (uint8_t *)PDM_Buffer, AUDIO_IN_BUFFER_SIZE);     
}


/**
* @brief  Manages the DMA Half Transfer complete event.
* @param  OutputDevice: the sound terminal device related to the DMA 
*         channel that generates the interrupt
* @retval None
*/
void CCA01M1_AUDIO_OUT_HalfTransfer_CallBack(uint32_t Instance)
{ 
}

/**
* @brief  Manages the DMA Transfer complete event.
* @param  OutputDevice: the sound terminal device related to the DMA 
*         channel that generates the interrupt
* @retval None
*/
void CCA01M1_AUDIO_OUT_TransferComplete_CallBack(uint32_t Instance)
{
}


uint8_t Init_Libraries(void)
{ 
  /*AEC Initialization*/
  
  uint32_t error_value = 0;   
  
  EchoHandlerInstance.tail_length=1024;
  EchoHandlerInstance.preprocess_init = 1;
  EchoHandlerInstance.ptr_primary_channels=1;  
  EchoHandlerInstance.ptr_reference_channels=1;
  EchoHandlerInstance.ptr_output_channels=1;
  AcousticEC_getMemorySize(&EchoHandlerInstance);
  
  EchoHandlerInstance.pInternalMemory = (uint32_t *)malloc(EchoHandlerInstance.internal_memory_size);
  if(EchoHandlerInstance.pInternalMemory == NULL)
  {
    while(1);
  }
  
  error_value = AcousticEC_Init((AcousticEC_Handler_t *)&EchoHandlerInstance);
  if(error_value != 0)
  {
    while(1); 
  }
  
  EchoConfigInstance.preprocess_state = ACOUSTIC_EC_PREPROCESS_ENABLE;
  EchoConfigInstance.AGC_value = 0;
  EchoConfigInstance.noise_suppress_default = -15; /* Default: -15 */
  EchoConfigInstance.echo_suppress_default = -40; /* Default: -40 */ 
  EchoConfigInstance.echo_suppress_active = -15;  /* Default: -15 */
  EchoConfigInstance.residual_echo_remove = 1;    /* Default: 1   */
  
  error_value = AcousticEC_setConfig((AcousticEC_Handler_t *)&EchoHandlerInstance, (AcousticEC_Config_t *) &EchoConfigInstance);
  if(error_value != 0)
  {
    while(1); 
  }
 
  return 0;
}


uint8_t CCA01M1_AUDIO_OUT_ClockConfig(uint16_t OutputDevice,I2S_HandleTypeDef *hi2s, uint32_t AudioFreq, void *Params)
{
  return 0;
}

/**
* @brief Throws Highest priority interrupt
* @param  None
* @retval None
*/
void SW_Task3_Start(void)
{  
  HAL_NVIC_SetPendingIRQ(EXTI3_IRQn);  
}


/**
* @brief Lower priority interrupt handler routine
* @param  None
* @retval None
*/

void SW_Task3_Callback(void)
{  
  AcousticEC_Process((AcousticEC_Handler_t *)&EchoHandlerInstance);   
}

/**
* @brief  Initializes two SW interrupt with different priorities
* @param  None
* @retval None
*/
void SW_IRQ_Tasks_Init(void)
{
  HAL_NVIC_SetPriority((IRQn_Type)EXTI3_IRQn, 0x0A, 0);  
  HAL_NVIC_EnableIRQ((IRQn_Type)EXTI3_IRQn); 
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


