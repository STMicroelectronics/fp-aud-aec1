## __AEC application__

FP-AUD-AEC1 STM32Cube Function Pack for Acoustic Echo Cancellation is a specific
example fully focused on Acoustic Echo Cancellation and provides an implementation 
of a USB smart speaker use case with microphone

The package includes the AcousticEC library,that provides an implementation for a real-time echo 
cancellation routine based on the well-known SPEEX implementation of the MDF 
algorithm. 

The firmware provides implementation example for STM32 NUCLEO-F446RE board or  
STM32 NUCLEO-F746 equipped with:
-	X-NUCLEO-CCA01M1, an expansion board based on the STA350BW Sound Terminal® 
	2.1-channel high-efficiency digital audio output system.
-	X-NUCLEO-CCA02M2, an evaluation board based on digital MEMS microphones, designed 
	around STMicroelectronics MP34DT06J digital microphones.

 After the initialization of all the required elements (microphones, audio output, 
 USB communication and streaming) digital MEMS microphone acquisition starts and 
 drives the whole application: when a millisecond of the microphones signal is made 
 available by the BSP layer, several operations are performed concurrently, 
 depending on the current status of the application.

Then the processed audio is streamed to 2 interfaces at the same time: 
-	 USB: the device is recognized by a host PC as a standard USB as a USB microphone 
and USB speaker at the same time. It can be used to record and save the real time audio streaming
and, in a meanwhile, to send an audio signal to the speaker.
The stereo track contains the processed signal (L channel) and an omnidirectional microphone 
as a reference (R channel)
-	I2S: an I2S peripheral of the MCU is connected to the STA350BW Sound Terminal® 
device mounted on the X-NUCLEO-CCA01M1 board. In this way the processed signal can 
be reproduced by a loudspeaker connected to the expansion board.

The HCLK is configured at 168 MHz for STM32F446xx Devices and 144 MHz for STM32F746xx Devices.

@note Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.
      
@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@note The clock setting is configured to have an high product performance (high clock frequency) 
      so not optimized in term of power consumption.

### __Hardware and Software environment__

  - This example runs on STM32F446xx and STM32F746xx devices.
    
  - This example has been tested with STMicroelectronics STM32F4xx-Nucleo rev C
    and STM32F7xx-Nucleo rev B boards and can be easily tailored to any other supported device 
    and development board.    

### __How to use it ?__

In order to make the program work, you must do the following :
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the example

