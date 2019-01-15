#include "mbad.h"
#include "wormboy/ntsc.h"
#include "wormboy/cpu.h"
#include "wormboy/mem.h"
#include "wormboy/video.h"
#include "stm32h7xx_nucleo_144.h"
//#include "wormboy/tetris.h"
#include "wormboy/pkmn_red.h"
#include "wormboy/sound.h"

int led_on = 1;

void toggle_led() {
	if (led_on) BSP_LED_On(LED2);
	else BSP_LED_Off(LED2);
	led_on = !led_on;
}

void update_inputs() {
	keyboard.a = !digitalRead(PC, 6);
	keyboard.b = !digitalRead(PB, 15);
	keyboard.start = !digitalRead(PB, 13);
	keyboard.select = !digitalRead(PB, 12);
	keyboard.r = !digitalRead(PA, 15);
	keyboard.d = !digitalRead(PC, 7);
	keyboard.l = !digitalRead(PB, 5);
	keyboard.u = !digitalRead(PB, 3);
}

int main(void)
{
	// initialize MBAD
	System_Setup();
	Init_USB_Serial(115200);

	// debuggin prints
	printf("derp!\r\n");
	printf("System core clock : %f MHz\r\n", (double)SystemCoreClock / 1e6);

	// initialize pins with MBAD
	BSP_LED_Init(LED2); // flashy light on the board
	digitalOut(PD, 7);  // video output
	digitalOut(PD, 6);
	digitalOut(PD, 5);
	digitalOut(PD, 4);
	digitalOut(PD, 3);
	digitalOut(PD, 2);
	digitalOut(PD, 1);
	digitalOut(PD, 0);
	digitalOut(PA, 3);
	digitalOut(PC, 0);

	digitalIn(PC, 6, 1);    // controller
	digitalIn(PB, 15, 1);
	digitalIn(PB, 13, 1);
	digitalIn(PB, 12, 1);
	digitalIn(PA, 15, 1);
	digitalIn(PC, 7, 1);
	digitalIn(PB, 5, 1);
	digitalIn(PB, 3, 1);

	digitalOut(PC, 8); // sound

	// initialize video out
	init_ntsc();

	printf("[main] start vid\r\n");
	// set up video interrupt
	tickerStart_usecs(TIMER5, 64);
	tickerAttach(TIMER5, vid);

//	tickerStart_usecs(TIMER2, 20);
//	tickerAttach(TIMER2, snd);

    printf("[main] screen %d x %d (buffer %d x %d, %d bytes)\n",
           XL,YL,H_RES,V_RES,H_RES*V_RES);

    printf("[main] load rom\r\n");
    FileLoadData rom;
    rom.size = 0x10000;
    rom.data = (u8*)(void*)pkmn_red;

    printf("[main] START WORM BOY\r\n");
    initMem(rom);
    initVideo(nullptr);

    __gpio_set_analog(PA, 4);

    __HAL_RCC_DAC12_CLK_ENABLE(); // enable DAC clock
    DAC1->CR = 1;
    DAC1->DHR12R1 = (1 << 11);

	for(;;) {

		update_inputs();
		u32 soundCycleCounter = 0;

		for(int i = 0; i < 100; i++) {
		    u32 cpuCycles = cpuStep();
		    stepVideo(cpuCycles);
		    soundCycleCounter += cpuCycles;
		}
		run_sound(soundCycleCounter);
	}
}
