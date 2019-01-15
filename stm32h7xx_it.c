/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "stm32h7xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32h7xx_it.h"
#include "gp_timer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
#ifdef USE_RTOS_SYSTICK
	osSystickHandler();
#endif
}

__weak void TIM2_IRQHandler() {
	if (TIM2->SR & TIM_SR_UIF) {
		if (__gp_timx_irq[TIMER2] != NULL) (*__gp_timx_irq[TIMER2])();
	}
	TIM2->SR = 0x00;
}

__weak void TIM3_IRQHandler() {
	if (TIM3->SR & TIM_SR_UIF) {
		if (__gp_timx_irq[TIMER3] != NULL) (*__gp_timx_irq[TIMER3])();
	}
	TIM3->SR = 0x00;
}

__weak void TIM4_IRQHandler() {
	if (TIM4->SR & TIM_SR_UIF) {
		if (__gp_timx_irq[TIMER4] != NULL) (*__gp_timx_irq[TIMER4])();
	}
	TIM4->SR = 0x00;
}

__weak void TIM5_IRQHandler() {
	if (TIM5->SR & TIM_SR_UIF) {
		if (__gp_timx_irq[TIMER5] != NULL) (*__gp_timx_irq[TIMER5])();
	}
	TIM5->SR = 0x00;
}

__weak void TIM15_IRQHandler() {
	if (TIM15->SR & TIM_SR_UIF) {
		if (__gp_timx_irq[TIMER15] != NULL) (*__gp_timx_irq[TIMER15])();
	}
	TIM15->SR = 0x00;
}

__weak void TIM16_IRQHandler() {
	if (TIM16->SR & TIM_SR_UIF) {
		if (__gp_timx_irq[TIMER16] != NULL) (*__gp_timx_irq[TIMER16])();
	}
	TIM16->SR = 0x00;
}

__weak void TIM17_IRQHandler() {
	if (TIM17->SR & TIM_SR_UIF) {
		if (__gp_timx_irq[TIMER17] != NULL) (*__gp_timx_irq[TIMER17])();
	}
	TIM17->SR = 0x00;
}
