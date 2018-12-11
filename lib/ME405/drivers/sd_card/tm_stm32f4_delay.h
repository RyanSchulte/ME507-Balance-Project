/**
 * @author  Tilen Majerle
 * @email   tilen@majerle.eu
 * @website http://stm32f4-discovery.com
 * @link    http://stm32f4-discovery.com/2014/04/library-03-stm32f429-discovery-system-clock-and-pretty-precise-delay-library/
 * @version v2.2
 * @ide     Keil uVision
 * @license GNU GPL v3
 * @brief   Pretty accurate delay functions with SysTick or any other timer
 *	
@verbatim
   ----------------------------------------------------------------------
    Copyright (C) Tilen Majerle, 2015
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
     
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
@endverbatim
 */
#ifndef TM_DELAY_H
#define TM_DELAY_H 220
/**
 * Library dependencies
 * - STM32F4xx
 * - STM32F4xx RCC
 * - STM32F4xx TIM 			(Only if you want to use TIMx for delay instead of Systick)
 * - MISC         			(Only if you want to use TIMx for delay instead of Systick)
 * - TM TIMER PROPERTIES	(Only if you want to use TIMx for delay instead of Systick)
 * - defines.h
 * - attributes.h
 */
/**
 * Includes
 */
/**
 * @addtogroup TM_STM32F4xx_Libraries
 * @{
 */

/**
 * @defgroup TM_DELAY
 * @brief    Pretty accurate delay functions with SysTick or any other timer - http://stm32f4-discovery.com/2014/04/library-03-stm32f429-discovery-system-clock-and-pretty-precise-delay-library/
 * @{
 *
@verbatim
------!!!!!!!!!!!------
If you are using GCC (sucks) compiler, then your delay is probably totally inaccurate.
USE TIMER FOR DELAY, otherwise your delay will not be accurate

Or use ARM compiler!
------!!!!!!!!!!!------
@endverbatim
 *
 * As of version 2.0 you have now to possible ways to make a delay.
 * The first (and default) is Systick timer. It makes interrupts every 1ms.
 * If you want delay in "us" accuracy, there is simple pooling (variable) mode.
 *
 *
 * The second (better) options is to use one of timers on F4xx MCU.
 * Timer also makes an interrupts every 1ms (for count time) instead of 1us as it was before.
 * For "us" delay, timer's counter is used to count ticks. It makes a new tick each "us".
 * Not all MCUs have all possible timers, so this lib has been designed that you select your own.
 *
 * \par Select custom TIM for delay functions
 *
 * By default, Systick timer is used for delay. If you want your custom timer,
 * open defines.h file, add lines below and edit for your needs:
 *
@verbatim
//Select custom timer for delay, here is TIM2 selected.
//If you want custom TIMx, just replace number "2" for your TIM's number.
#define TM_DELAY_TIM				TIM2
#define TM_DELAY_TIM_IRQ			TIM2_IRQn
#define TM_DELAY_TIM_IRQ_HANDLER	TIM2_IRQHandler
@endverbatim
 *
 *
 * With this setting (using custom timer) you have better accuracy in "us" delay.
 * Also, you have to know, that if you want to use timer for delay, you have to include additional files:
 *
 *	- CMSIS:
 *		- STM32F4xx TIM
 *		- MISC
 *	- TM:
 *		TM TIMER PROPERTIES
 *
 * Delay functions (Delay, Delayms) are now Inline functions.
 * This allows faster execution and more accurate delay.
 *
 * If you are working with Keil uVision and you are using Systick for delay,
 * then set KEIL_IDE define in options for project:
 *	- Open "Options for target"
 *	- Tab "C/C++"
 *	- Under "Define" add "KEIL_IDE", without quotes
 *
 * \par Changelog
 *
@verbatim
 Version 2.2
  - January 12, 2015
  - Added support for custom function call each time 1ms interrupt happen
  - Function is called TM_DELAY_1msHandler(void), with __weak parameter
  - attributes.h file needed
  
 Version 2.1
  - GCC compiler fixes
  - Still prefer that you use TIM for delay if you are working with ARM-GCC compiler

 Version 2.0
  - November 28, 2014
  - Delay library has been totally rewritten. Because Systick is designed to be used
       in RTOS, it is not compatible to use it at the 2 places at the same time.
       For that purpose, library has been rewritten.
  - Read full documentation below

 Version 1.0
  - First release
@endverbatim
 *
 * \par Dependencies
 *
@verbatim
 - STM32F4xx
 - STM32F4xx RCC:       Only if you want to use TIMx for delay instead of Systick
 - STM32F4xx TIM:       Only if you want to use TIMx for delay instead of Systick
 - MISC
 - defines.h
 - TM TIMER PROPERTIES: Only if you want to use TIMx for delay instead of Systick
 - attribute.h
@endverbatim
 */
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "sdio_defines.h"
#include "attributes.h"
/* If user selectable timer is selected for delay */
#if defined(TM_DELAY_TIM)
#include "misc.h"
#include "stm32f4xx_tim.h"
#include "tm_stm32f4_timer_properties.h"
#endif

/**
 * This variable can be used in main
 * It is automatically increased every time systick make an interrupt
 */
extern __IO uint32_t TM_Time;
extern __IO uint32_t TM_Time2;
extern __IO uint32_t mult;

/**
 * @defgroup TM_LIB_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @param  Delays for specific amount of microseconds
 * @param  micros: Time in microseconds for delay
 * @retval None
 * @note   Declared as static inline
 */
static __INLINE void Delay(uint32_t micros) {
#if defined(TM_DELAY_TIM)
	volatile uint32_t timer = TM_DELAY_TIM->CNT;

	do {
		/* Count timer ticks */
		while ((TM_DELAY_TIM->CNT - timer) == 0);

		/* Increase timer */
		timer = TM_DELAY_TIM->CNT;

		/* Decrease microseconds */
	} while (--micros);
#else
	uint32_t amicros;

	/* Multiply micro seconds */
	amicros = (micros) * (mult);

	#ifdef __GNUC__
		if (SystemCoreClock == 180000000 || SystemCoreClock == 100000000) {
			amicros -= mult;
		}
	#endif

	/* If clock is 100MHz, then add additional multiplier */
	/* 100/3 = 33.3 = 33 and delay wouldn't be so accurate */
	#if defined(STM32F411xE)
	amicros += mult;
	#endif

	/* While loop */
	while (amicros--);
#endif /* TM_DELAY_TIM */
}

/**
 * @param  Delays for specific amount of milliseconds
 * @param  millis: Time in milliseconds for delay
 * @retval None
 * @note   Declared as static inline
 */
static __INLINE void Delayms(uint32_t millis) {
	volatile uint32_t timer = TM_Time;

	/* Wait for timer to count milliseconds */
	while ((TM_Time - timer) < millis);
}

/**
 * @brief  Initializes timer settings for delay
 * @note   This function will initialize Systick or user timer, according to settings
 * @param  None
 * @retval None
 */
void TM_DELAY_Init(void);

/**
 * @brief  Gets the TM_Time variable value
 * @param  None
 * @retval Current time in milliseconds
 */
#define TM_DELAY_Time()					(TM_Time)

/**
 * @brief  Sets value for TM_Time variable
 * @param  time: Time in milliseconds
 * @retval None
 */
#define TM_DELAY_SetTime(time)			(TM_Time = (time))

/**
 * @brief  Re-enables delay timer It has to be configured before with TM_DELAY_Init()
 * @note   This function enables delay timer. It can be systick or user selectable timer.
 * @param  None
 * @retval None
 */
void TM_DELAY_EnableDelayTimer(void);

/**
 * @brief  Disables delay timer
 * @note   This function disables delay timer. It can be systick or user selectable timer.
 * @param  None
 * @retval None
 */
void TM_DELAY_DisableDelayTimer(void);

/**
 * @brief  Gets the TM_Time2 variable value
 * @param  None
 * @retval Current time in milliseconds
 * @note   This is not meant for public use
 */
#define TM_DELAY_Time2()				(TM_Time2)

/**
 * @brief  Sets value for TM_Time variable
 * @param  time: Time in milliseconds
 * @retval None
 * @note   This is not meant for public use
 */
#define TM_DELAY_SetTime2(time)			(TM_Time2 = (time))

/**
 * @brief  User function, called each 1ms when interrupt from timer happen
 * @note   Here user should put things which has to be called periodically
 * @param  None
 * @retval None
 * @note   With __weak parameter to prevent link errors if not defined by user
 */
__weak void TM_DELAY_1msHandler(void);

/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */

#endif
