#include "alternate_functions.h"
#include "digital_inout.h"
#include "stm32h7xx.h"

void __map_af(uint8_t port, uint8_t pin, uint8_t af) {
	//gpio clock enable
	RCC->AHB4ENR |= __gpiox_rcc_en[port];

	//gpio alternate function mode
	*__gpiox_moder[port] &= ~(1 << (2 * pin));
	*__gpiox_moder[port] |= (1 << (2 * pin + 1));

	//which AFR register?
	volatile uint32_t * afr;
	if (pin > 7) afr = __gpiox_afrh[port];
	else afr = __gpiox_afrl[port];
	if (pin > 7) pin -= 8;

	//set alternate function
	*afr &= ~(0x0f << (4 * pin));
	*afr |= (af << (4 * pin));
}
