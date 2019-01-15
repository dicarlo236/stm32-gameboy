/*
 * ntsc.h
 *
 *  Created on: Jan 13, 2019
 *      Author: jared
 */

#ifndef WORMBOY_NTSC_H_
#define WORMBOY_NTSC_H_

#include "types.h"
#include <stdint.h>


void draw_vincent_string(char* str);
void new_line();
void clear_all_text();
void draw_gfx_line(float x0, float y0, float x1, float y1);
void init_ntsc();
void vid();
extern volatile uint8_t drawing;
// Buffer sizes
#define V_RES 144
#define H_RES (279 - 80 + 20 - 4)

// Porches
#define V_PORCH_SIZE 30
#define H_PORCH_SIZE 25

// good new stuff
#define X0s 50  // start of image in X
#define Y0s 0  // start of image in Y
#define XL (224 - 80 + 20 - 4) // 25 chars
#define YL 144 // 20 chars

//video
#define VIDEO_FRAME_SIZE (XL * YL / 2)

//*SD card*/
#define DI PC_3
#define DO PC_2
#define SCK PB_10
#define CS PB_12

extern uint8_t *im_line_vas[];
extern volatile uint8_t bufferSelect;
extern uint32_t tics;

#define set_pixel(x, y, color) im_line_va[H_RES*((y)+Y0) + (x) + X0] = (color)

#endif /* WORMBOY_NTSC_H_ */
