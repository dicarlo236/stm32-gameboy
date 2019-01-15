/*
 * sound.h
 *
 *  Created on: Jan 13, 2019
 *      Author: jared
 */

#ifndef WORMBOY_SOUND_H_
#define WORMBOY_SOUND_H_

#include "types.h"

struct SqOsc {
	u32 per;
	//u32 duty;
	u32 amp;
	u32 cnt;
};


void snd();
void run_sound(u32 cpuCycles);

void ch1_trigger_load();
void ch1_length_load();
void ch1_update_freq();

void ch2_trigger_load();
void ch2_length_load();
void ch2_update_freq();

void ch3_trigger_load();
void ch3_length_load();
void ch3_update_freq();

void ch4_trigger_load();
void ch4_length_load();
void ch4_update_freq();

#endif /* WORMBOY_SOUND_H_ */
