/*
 * ntsc.cpp
 *
 *  Created on: Jan 13, 2019
 *      Author: jared
 */

#include "ntsc.h"
#include "types.h"
#include "vincent_data.h"
#include "mbad.h"
#include "sound.h"
#include "stm32h7xx_nucleo_144.h"
#include <math.h>
#include <string.h>

#define TEXT_LEVEL 5

#define TX 24  // number of characters in X
#define TY 10  // number of characters in Y

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define DAC_SYNC 2

uint8_t char_col = 0;  // current column counter
uint8_t char_row = 0;  // current row counter

uint8_t* im_line_va_1;
uint8_t bl_line_s[H_RES]; //blank line sync buffer
uint8_t bl_line_v[H_RES]; //blank line video buffer
uint8_t vb_line_s[H_RES]; //vertical sync, sync buffer
uint8_t vb_line_v[H_RES]; //vertical sync, video buffer
uint8_t im_line_s[H_RES]; //image sync buffer

//buffers
uint8_t current_frame[YL][XL / 2];

volatile u8 bufferSelect = 0;
uint8_t im_line_va_2[H_RES*V_RES];

//double buffered drawing
u8* im_line_vas[2];

uint16_t l=0; //current line of scan
uint32_t tics = 0; //timer

// positive or negative?
int16_t sign(int16_t a)
{
    if(a > 0) return 1;
    if(a < 0) return -1;
    return 0;
}

// clear the screen and the text buffer
void clr()
{
    for(int i = 0; i < H_RES; i++)
        for(int j = 0; j < V_RES; j++)
            im_line_vas[bufferSelect][i+j*H_RES] = 0;
}

// initialize video buffers
void init_buffers()
{
    clr(); // zero buffers
    for(int i = 0; i < H_RES; i++)
    {
        im_line_s[i] = DAC_SYNC;
        bl_line_s[i] = DAC_SYNC;
        bl_line_v[i] = 0;
        vb_line_s[i] = 0;
        vb_line_v[i] = 0;
    }
    im_line_s[0] = 0;
    im_line_s[1] = 0;
    im_line_s[2] = 0;
    im_line_s[3] = 0;

    for(int i = 0; i < 15; i++) im_line_s[i] = 0;

    bl_line_s[0] = 0;
    vb_line_s[0] = DAC_SYNC;
    bl_line_s[1] = 0;
    vb_line_s[1] = DAC_SYNC;

    bl_line_s[3] = 0;
    vb_line_s[3] = DAC_SYNC;
    bl_line_s[2] = 0;
    vb_line_s[2] = DAC_SYNC;
}

volatile uint8_t drawing = 0;
// video interrupt

//extern u32 frameCount;
u32 frameCountNTSC = 0;
void vid() {
    uint8_t nop = 0, img = 0; //use nops or use wait_us
    uint8_t* sptr; //pointer to sync buffer for line
    uint8_t* vptr; //pointer to video buffer for line
    uint8_t* pptr; //porch

   // drawing = (l > 30 && l < (30 + YL));

    //if(l > 180) {
//        drawing = 1;
//    }
    if (l < V_PORCH_SIZE) {

        vptr = bl_line_v;
        sptr = im_line_s;
        nop = 1;
    }
    else if ((l < YL + V_PORCH_SIZE)) {
        //drawing = l - V_PORCH_SIZE + 1;
        //drawing = 1;
        vptr = im_line_vas[bufferSelect] + (l - 30) * H_RES;
        sptr = im_line_s;
        nop = 1;
        img = 1;
    }
    else if (l < 255) {
        //drawing = 0;
        vptr = bl_line_v;
        sptr = bl_line_s;
        nop = 0;
    }
    else {
    	frameCountNTSC++;
        vptr = vb_line_v;
        sptr = vb_line_s;
        nop = 1;
    }


    pptr = img ? bl_line_v : vptr;

    if (nop) {
        for (uint16_t i = 0; i < H_PORCH_SIZE; i++) {
        	__gpio_set_port(PD,(pptr[i] + sptr[i]) << 4);

            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");

            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                       asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                       asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                                              asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                                              asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                                              asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                                              asm("nop");asm("nop");asm("nop");asm("nop");
        }

        for (uint16_t i = 0; i < H_RES; i++) //loop over each column
        {
        	__gpio_set_port(PD,(vptr[i] + sptr[i]) << 4);

            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
                                    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        }
    } else {
        for(uint16_t i = 0; i < 12; i++) //loop over each column
        {
        	__gpio_set_port(PD,sptr[i] << 4);

            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");

            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");

            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");

            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
            asm("nop");asm("nop");asm("nop");asm("nop");
        }
    }

    //move to next line
    l++;
    tics++;
    if(l > 255) {l = 0;}

    snd();
}

// draw letter
void draw_vincent(uint16_t x0, uint16_t y0, uint8_t c)
{
    if(c == 40) c = 41;
    else if(c == 41) c = 40;
    char* letter = vincent_data[c];
    for(uint16_t xp = 0; xp < 8; xp++)
    {
        for(uint16_t yp = 0; yp < 8; yp++)
        {
            im_line_vas[bufferSelect][H_RES*(yp+y0) + xp + x0] = CHECK_BIT(letter[yp],8-xp)?TEXT_LEVEL:0;
        }
    }
}

// draw string
void draw_vincent_string(char* str)
{
    for(int i = 0; i < strlen(str); i++)
    {
        if(str[i] == 0) return;
        char_col++;
    if(char_col >= TX)
    {
        char_col = 0;
        char_row++;
    }
    if(char_row >= TY)
    {
        char_row = 0;
    }

    draw_vincent(X0s +2 + 8*char_col, Y0s + 14 + 8*char_row,
        str[i]);
    }
}

void set_status_string(char* str)
{
    uint8_t char_col_backup = char_col;
    uint8_t char_row_backup = char_row;
    char_col = 0;
    char_row = TY - 1;

    draw_vincent_string(str);

    char_col = char_col_backup;
    char_row = char_row_backup;
}

void init_ntsc() {
	im_line_vas[0] = im_line_va_2;
	im_line_vas[1] = im_line_va_1;
	printf("[NTSC] Init...\r\n");
	init_buffers();

	    for(int x = 0; x < XL; x++) {
	        for(int y = 0; y < YL; y++) {
	        	im_line_vas[bufferSelect][H_RES*(y+Y0s) + x + X0s] = 6 * (((x+y)>>1)&1);
	        }
	    }

	printf("[NTSC] Init done!\r\n");
}



