#include "interrupt_in.h"
#include "digital_inout.h"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

void __gpio_set_exti(uint8_t port, uint8_t pin) {
	//this also eanbles the GPIO clock
	__gpio_set_input(port, pin);

	//enable syscfg clock
	RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;

	//configure EXTI mapping
	uint8_t exticrx = pin / 4;
	uint8_t exticrp = pin % 4;
	SYSCFG->EXTICR[exticrx] &= ~(0xf << (4 * exticrp));
	SYSCFG->EXTICR[exticrx] |= port << (4 * exticrp);
}
