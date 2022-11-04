/**
  ******************************************************************************
  * @file    usbd_audio_if.c
  * @author  SRA
  * 
  * 
  * @brief   USB Device Audio interface file.
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
#include "usbd_audio_if.h"
#include "cca02m2_audio.h"
#include "cca01m1_audio.h"

/** @addtogroup X_NUCLEO_CCA02M1_Applications
* @{
*/ 

/** @addtogroup Microphones_Acquisition
* @{
*/

/** @defgroup usbd_audio_if 
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static int8_t Audio_IN_Init(uint32_t  AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
static int8_t Audio_IN_DeInit(uint32_t options);
static int8_t Audio_IN_Record(void);
static int8_t Audio_IN_VolumeCtl(int16_t Volume);
static int8_t Audio_IN_MuteCtl(uint8_t cmd);
static int8_t Audio_IN_Stop(void);
static int8_t Audio_IN_Pause(void);
static int8_t Audio_IN_Resume(void);
static int8_t Audio_IN_CommandMgr(uint8_t cmd);

static int8_t Audio_OUT_Init(uint32_t Volume, uint32_t  AudioFreq);
static int8_t Audio_OUT_DeInit(uint32_t options);
static int8_t Audio_OUT_Play(uint8_t* pData, uint32_t size);
static int8_t Audio_OUT_VolumeCtl(int16_t Volume);
static int8_t Audio_OUT_MuteCtl(uint8_t cmd);
static int8_t Audio_OUT_Stop(void);
static int8_t Audio_OUT_Pause(void);
static int8_t Audio_OUT_Resume(void);
static uint32_t Audio_OUT_GetStreamingPos(uint32_t totalBufferSize);
static int8_t Audio_OUT_CommandMgr(uint8_t cmd);
static int32_t Audio_OUT_PeriodicTC(uint8_t * data, uint32_t count);


/* Private variables ---------------------------------------------------------*/

extern USBD_HandleTypeDef hUSBDDevice;
USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops = {
  Audio_IN_Init,
  Audio_IN_DeInit,
  Audio_IN_Record,
  Audio_IN_VolumeCtl,
  Audio_IN_MuteCtl,
  Audio_IN_Stop,
  Audio_IN_Pause,
  Audio_IN_Resume,
  Audio_IN_CommandMgr,
  
  Audio_OUT_Init,
  Audio_OUT_DeInit,
  Audio_OUT_Play,
  Audio_OUT_VolumeCtl,
  Audio_OUT_MuteCtl,
  Audio_OUT_Stop,
  Audio_OUT_Pause,
  Audio_OUT_Resume,
  Audio_OUT_GetStreamingPos,
  Audio_OUT_CommandMgr,
  Audio_OUT_PeriodicTC
};

#ifdef USE_STM32L4XX_NUCLEO
extern uint16_t PCM_Buffer[];
#else
extern uint16_t PDM_Buffer[];
#endif


/* Private functions ---------------------------------------------------------*/

/* This table maps the audio device class setting in 1/256 dB to a
* linear 0-64 scaling used in pdm_filter.c. It is computed as
* 256*20*log10(index/64). */
const int16_t vol_table[65] =
{ 0x8000, 0xDBE0, 0xE1E6, 0xE56B, 0xE7EB, 0xE9DB, 0xEB70, 0xECC7,
0xEDF0, 0xEEF6, 0xEFE0, 0xF0B4, 0xF176, 0xF228, 0xF2CD, 0xF366,
0xF3F5, 0xF47C, 0xF4FB, 0xF574, 0xF5E6, 0xF652, 0xF6BA, 0xF71C,
0xF778, 0xF7D6, 0xF82D, 0xF881, 0xF8D2, 0xF920, 0xF96B, 0xF9B4,
0xF9FB, 0xFA3F, 0xFA82, 0xFAC2, 0xFB01, 0xFB3E, 0xFB79, 0xFBB3,
0xFBEB, 0xFC22, 0xFC57, 0xFC8C, 0xFCBF, 0xFCF1, 0xFD22, 0xFD51,
0xFD80, 0xFDAE, 0xFDDB, 0xFE07, 0xFE32, 0xFE5D, 0xFE86, 0xFEAF,
0xFED7, 0xFF00, 0xFF25, 0xFF4B, 0xFF70, 0xFF95, 0xFFB9, 0xFFD0,
0x0000 };
static uint32_t aVolumeLin[] = 
{
   255,241,228,216,204,193,182,172,163,154,146,138,130,123,116,110,104,98,93,
   88,83,79,74,70,67,63,60,56,53,50,48,45,43,40,38,36,34,32,30,29,27,26,24,
   23,22,21,19,18,17,16,16,15,14,13,12,12,11,10,10,9,9,8,8,8,7,7,6,6,6,5,5,
   5,5,4,4,4,4,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1
};


/**
* @brief  Initializes the AUDIO media low layer.
* @param  AudioFreq: Audio frequency used to play the audio stream.
* @param  BitRes: desired bit resolution
* @param  ChnlNbr: number of channel to be configured
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_IN_Init(uint32_t  AudioFreq, uint32_t BitRes, uint32_t ChnlNbr)
{
#ifndef DISABLE_USB_DRIVEN_ACQUISITION 
  
  /* Configure Audio Input peripheral*/  
  CCA02M2_AUDIO_Init_t MicParams;
  MicParams.BitsPerSample = BitRes;
  MicParams.ChannelsNbr = ChnlNbr;
  MicParams.Device = AUDIO_IN_DIGITAL_MIC;
  MicParams.SampleRate = AudioFreq;
  MicParams.Volume = AUDIO_VOLUME_INPUT;
  
  return CCA02M2_AUDIO_IN_Init(CCA02M2_AUDIO_IN_INSTANCE, &MicParams);  
#endif
  return AUDIO_OK;
}

/**
* @brief  De-Initializes the AUDIO media low layer.      
* @param  options: Reserved for future use
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_IN_DeInit(uint32_t options)
{
  return AUDIO_OK;
}

/**
* @brief  Start audio recording engine
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_IN_Record(void)
{
#ifndef DISABLE_USB_DRIVEN_ACQUISITION  
  return CCA02M2_AUDIO_IN_Record(CCA02M2_AUDIO_IN_INSTANCE, (uint8_t *)PDM_Buffer, AUDIO_IN_BUFFER_SIZE);   
#endif
  return AUDIO_OK;  
}

/**
* @brief  Controls AUDIO Volume.             
* @param  vol: Volume level
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_IN_VolumeCtl(int16_t Volume)
{ 
  /* Call low layer volume setting function */
  uint32_t j, mic_instance;
  
  j = 0;
  /* Find the setting nearest to the desired setting */
  while(j<64 &&
        abs(Volume-vol_table[j]) > abs(Volume-vol_table[j+1])) {
          j++;
        }
  mic_instance = 0;
  /* Now do the volume adjustment */
  return CCA02M2_AUDIO_IN_SetVolume(mic_instance, j);  
}

/**
* @brief  Controls AUDIO Mute.              
* @param  cmd: Command opcode
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_IN_MuteCtl(uint8_t cmd)
{
  return AUDIO_OK;
}

/**
* @brief  Stops audio acquisition
* @param  none
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_IN_Stop(void)
{  
#ifndef DISABLE_USB_DRIVEN_ACQUISITION  
  return CCA02M2_AUDIO_IN_Stop(CCA02M2_AUDIO_IN_INSTANCE);    
#endif
  return AUDIO_OK;
}

/**
* @brief  Pauses audio acquisition
* @param  none
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_IN_Pause(void)
{
  return AUDIO_OK;
}

/**
* @brief  Resumes audio acquisition
* @param  none
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_IN_Resume(void)
{  
  return AUDIO_OK;
}

/**
* @brief  Manages command from usb
* @param  None
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/

static int8_t Audio_IN_CommandMgr(uint8_t cmd)
{
  return AUDIO_OK;
}
/**
* @brief  Fills USB audio buffer with the right amount of data, depending on the
*			channel/frequency configuration
* @param  audioData: pointer to the PCM audio data
* @param  PCMSamples: number of PCM samples to be passed to USB engine
* @note Depending on the calling frequency, a coherent amount of samples must be passed to
*       the function. E.g.: assuming a Sampling frequency of 16 KHz and 1 channel,
*       you can pass 16 PCM samples if the function is called each millisecond,
*       32 samples if called every 2 milliseconds and so on.
*/
void Send_Audio_to_USB(int16_t * audioData, uint16_t PCMSamples)
{  
  USBD_AUDIO_Data_Transfer(&hUSBDDevice, (int16_t *)audioData, PCMSamples);
}

/**
* @brief  Initializes the AUDIO OUT media low layer.
* @param  AudioFreq: Audio frequency used to play the audio stream.
* @param  Volume: volume to be configured
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_Init(uint32_t Volume, uint32_t  AudioFreq)
{
  uint16_t volume_temp = AUDIO_VOLUME_OUT;
  
  /* Configure Audio Output peripheral (SAI) and external DAC */
  
  CCA01M1_AUDIO_Init_t OutParams;
  OutParams.BitsPerSample = 16;
  OutParams.ChannelsNbr = 1;
  OutParams.Device = STA350BW_1;
  OutParams.SampleRate = AudioFreq;
  OutParams.Volume = volume_temp;
  
  return CCA01M1_AUDIO_OUT_Init(CCA01M1_AUDIO_OUT_INSTANCE, &OutParams);
}

/**
* @brief  Deinit the AUDIO OUT media low layer.
* @param  Option.
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_DeInit(uint32_t options)
{
  return AUDIO_OK;
}

/**
* @brief  Starts audio streaming to the AUDIO OUT media low layer.
* @param  pData: buffer to be played.
* @param  size: buffer size in int16_t
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_Play(uint8_t* pData, uint32_t size){
  
  CCA01M1_AUDIO_OUT_Play(CCA01M1_AUDIO_OUT_INSTANCE, pData, size/2);
  return AUDIO_OK;
}

/**
* @brief  Controls AUDIO Volume.             
* @param  vol: Volume level
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_VolumeCtl(int16_t Volume)
{
  if (Volume > -1 && Volume < 100)
  {
    CCA01M1_AUDIO_OUT_SetVolume(CCA01M1_AUDIO_OUT_INSTANCE,aVolumeLin[Volume]);  
  }
  return AUDIO_OK;
}

/**
* @brief  Controls AUDIO Mute.             
* @param  cmd: command
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_MuteCtl(uint8_t cmd)
{
  return AUDIO_OK;
}

/**
* @brief  Stops audio streaming
* @param  none
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_Stop(void)
{
  CCA01M1_AUDIO_OUT_Stop(CCA01M1_AUDIO_OUT_INSTANCE);
  return AUDIO_OK;
}

/**
* @brief  Pause audio streaming
* @param  none
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_Pause(void)
{
  CCA01M1_AUDIO_OUT_Pause(CCA01M1_AUDIO_OUT_INSTANCE);
  return AUDIO_OK;
}

/**
* @brief  Resume audio streaming
* @param  none
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_Resume(void)
{
  CCA01M1_AUDIO_OUT_Resume(CCA01M1_AUDIO_OUT_INSTANCE);
  return AUDIO_OK;
}

/**
* @brief  Return the reading  position of I2S buffer
* @param  totalBufferSize total size of the buffer
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static uint32_t Audio_OUT_GetStreamingPos(uint32_t totalBufferSize)
{  
  return (totalBufferSize) - ((hAudioOut[1].hdmatx->Instance->NDTR)) * 2 ;  
}

/**
* @brief  Manage optional commands
* @param  cmd command
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int8_t Audio_OUT_CommandMgr(uint8_t cmd)
{
  return AUDIO_OK;
}

/**
* @brief  Manage optional commands
* @param  cmd command
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
static int32_t Audio_OUT_PeriodicTC(uint8_t * data, uint32_t count)
{  
  return AUDIO_OK;
}

///**
//* @brief  Function to be called periodically (e.g in the systick handles), it handles timeout of the OUT streaming
//* @param  none
//* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
//*/
//void Audio_OUT_IncTick(void)
//{
//  USBD_AUDIO_HandleTypeDef   *haudio = (USBD_AUDIO_HandleTypeDef*)hUSBDDevice.pClassData;
//  
//  if(USBD_AUDIO_Get_OUT_State(&hUSBDDevice) == STATE_USB_BUFFER_READ_STARTED)
//  {   
//    haudio->OUT_timeout++;
//  }
//  
//  if (haudio->OUT_timeout >= 2 && USBD_AUDIO_Get_OUT_State(&hUSBDDevice) == STATE_USB_BUFFER_READ_STARTED)
//  {
//    CCA01M1_AUDIO_OUT_Stop(CCA01M1_AUDIO_OUT_INSTANCE);
//    USBD_AUDIO_Set_OUT_State(&hUSBDDevice, STATE_USB_IDLE );
//    
//  }
//}

/**
* @brief  Gets OUT streaming state
* @param  none
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
uint8_t Audio_OUT_GetState(void)
{  
  return USBD_AUDIO_Get_OUT_State(&hUSBDDevice);
}

/**
* @brief  Sets OUT streaming state
* @param  state: new state
* @retval AUDIO_OK in case of success, AUDIO_ERROR otherwise
*/
uint8_t Audio_OUT_SetState(uint8_t state)
{  
  return USBD_AUDIO_Set_OUT_State(&hUSBDDevice, state );
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


