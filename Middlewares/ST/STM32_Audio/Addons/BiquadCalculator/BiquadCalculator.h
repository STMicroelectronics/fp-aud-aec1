/**
******************************************************************************
* @file    BiquadCalculator.h
* @author  Central Labs
* @version V1.0.0
* @date    30-June-2015
* @brief   This file contains definitions for BiquadCalculator.c functions
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2019 STMicroelectronics</center></h2>
*
* Licensed under Software License Agreement SLA0077, (the "License"). 
* You may not use this package except in compliance with the License. 
* You may obtain a copy of the License at:
*
* http://www.st.com/content/st_com/en/search.html#q=SLA0077-t=keywords-page=1
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SOUNDTERMINALEQ_H
#define __SOUNDTERMINALEQ_H

#ifdef __cplusplus
extern "C" {
#endif 
  
#ifndef PI
#define PI 3.141592653589793f
#endif
  
#include "stdint.h"
#include "math.h"
  
  
#define K_NUM 5
  
  /** @addtogroup MIDDLEWARES
  * @{
  */
  
  /** @addtogroup SOUND_TERMINAL_BIQUAD_CALCULATOR
  * @{
  */
  
  /** @defgroup SOUND_TERMINAL_BIQUAD_CALCULATOR_Exported_Types_Definitions 
  * @{
  */
  
  /** 
  * @brief Sound terminal biquad filter structure. It keeps information about
  *        all the filter parameters and setting, as well as the computed filter.
  */ 
  typedef struct
  {
    uint32_t		Type;                    /*!< Filter type, This parameter can be a value of @ref Biquad_Calculator_filter_types */
    uint32_t		Fs;                      /*!< Sampling frequency */
    uint32_t		Fc;                      /*!< Cut frequency */
    float	        Q;                       /*!< Quality factor (not applicable for low and high-shelf filters) */
    float 		Slope;                   /*!< Slope: applicable only for low and high-shelf filters */       
    float		Gain;                    /*!< The boost or the attenuation at f = fc */
    uint32_t 	        Coefficients[K_NUM];     /*!< Memory area that will contain computed coefficients */
  }BIQUAD_Filter_t;
  
  /**
  * @}
  */
  
  /** @defgroup  SOUND_TERMINAL_BIQUAD_CALCULATOR_Exported_Constants 
  * @{
  */
  
  /** @defgroup Biquad_Calculator_Filter_Types Biquad Calculator Filter Types
  * @brief Equalizer Type definition
  * @{
  */
#define BIQUAD_CALCULATOR_FO_LPF   			((uint32_t)0x00000000)  /*!< First order low pass filter */
#define BIQUAD_CALCULATOR_FO_HPF    			((uint32_t)0X00000001)  /*!< First order high pass filter*/
#define BIQUAD_CALCULATOR_SO_LPF   			((uint32_t)0x00000002)  /*!< Second order low pass filter*/
#define BIQUAD_CALCULATOR_SO_HPF    			((uint32_t)0X00000003)  /*!< Second order low pass filter*/
#define BIQUAD_CALCULATOR_LOW_SHELF   		        ((uint32_t)0x00000004)  /*!< Low shelf filter */
#define BIQUAD_CALCULATOR_HIGH_SHELF    		((uint32_t)0X00000005)  /*!< High shelf filter */
#define BIQUAD_CALCULATOR_NOTCH   			((uint32_t)0x00000006)  /*!< Notch filter */
#define BIQUAD_CALCULATOR_ALL_PASS    		        ((uint32_t)0X00000007)  /*!< All pass filter */
#define BIQUAD_CALCULATOR_BAND_PASS   		        ((uint32_t)0x00000008)  /*!< Band pass filter */
#define BIQUAD_CALCULATOR_PEAK    			((uint32_t)0X00000009)  /*!< Peak filter */
  /**
  * @}
  */ 
  
  /** @defgroup  Biquad_Calculator_Return_values_definition Biquad Calculator Return values definition
  * @brief Return values definition
  * @{
  */
#define BIQUAD_RANGE_ONE                                ((int32_t) 1) /*!< filter coefficeints in the [-1 1) range */
#define BIQUAD_RANGE_TWO                                ((int32_t) 2) /*!< filter coefficeints in the [-2 2) range */
#define BIQUAD_RANGE_FOUR                               ((int32_t) 4) /*!< filter coefficeints in the [-4 4) range */
  
#define BIQUAD_CALCULATOR_OK                            ((int32_t) 0)
#define BIQUAD_CALCULATOR_ERROR                         ((int32_t) -1)    
  
  /**
  * @}
  */
  
  /**
  * @}
  */
  
  /** @defgroup  SOUND_TERMINAL_BIQUAD_CALCULATOR_Functions 
  * @{ */  
  int32_t BQ_CALC_ComputeFilter(BIQUAD_Filter_t *pEq);
  int32_t BQ_CALC_ShiftCoefficients(BIQUAD_Filter_t *pEq, uint8_t coeffRange);
  /**
  * @}
  */
  
  /**
  * @}
  */
  
  /**
  * @}
  */
  
#ifdef __cplusplus
}
#endif

#endif /* __SOUNDTERMINALEQ_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/



