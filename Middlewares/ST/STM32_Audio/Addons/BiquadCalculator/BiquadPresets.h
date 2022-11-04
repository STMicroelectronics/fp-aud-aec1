/**
******************************************************************************
* @file    BiquadPresets.h
* @author  Central Labs
* @version V1.0.0
* @date    18-August-2015
* @brief   This file contains preset variable definitions
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
#ifndef __BIQUAD_PRESETS_H
#define __BIQUAD_PRESETS_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {  
#endif
  
extern uint32_t FLAT_EQ_PRESET[];
extern uint32_t ROCK_EQ_PRESET[];
extern uint32_t SOFT_ROCK_EQ_PRESET[];
extern uint32_t JAZZ_EQ_PRESET[];
extern uint32_t CLASSICAL_EQ_PRESET[];
extern uint32_t DANCE_EQ_PRESET[];
extern uint32_t POP_EQ_PRESET[];
extern uint32_t SOFT_EQ_PRESET[];
extern uint32_t HARD_EQ_PRESET[];
extern uint32_t PARTY_EQ_PRESET[];
extern uint32_t VOCAL_EQ_PRESET[];
extern uint32_t HIPHOP_EQ_PRESET[];
extern uint32_t DIALOG_EQ_PRESET[];
extern uint32_t BASS_BOOST1_EQ_PRESET[];
extern uint32_t BASS_BOOST2_EQ_PRESET[];
extern uint32_t BASS_BOOST3_EQ_PRESET[];
  
#ifdef __cplusplus
}
#endif

#endif /*__BIQUAD_PRESETS_H*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
