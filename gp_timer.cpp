#include "gp_timer.h"
#include "digital_inout.h"
#include "alternate_functions.h"
#include "stm32h7xx.h"

volatile uint16_t * __gp_timx_cr1[7] = {&(TIM2->CR1), &(TIM3->CR1), &(TIM4->CR1), &(TIM5->CR1),
										&(TIM15->CR1), &(TIM16->CR1), &(TIM17->CR1)};

volatile uint32_t * __gp_timx_dier[7] = {&(TIM2->DIER), &(TIM3->DIER), &(TIM4->DIER), &(TIM5->DIER),
										 &(TIM15->DIER), &(TIM16->DIER), &(TIM17->DIER)};

volatile uint32_t * __gp_timx_egr[7] = {&(TIM2->EGR), &(TIM3->EGR), &(TIM4->EGR), &(TIM5->EGR),
										&(TIM15->EGR), &(TIM16->EGR), &(TIM17->EGR)};

volatile uint16_t * __gp_timx_psc[7] = {&(TIM2->PSC), &(TIM3->PSC), &(TIM4->PSC), &(TIM5->PSC),
										&(TIM15->PSC), &(TIM16->PSC), &(TIM17->PSC)};

volatile uint32_t * __gp_timx_arr[7] = {&(TIM2->ARR), &(TIM3->ARR), &(TIM4->ARR), &(TIM5->ARR),
										&(TIM15->ARR), &(TIM16->ARR), &(TIM17->ARR)};

volatile uint32_t * __gp_timx_ccmr1[7] = {&(TIM2->CCMR1), &(TIM3->CCMR1), &(TIM4->CCMR1), &(TIM5->CCMR1),
										  &(TIM15->CCMR1), &(TIM16->CCMR1), &(TIM17->CCMR1)};

volatile uint32_t * __gp_timx_ccmr2[7] = {&(TIM2->CCMR2), &(TIM3->CCMR2), &(TIM4->CCMR2), &(TIM5->CCMR2),
										  NULL, NULL, NULL};

volatile uint32_t * __gp_timx_ccer[7] = {&(TIM2->CCER), &(TIM3->CCER), &(TIM4->CCER), &(TIM5->CCER),
										 &(TIM15->CCER), &(TIM16->CCER), &(TIM17->CCER)};

volatile uint32_t * __gp_timx_ccr1[7] = {&(TIM2->CCR1), &(TIM3->CCR1), &(TIM4->CCR1), &(TIM5->CCR1),
										 &(TIM15->CCR1), &(TIM16->CCR1), &(TIM17->CCR1)};

volatile uint32_t * __gp_timx_ccr2[7] = {&(TIM2->CCR2), &(TIM3->CCR2), &(TIM4->CCR2), &(TIM5->CCR2),
										 &(TIM15->CCR2), NULL, NULL};

volatile uint32_t * __gp_timx_ccr3[7] = {&(TIM2->CCR3), &(TIM3->CCR3), &(TIM4->CCR3), &(TIM5->CCR3),
										 NULL, NULL, NULL};

volatile uint32_t * __gp_timx_ccr4[7] = {&(TIM2->CCR4), &(TIM3->CCR4), &(TIM4->CCR4), &(TIM5->CCR4),
										 NULL, NULL, NULL};

volatile uint32_t ** __gp_timx_ccr[4] = {__gp_timx_ccr1, __gp_timx_ccr2, __gp_timx_ccr3, __gp_timx_ccr4};

IRQn_Type __gp_timx_irqn[7]  = {TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn,
								TIM15_IRQn, TIM16_IRQn, TIM17_IRQn};

uint32_t __gp_timx_rcc_en[7] = {RCC_APB1LENR_TIM2EN, RCC_APB1LENR_TIM3EN, RCC_APB1LENR_TIM4EN, RCC_APB1LENR_TIM5EN,
								RCC_APB2ENR_TIM15EN, RCC_APB2ENR_TIM16EN, RCC_APB2ENR_TIM17EN};

uint32_t __gp_timx_ocxm[4] = {TIM_CCMR1_OC1M, TIM_CCMR1_OC2M, TIM_CCMR2_OC3M, TIM_CCMR2_OC4M};
uint32_t __gp_timx_ocxce[4] = {TIM_CCMR1_OC1CE, TIM_CCMR1_OC2CE, TIM_CCMR2_OC3CE, TIM_CCMR2_OC4CE};
uint32_t __gp_timx_ocxpe[4] = {TIM_CCMR1_OC1PE, TIM_CCMR1_OC2PE, TIM_CCMR2_OC3PE, TIM_CCMR2_OC4PE};
uint32_t __gp_timx_ccxp[4] = {TIM_CCER_CC1P, TIM_CCER_CC2P, TIM_CCER_CC3P, TIM_CCER_CC4P};
uint32_t __gp_timx_ccxe[4] = {TIM_CCER_CC1E, TIM_CCER_CC2E, TIM_CCER_CC3E, TIM_CCER_CC4E};

uint32_t __gp_timx_ocxm_mode1[4] = {0x06U << 4U, 0x06U << 12U, 0x06U << 4U, 0x06U << 12U};

void (*__gp_timx_irq[7])(void);

void __init_gp_timer(uint8_t tim, uint32_t tics) {
	*__gp_timx_cr1[tim] &= ~TIM_CR1_CEN;
	__gp_timx_irq[tim] = NULL;

	if (tim < TIMER15) RCC->APB1LENR |= __gp_timx_rcc_en[tim];
	else RCC->APB2ENR |= __gp_timx_rcc_en[tim];

	//buffered auto-reload
	*__gp_timx_cr1[tim] |= TIM_CR1_ARPE;

	//prescaler = 0
	*__gp_timx_psc[tim] = 0x00;

	//set auto-reload register
	*__gp_timx_arr[tim] = tics;

	//update registers once
	*__gp_timx_egr[tim] |= TIM_EGR_UG;

	//enable timer
	*__gp_timx_cr1[tim] |= TIM_CR1_CEN;
}

void __attach_fn(uint8_t tim, void (*fn)(void)) {
	//disable timer
	*__gp_timx_cr1[tim] &= ~TIM_CR1_CEN;

	//attach function
	__gp_timx_irq[tim] = fn;

	//update interrupt enable
	NVIC_EnableIRQ(__gp_timx_irqn[tim]);
	*__gp_timx_dier[tim] |= TIM_DIER_UIE;

	//enable timer
	*__gp_timx_cr1[tim] |= TIM_CR1_CEN;
}

void __enable_pwm(uint8_t tim, uint8_t ch) {
	if (ch < 1) return; //1-indexed!
	if (tim > TIMER5 && ch > 2) return; //15, 16, 17 1 or 2 channels only
	if (tim > TIMER15 && ch > 1) return; //16, 17 1 chanel only

	//disable timer
	*__gp_timx_cr1[tim] &= ~TIM_CR1_CEN;

	//figure out which CCMR register controls this channel
	volatile uint32_t * ccmr;

	if (ch > 2) ccmr = __gp_timx_ccmr2[tim];
	else ccmr = __gp_timx_ccmr1[tim];

	//OCxM = 0110 (PWM mode 1)
	*ccmr &= ~__gp_timx_ocxm[ch - 1];
	*ccmr |= __gp_timx_ocxm_mode1[ch - 1];

	//capture compare preload enable
	*ccmr |= __gp_timx_ocxpe[ch - 1];

	//duty cycle = 0;
	*__gp_timx_ccr[ch - 1][tim] = 0;

	//output active high
	*__gp_timx_ccer[tim] &= ~__gp_timx_ccxp[ch - 1];

	//output enable
	*__gp_timx_ccer[tim] |= __gp_timx_ccxe[ch - 1];

	//update registers once
	*__gp_timx_egr[tim] |= TIM_EGR_UG;

	//enable timer
	*__gp_timx_cr1[tim] |= TIM_CR1_CEN;
}

void __set_on_time(uint8_t tim, uint8_t ch, uint32_t tics) {
	*__gp_timx_ccr[ch - 1][tim] = tics;
}

void tickerStart(uint8_t tim, uint32_t tics) {
	__init_gp_timer(tim, tics);
}

void tickerStart_usecs(uint8_t tim, uint32_t usecs) {
	uint64_t tics;
	tics = SystemCoreClock / 2 / 1000000 * usecs;
	__init_gp_timer(tim, (uint32_t)tics);
}

void tickerStart_secs(uint8_t tim, double secs) {
	double tics;
	tics = SystemCoreClock / 2 * secs;
	__init_gp_timer(tim, (uint32_t)tics);
}

void tickerAttach(uint8_t tim, void (*fn)(void)) {
	__attach_fn(tim, fn);
}

void pwmOut(uint8_t tim, uint8_t ch, uint8_t port, uint8_t pin, uint8_t af) {
	__init_gp_timer(tim, SystemCoreClock / 2 / 10000);
	__map_af(port, pin, af);
	__enable_pwm(tim, ch);
}

void pwmOutFreq(uint8_t tim, uint8_t ch, uint32_t freq) {
	uint32_t tics = SystemCoreClock / 2 / freq;
	*__gp_timx_arr[tim] = tics;
}

void pwmOutDtc(uint8_t tim, uint8_t ch, float dtc) {
	float fperiod = (float)(*__gp_timx_arr[tim]);
	uint32_t tics = (uint32_t)(dtc * fperiod);
	__set_on_time(tim, ch, tics);
}
