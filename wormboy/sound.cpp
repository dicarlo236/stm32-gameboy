/*
 * sound.cpp
 *
 *  Created on: Jan 13, 2019
 *      Author: jared
 */

#include "sound.h"
#include "types.h"
#include "mem.h"
#include "mbad.h"
#include "stm32h7xx_nucleo_144.h"

#include <stdlib.h>


#define SND_PERIOD 64 // microseconds

struct Ch1SoundState {
	u8 vol;

	u8 sweepShadow;
	u8 sweepEnable;

	u8 len;
	u8 lenCntEnable;

	u8 enable;
	u32 gbFreq;
	u32 per;
};

struct Ch2SoundState {
	u8 vol;

	u8 len;
	u8 lenCntEnable;

	u8 enable;
	u32 gbFreq;
	u32 per;
};

struct Ch3SoundState {
	u8 vol;

	u32 len;
	u8 lenCntEnable;

	u8 enable;
	u32 gbFreq;
	u32 per;
};

struct Ch4SoundState {
	u8 vol;
	u32 len;
	u8 lenCntEnable;
	u8 enable;
};

struct SoundState {
	u8 masterEnable;
	Ch1SoundState ch1;
	Ch2SoundState ch2;
	Ch3SoundState ch3;
	Ch4SoundState ch4;
};

SoundState sndState;

u32 sndCount = 0;
u32 c1c = 0, c2c = 0, c3c = 0;
void snd() {
	u8 sndOut = 0;
	sndCount++;
	if(!sndState.masterEnable) return;

	if(sndState.ch1.enable && sndState.ch1.vol) {
		c1c++;
		u32 progress = sndCount % sndState.ch1.per;
		if(progress > sndState.ch1.per / 2) {
			sndOut += sndState.ch1.vol;
		}
		if(c1c > sndState.ch1.per) c1c = 0;
	}

	if(sndState.ch2.enable && sndState.ch2.vol) {
		c2c++;
		u32 progress = sndCount % sndState.ch2.per;
		if(progress > sndState.ch2.per / 2) {
			sndOut += sndState.ch2.vol;
		}
		if(c2c > sndState.ch2.per) c2c = 0;
	}

	if(sndState.ch3.enable && sndState.ch3.vol) {
		c3c++;
		u32 progress = sndCount % sndState.ch3.per;
		if(progress > sndState.ch3.per / 2) {
			sndOut += sndState.ch3.vol;
		}
		if(c3c > sndState.ch3.per) c3c = 0;
	}

	if(sndState.ch4.enable && sndState.ch4.vol) {
			sndOut += sndState.ch4.vol * (rand() & 1);
	}

	DAC1->DHR12R1 = (sndOut << 6);
	//digitalWrite(PC, 8, sndOut);
}


void init_sound() {
	printf("[sound] init\r\n");
	sndState.masterEnable = 0;

	sndState.ch1.vol = 0;
	sndState.ch1.sweepShadow = 0;
	sndState.ch1.sweepEnable = 0;
	sndState.ch1.len = 0;
	sndState.ch1.lenCntEnable = 0;
	sndState.ch1.enable = 0;
	sndState.ch1.per = 0;
	sndState.ch1.gbFreq = 0;

	sndState.ch2.vol = 0;
	sndState.ch2.len = 0;
	sndState.ch2.lenCntEnable = 0;
	sndState.ch2.enable = 0;
	sndState.ch2.per = 0;
	sndState.ch2.gbFreq = 0;

	sndState.ch3.vol = 0;
	sndState.ch3.len = 0;
	sndState.ch3.lenCntEnable = 0;
	sndState.ch3.enable = 0;
	sndState.ch3.per = 0;
	sndState.ch3.gbFreq = 0;

	sndState.ch4.vol = 0;
	sndState.ch4.len = 0;
	sndState.ch4.lenCntEnable = 0;
	sndState.ch4.enable = 0;
}



void ch4_length_load() {
	// todo: currently ignoring duty cycle
	// first check the mode:
	u8 nr44 = globalMemState.ioRegs[IO_NR44];
	u8 lenEnable = (nr44 & 0x40) != 0;
	sndState.ch4.lenCntEnable = lenEnable;

	if(lenEnable) {
		//printf("length load\r\n");
		u8 nr41 = globalMemState.ioRegs[IO_NR41];
		sndState.ch4.len = nr41 & 0x3f;
		sndState.ch4.enable = 1;
	} else {
		//sndState.ch2.enable = 1; // not 100% sure about this...
	}
}

void ch4_trigger_load() {
	u8 nr44 = globalMemState.ioRegs[IO_NR44];
	u8 nr42 = globalMemState.ioRegs[IO_NR42];

	u8 lenEnable = (nr44 & 0x40) != 0;
	sndState.ch4.lenCntEnable = lenEnable;
	//printf("trigger: 0x%x\r\n", nr14);
	if(nr44 & 0x80) {
		// trigger!
		u8 startingVol = nr42 >> 4;
		sndState.ch4.enable = 1;
		sndState.ch4.vol = startingVol;
		u8 nr41 = globalMemState.ioRegs[IO_NR41];
		sndState.ch4.len = (64 - (nr41 & 0x3f));
	}
}

void ch4_len_update() {
	if(sndState.ch4.lenCntEnable && sndState.ch4.len) {
		//printf("len: %d\r\n", sndState.ch1.len);
		sndState.ch4.len--;
	}

	if(sndState.ch4.len == 0) {
		sndState.ch4.lenCntEnable = 0;
		sndState.ch4.enable = 0;
	}

}

u8 ch4_env_subcount = 0;
void ch4_env_update() {
	u8 nr42 = globalMemState.ioRegs[IO_NR42];

	ch4_env_subcount++;
	u8 dir = (nr42 & 8) != 0;
	u8 sbc = nr42 & 7;
	if(ch4_env_subcount > sbc) {
		ch4_env_subcount = 0;
		if(dir && sndState.ch4.vol < 15) {
			sndState.ch4.vol++;
		} else if(sndState.ch4.vol > 0){
			sndState.ch4.vol--;
		}
	}
}

//////////
// ch3
//////////

// update the sndState with frequency from the NR13/NR14 register
void ch3_update_freq() {
	u8 nr33 = globalMemState.ioRegs[IO_NR33];
	u8 nr34 = globalMemState.ioRegs[IO_NR34];

	// compute the "gb freq"
	u16 gbFreq = (((u16)nr34 & 0x7) << 8) + (u16)nr33;
	sndState.ch3.gbFreq = gbFreq;

	// compute frequency in Hz (todo don't use floats)
	float gbf = gbFreq;
	float freqHz = 131072.f / (2048.f - gbf);
	//printf("freq %f\r\n", freqHz);
	// compute period in timer ticks
	float period = 2.f*15625.f / freqHz;
	sndState.ch3.per = period;
}

void ch3_length_load() {
	ch3_update_freq();
	// todo: currently ignoring duty cycle
	// first check the mode:
	u8 nr34 = globalMemState.ioRegs[IO_NR34];
	u8 lenEnable = (nr34 & 0x40) != 0;
	sndState.ch3.lenCntEnable = lenEnable;

	if(lenEnable) {
		//printf("length load\r\n");
		u8 nr31 = globalMemState.ioRegs[IO_NR31];
		sndState.ch3.len = nr31 & 0x3f;
		sndState.ch3.enable = 1;
	} else {
		//sndState.ch2.enable = 1; // not 100% sure about this...
	}
}

void ch3_trigger_load() {
	u8 nr34 = globalMemState.ioRegs[IO_NR34];
	u8 nr32 = globalMemState.ioRegs[IO_NR32];
	ch3_update_freq();
	u8 lenEnable = (nr34 & 0x40) != 0;
	sndState.ch3.lenCntEnable = lenEnable;
	//printf("trigger: 0x%x\r\n", nr14);
	if(nr34 & 0x80) {
		// trigger!
		u8 startingVol = nr32 >> 5;
		sndState.ch3.enable = 1;
		sndState.ch3.vol = (startingVol & 0x3) << 3;
		u8 nr31 = globalMemState.ioRegs[IO_NR31];
		sndState.ch3.len = ((u16)256 - (nr31));
	}
}

void ch3_len_update() {
	if(sndState.ch3.lenCntEnable && sndState.ch3.len) {
		//printf("len: %d\r\n", sndState.ch1.len);
		sndState.ch3.len--;
	}

	if(sndState.ch3.len == 0) {
		sndState.ch3.lenCntEnable = 0;
		sndState.ch3.enable = 0;
	}

	u8 nr32 = globalMemState.ioRegs[IO_NR32];
	ch3_update_freq();
	u8 startingVol = nr32 >> 5;
	sndState.ch3.vol = (startingVol & 0x3) << 3;

}

//////////
// snd2
/////////

// update the sndState with frequency from the NR13/NR14 register
void ch2_update_freq() {
	u8 nr23 = globalMemState.ioRegs[IO_NR23];
	u8 nr24 = globalMemState.ioRegs[IO_NR24];

	// compute the "gb freq"
	u16 gbFreq = (((u16)nr24 & 0x7) << 8) + (u16)nr23;
	sndState.ch2.gbFreq = gbFreq;

	// compute frequency in Hz (todo don't use floats)
	float gbf = gbFreq;
	float freqHz = 131072.f / (2048.f - gbf);
	//printf("freq %f\r\n", freqHz);
	// compute period in timer ticks
	float period = 2.f*15625.f / freqHz;
	sndState.ch2.per = period;
}

void ch2_length_load() {
	ch2_update_freq();
	// todo: currently ignoring duty cycle
	// first check the mode:
	u8 nr24 = globalMemState.ioRegs[IO_NR24];
	u8 lenEnable = (nr24 & 0x40) != 0;
	sndState.ch2.lenCntEnable = lenEnable;

	if(lenEnable) {
		//printf("length load\r\n");
		u8 nr21 = globalMemState.ioRegs[IO_NR21];
		sndState.ch2.len = nr21 & 0x3f;
		sndState.ch2.enable = 1;
	} else {
		//sndState.ch2.enable = 1; // not 100% sure about this...
	}
}

void ch2_trigger_load() {
	u8 nr24 = globalMemState.ioRegs[IO_NR24];
	u8 nr22 = globalMemState.ioRegs[IO_NR22];
	ch2_update_freq();
	u8 lenEnable = (nr24 & 0x40) != 0;
	sndState.ch2.lenCntEnable = lenEnable;
	//printf("trigger: 0x%x\r\n", nr14);
	if(nr24 & 0x80) {
		// trigger!
		u8 startingVol = nr22 >> 4;
		sndState.ch2.enable = 1;
		sndState.ch2.vol = startingVol;
		u8 nr21 = globalMemState.ioRegs[IO_NR21];
		sndState.ch2.len = (64 - (nr21 & 0x3f));
	}
}

void ch2_len_update() {
	if(sndState.ch2.lenCntEnable && sndState.ch2.len) {
		//printf("len: %d\r\n", sndState.ch1.len);
		sndState.ch2.len--;
	}

	if(sndState.ch2.len == 0) {
		sndState.ch2.lenCntEnable = 0;
		sndState.ch2.enable = 0;
	}

}

u8 ch2_env_subcount = 0;
void ch2_env_update() {
	u8 nr22 = globalMemState.ioRegs[IO_NR22];

	ch2_env_subcount++;
	u8 dir = (nr22 & 8) != 0;
	u8 sbc = nr22 & 7;
	if(ch2_env_subcount > sbc) {
		ch2_env_subcount = 0;
		if(dir && sndState.ch2.vol < 15) {
			sndState.ch2.vol++;
		} else if(sndState.ch2.vol > 0){
			sndState.ch2.vol--;
		}
	}
}

///////////////
// channel 1
///////////////

// update the sndState with frequency from the NR13/NR14 register
void ch1_update_freq() {
	u8 nr13 = globalMemState.ioRegs[IO_NR13];
	u8 nr14 = globalMemState.ioRegs[IO_NR14];

	// compute the "gb freq"
	u16 gbFreq = (((u16)nr14 & 0x7) << 8) + (u16)nr13;
	sndState.ch1.gbFreq = gbFreq;

	// compute frequency in Hz (todo don't use floats)
	float gbf = gbFreq;
	float freqHz = 131072.f / (2048.f - gbf);
	//printf("freq %f\r\n", freqHz);
	// compute period in timer ticks
	float period = 2.f*15625.f / freqHz;
	sndState.ch1.per = period;
}

void ch1_length_load() {
	ch1_update_freq();
	// todo: currently ignoring duty cycle
	// first check the mode:
	u8 nr14 = globalMemState.ioRegs[IO_NR14];
	u8 lenEnable = (nr14 & 0x40) != 0;
	sndState.ch1.lenCntEnable = lenEnable;

	if(lenEnable) {
		//printf("length load\r\n");
		u8 nr11 = globalMemState.ioRegs[IO_NR11];
		sndState.ch1.len = nr11 & 0x3f;
		sndState.ch1.enable = 1;
	} else {
		//sndState.ch1.enable = 1; // not 100% sure about this...
	}
}

void ch1_trigger_load() {
	u8 nr14 = globalMemState.ioRegs[IO_NR14];
	u8 nr12 = globalMemState.ioRegs[IO_NR12];
	ch1_update_freq();
	u8 lenEnable = (nr14 & 0x40) != 0;
	sndState.ch1.lenCntEnable = lenEnable;
	//printf("trigger: 0x%x\r\n", nr14);
	if(nr14 & 0x80) {
		// trigger!
		u8 startingVol = nr12 >> 4;
		sndState.ch1.enable = 1;
		sndState.ch1.vol = startingVol;
		u8 nr11 = globalMemState.ioRegs[IO_NR11];
		sndState.ch1.len = (64 - (nr11 & 0x3f));
	}
}

void ch1_len_update() {
	if(sndState.ch1.lenCntEnable && sndState.ch1.len) {
		//printf("len: %d\r\n", sndState.ch1.len);
		sndState.ch1.len--;
	}

	if(sndState.ch1.len == 0) {
		sndState.ch1.lenCntEnable = 0;
		sndState.ch1.enable = 0;
	}

}

u8 ch1_env_subcount = 0;
void ch1_env_update() {
	u8 nr12 = globalMemState.ioRegs[IO_NR12];

	ch1_env_subcount++;
	u8 dir = (nr12 & 8) != 0;
	u8 sbc = nr12 & 7;
	if(ch1_env_subcount > sbc) {
		ch1_env_subcount = 0;
		if(dir && sndState.ch1.vol < 15) {
			sndState.ch1.vol++;
		} else if(sndState.ch1.vol > 0){
			sndState.ch1.vol--;
		}
	}
}

void ch1_sweep_update() {

}


u32 sndCpuCycleCount = 0;
u32 sndIterCnt = 0;
void run_sound(u32 cpuCycles) {
	// read main sound registers
	u8 nr50 = globalMemState.ioRegs[IO_NR50];
	u8 nr51 = globalMemState.ioRegs[IO_NR51];
	u8 nr52 = globalMemState.ioRegs[IO_NR52];

	u8 nr30 = globalMemState.ioRegs[IO_NR30];

	// unpack
	u8 so2Vol = (nr50 & 0x70) >> 4;
	u8 so1Vol = (nr50 & 0x7);

	u8 ch1So1 = (nr51 & 0x01) != 0;
	u8 ch2So1 = (nr51 & 0x02) != 0;
	u8 ch3So1 = (nr51 & 0x04) != 0;
	u8 ch4So1 = (nr51 & 0x08) != 0;
	u8 ch1So2 = (nr51 & 0x10) != 0;
	u8 ch2So2 = (nr51 & 0x20) != 0;
	u8 ch3So2 = (nr51 & 0x40) != 0;
	u8 ch4So2 = (nr51 & 0x80) != 0;

	u8 ch1Master = (nr52 & 0x01) != 0;
	u8 ch2Master = (nr52 & 0x02) != 0;
	u8 ch3Master = (nr52 & 0x04) != 0;
	u8 ch4Master = (nr52 & 0x08) != 0;
	u8 sndMaster = (nr52 & 0x80) != 0;
	//ch1_update_freq();
	if(sndMaster) {
		// do the thing
		sndState.masterEnable = 1;
		sndCpuCycleCount += cpuCycles;
		if(sndCpuCycleCount >= (1 << 14)) {
			sndCpuCycleCount = 0;
			//printf("sdupdate\r\n");
			sndIterCnt++;
			ch1_len_update();
			ch2_len_update();
			ch3_len_update();
			ch4_len_update();
			if((sndIterCnt & 0x1) == 0) {
				ch1_sweep_update();
				//ch2_sweep_update();
			}
			if((sndIterCnt & 0x3) == 0) {
				ch1_env_update();
				ch2_env_update();
				ch4_env_update();
			}
		}
		if(!(ch1So1 || ch1So2)) {
			sndState.ch1.enable = 0;
		}

		if(!((ch2So1 || ch2So2))) {
			sndState.ch2.enable = 0;
		}

		if(!((ch3So1 || ch3So2))) {
			sndState.ch3.enable = 0;
		}

		if(!((ch4So1 || ch4So2))) {
			sndState.ch4.enable = 0;
		}

		if(!(nr30 & 0x80)) {
			sndState.ch3.enable = 0;
		}
	} else {
		sndState.masterEnable = 0;
	}
}

