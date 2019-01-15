#include "analog_in.h"
#include "digital_inout.h"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stdio.h"

volatile uint32_t * __adcx_cr[3] = {&(ADC1->CR), &(ADC2->CR), &(ADC3->CR)};
volatile uint32_t * __adcx_rcc[3] = {&(RCC->AHB1ENR), &(RCC->AHB1ENR), &(RCC->AHB4ENR)};
volatile uint32_t * __adcx_cfgr[3] = {&(ADC1->CFGR), &(ADC2->CFGR), &(ADC3->CFGR)};
volatile uint32_t * __adcx_ccr[3] = {&(ADC12_COMMON->CCR), &(ADC12_COMMON->CCR), &(ADC3_COMMON->CCR)};
volatile uint32_t * __adcx_isr[3] = {&(ADC1->ISR), &(ADC2->ISR), &(ADC3->ISR)};
volatile uint32_t * __adcx_sqr1[3] = {&(ADC1->SQR1), &(ADC2->SQR1), &(ADC3->SQR1)};
volatile uint32_t * __adcx_dr[3] = {&(ADC1->DR), &(ADC2->DR), &(ADC3->DR)};
volatile uint32_t * __adcx_pcsel[3] = {&(ADC1->PCSEL), &(ADC2->PCSEL), &(ADC3->PCSEL)};

volatile uint32_t __adc_rcc_adcxen[3] = {RCC_AHB1ENR_ADC12EN, RCC_AHB1ENR_ADC12EN, RCC_AHB4ENR_ADC3EN};

void __gpio_set_analog(uint8_t port, uint8_t pin) {
	//gpio clock enable
	RCC->AHB4ENR |= __gpiox_rcc_en[port];

	//analog input
	*__gpiox_moder[port] |= 1 << (2 * pin);
	*__gpiox_moder[port] |= 1 << (2 * pin + 1);
}

void __init_adc(uint8_t adc, uint8_t bits) {
	//set ADC clock source to per_ck
	RCC->D3CCIPR &= ~RCC_D3CCIPR_ADCSEL;
	RCC->D3CCIPR |= (0x2U << 16U);

	//enable ADCx clock
	*__adcx_rcc[adc] |= __adc_rcc_adcxen[adc];

	//ADCx clock prescaler = 4
	*__adcx_ccr[adc] &= ~ADC_CCR_PRESC;
	*__adcx_ccr[adc] |= 0x2U << 18U;

	//set data resolution
	*__adcx_cfgr[adc] &= ~ADC_CFGR_RES;
	uint8_t res_code = (16 - bits) >> 1;
	*__adcx_cfgr[adc] |= res_code << 2U;

	//exit deep power-down
	*__adcx_cr[adc] &= ~ADC_CR_DEEPPWD;

	//enable ADCx voltage regulator
	*__adcx_cr[adc] |= ADC_CR_ADVREGEN;

	//wait a while
	volatile uint32_t delay;
	for (delay = 0; delay < 4000; delay++) {}

	//enable BOOST MODE
	*__adcx_cr[adc] |= ADC_CR_BOOST;

	//launch ADC calibration
	*__adcx_cr[adc] &= ~ADC_CR_ADEN;
	*__adcx_cr[adc] |= ADC_CR_ADCALLIN;
	*__adcx_cr[adc] |= ADC_CR_ADCAL;

	while ((*__adcx_cr[adc] & ADC_CR_ADCAL) != 0) {}

	//enable adc
	*__adcx_isr[adc] |= ADC_ISR_ADRD;
	*__adcx_cr[adc] |= ADC_CR_ADEN;

	while ((*__adcx_isr[adc] & ADC_ISR_ADRD) == 0) {}
}

uint32_t __adc_single_conversion(uint8_t adc, uint8_t ch) {
	//sequence length = 1
	*__adcx_sqr1[adc] &= ~ADC_SQR1_L;
	*__adcx_sqr1[adc] |= 0x0;

	//1st conversion in regular sequence = ch
	*__adcx_pcsel[adc] |= (1 << ch);
	*__adcx_sqr1[adc] &= ~ADC_SQR1_SQ1;
	*__adcx_sqr1[adc] |= (ch << 6U);

	//start conversion
	*__adcx_cr[adc] |= ADC_CR_ADSTART;

	//wait for completion
	while ((*__adcx_isr[adc] & ADC_ISR_EOC) == 0) {}

	//un-preselect channel
	*__adcx_pcsel[adc] &= ~(1 << ch);

	//return result
	return *__adcx_dr[adc];
}

void __adc_double_conversion(uint8_t adc1, uint8_t ch1, uint8_t adc2, uint8_t ch2, uint32_t * r1, uint32_t * r2) {
	//sequence length = 1
	*__adcx_sqr1[adc1] &= ~ADC_SQR1_L;
	*__adcx_sqr1[adc1] |= 0x0;
	*__adcx_sqr1[adc2] &= ~ADC_SQR1_L;
	*__adcx_sqr1[adc2] |= 0x0;

	//1st conversion in regular sequence = ch
	*__adcx_pcsel[adc1] |= (1 << ch1);
	*__adcx_sqr1[adc1] &= ~ADC_SQR1_SQ1;
	*__adcx_sqr1[adc1] |= (ch1 << 6U);
	*__adcx_pcsel[adc2] |= (1 << ch2);
	*__adcx_sqr1[adc2] &= ~ADC_SQR1_SQ1;
	*__adcx_sqr1[adc2] |= (ch2 << 6U);

	//start conversions
	*__adcx_cr[adc1] |= ADC_CR_ADSTART;
	*__adcx_cr[adc2] |= ADC_CR_ADSTART;

	//wait for completion
	while ((*__adcx_isr[adc1] & ADC_ISR_EOC) == 0) {}

	//un-preselect channels
	*__adcx_pcsel[adc1] &= ~(1 << ch1);
	*__adcx_pcsel[adc2] &= ~(1 << ch2);

	//return result
	*r1 = *__adcx_dr[adc1];
	*r2 = *__adcx_dr[adc2];
}

void __adc_triple_conversion(uint8_t ch1, uint8_t ch2, uint8_t ch3, uint32_t * r1, uint32_t * r2, uint32_t * r3) {
	//sequence length = 1
	*__adcx_sqr1[AD1] &= ~ADC_SQR1_L;
	*__adcx_sqr1[AD1] |= 0x0;
	*__adcx_sqr1[AD2] &= ~ADC_SQR1_L;
	*__adcx_sqr1[AD2] |= 0x0;
	*__adcx_sqr1[AD3] &= ~ADC_SQR1_L;
	*__adcx_sqr1[AD3] |= 0x0;

	//1st conversion in regular sequence = ch
	*__adcx_sqr1[AD1] &= ~ADC_SQR1_SQ1;
	*__adcx_sqr1[AD1] |= (ch1 << 6U);
	*__adcx_pcsel[AD1] |= (1 << ch1);
	*__adcx_sqr1[AD2] &= ~ADC_SQR1_SQ1;
	*__adcx_sqr1[AD2] |= (ch2 << 6U);
	*__adcx_pcsel[AD2] |= (1 << ch2);
	*__adcx_sqr1[AD3] &= ~ADC_SQR1_SQ1;
	*__adcx_sqr1[AD3] |= (ch2 << 6U);
	*__adcx_pcsel[AD3] |= (1 << ch3);

	//start conversions
	*__adcx_cr[AD1] |= ADC_CR_ADSTART;
	*__adcx_cr[AD2] |= ADC_CR_ADSTART;
	*__adcx_cr[AD3] |= ADC_CR_ADSTART;

	//wait for completion
	while ((*__adcx_isr[AD1] & ADC_ISR_EOC) == 0) {}

	//un-preselect channels
	*__adcx_pcsel[AD1] &= ~(1 << ch1);
	*__adcx_pcsel[AD2] &= ~(1 << ch2);
	*__adcx_pcsel[AD3] &= ~(1 << ch3);

	//return result
	*r1 = *__adcx_dr[AD1];
	*r2 = *__adcx_dr[AD2];
	*r3 = *__adcx_dr[AD3];
}

void analogIn(uint8_t port, uint8_t pin, uint8_t adc) {
	__gpio_set_analog(port, pin);
	__init_adc(adc, 12);
}

uint32_t analogRead(uint8_t adc, uint8_t ch) {
	return __adc_single_conversion(adc, ch);
}
