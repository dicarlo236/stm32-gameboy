#include "setup.h"
#include "digital_inout.h"

volatile uint32_t * __gpiox_moder[11] =     {
										     &(GPIOA->MODER), &(GPIOB->MODER), &(GPIOC->MODER), &(GPIOD->MODER),
										     &(GPIOE->MODER), &(GPIOF->MODER), &(GPIOG->MODER), &(GPIOH->MODER),
										     &(GPIOI->MODER), &(GPIOJ->MODER), &(GPIOK->MODER)
										    };

volatile uint32_t * __gpiox_otyper[11] =   {
										    &(GPIOA->OTYPER), &(GPIOB->OTYPER), &(GPIOC->OTYPER), &(GPIOD->OTYPER),
										    &(GPIOE->OTYPER), &(GPIOF->OTYPER), &(GPIOG->OTYPER), &(GPIOH->OTYPER),
										    &(GPIOI->OTYPER), &(GPIOJ->OTYPER), &(GPIOK->OTYPER)
										   };

volatile uint32_t * __gpiox_ospeedr[11] =  {
										    &(GPIOA->OSPEEDR), &(GPIOB->OSPEEDR), &(GPIOC->OSPEEDR), &(GPIOD->OSPEEDR),
										    &(GPIOE->OSPEEDR), &(GPIOF->OSPEEDR), &(GPIOG->OSPEEDR), &(GPIOH->OSPEEDR),
										    &(GPIOI->OSPEEDR), &(GPIOJ->OSPEEDR), &(GPIOK->OSPEEDR)
									       };

volatile uint32_t * __gpiox_pupdr[11] =    {
									        &(GPIOA->PUPDR), &(GPIOB->PUPDR), &(GPIOC->PUPDR), &(GPIOD->PUPDR),
									        &(GPIOE->PUPDR), &(GPIOF->PUPDR), &(GPIOG->PUPDR), &(GPIOH->PUPDR),
									        &(GPIOI->PUPDR), &(GPIOJ->PUPDR), &(GPIOK->PUPDR)
									       };

volatile uint32_t * __gpiox_idr[11] =      {
									        &(GPIOA->IDR), &(GPIOB->IDR), &(GPIOC->IDR), &(GPIOD->IDR),
									        &(GPIOE->IDR), &(GPIOF->IDR), &(GPIOG->IDR), &(GPIOH->IDR),
									        &(GPIOI->IDR), &(GPIOJ->IDR), &(GPIOK->IDR)
									       };

volatile uint32_t * __gpiox_odr[11] =      {
									        &(GPIOA->ODR), &(GPIOB->ODR), &(GPIOC->ODR), &(GPIOD->ODR),
									        &(GPIOE->ODR), &(GPIOF->ODR), &(GPIOG->ODR), &(GPIOH->ODR),
									        &(GPIOI->ODR), &(GPIOJ->ODR), &(GPIOK->ODR)
									       };

volatile uint32_t *__gpiox_afrl[11] =      {
											&(GPIOA->AFR[0]), &(GPIOB->AFR[0]), &(GPIOC->AFR[0]), &(GPIOD->AFR[0]),
											&(GPIOE->AFR[0]), &(GPIOF->AFR[0]), &(GPIOG->AFR[0]), &(GPIOH->AFR[0]),
											&(GPIOI->AFR[0]), &(GPIOJ->AFR[0]), &(GPIOK->AFR[0]),
										   };

volatile uint32_t *__gpiox_afrh[11] =      {
											&(GPIOA->AFR[1]), &(GPIOB->AFR[1]), &(GPIOC->AFR[1]), &(GPIOD->AFR[1]),
											&(GPIOE->AFR[1]), &(GPIOF->AFR[1]), &(GPIOG->AFR[1]), &(GPIOH->AFR[1]),
											&(GPIOI->AFR[1]), &(GPIOJ->AFR[1]), &(GPIOK->AFR[1]),
										   };

uint32_t __gpiox_rcc_en[11] = {RCC_AHB4ENR_GPIOAEN, RCC_AHB4ENR_GPIOBEN, RCC_AHB4ENR_GPIOCEN, RCC_AHB4ENR_GPIODEN,
									 RCC_AHB4ENR_GPIOEEN, RCC_AHB4ENR_GPIOFEN, RCC_AHB4ENR_GPIOGEN, RCC_AHB4ENR_GPIOHEN,
									 RCC_AHB4ENR_GPIOIEN, RCC_AHB4ENR_GPIOJEN, RCC_AHB4ENR_GPIOKEN,};

void __gpio_set_input(uint8_t port, uint8_t pin) {
	//gpio clock enable
	RCC->AHB4ENR |= __gpiox_rcc_en[port];

	//input
	*__gpiox_moder[port] &= ~(1 << (2 * pin));
	*__gpiox_moder[port] &= ~(1 << (2 * pin + 1));

	//hi-z
	*__gpiox_pupdr[port] &= ~(1 << (2 * pin));
	*__gpiox_pupdr[port] &= ~(1 << (2 * pin + 1));
}

void __gpio_set_output(uint8_t port, uint8_t pin) {
	//gpio clock enable
	RCC->AHB4ENR |= __gpiox_rcc_en[port];

	//general purpose output
	*__gpiox_moder[port] |= 1 << (2 * pin);
	*__gpiox_moder[port] &= ~(1 << (2 * pin + 1));

	//push-pull
	*__gpiox_otyper[port] &= ~(1 << pin);

	//very high speed
	*__gpiox_ospeedr[port] |= 1 << (2 * pin);
	*__gpiox_ospeedr[port] |= 1 << (2 * pin + 1);
}

void __gpio_set_pupd(uint8_t port, uint8_t pin, uint8_t value) {
	//clear bits 2 * pin, 2 * pin + 1
	*__gpiox_pupdr[port] &= ~(1 << (2 * pin));

	//set bits 2 * pin, 2 * pin + 1 to [value]
	*__gpiox_pupdr[port] |= (value << (2 * pin));
}

void digitalIn(uint8_t port, uint8_t pin, uint8_t pullup) {
	__gpio_set_input(port, pin);
	__gpio_set_pupd(port, pin, pullup);
}

void digitalOut(uint8_t port, uint8_t pin) {
	__gpio_set_output(port, pin);
}

uint8_t digitalRead(uint8_t port, uint8_t pin) {
	return __gpio_get(port, pin);
}

void digitalWrite(uint8_t port, uint8_t pin, uint8_t value) {
	__gpio_set(port, pin, value);
}
