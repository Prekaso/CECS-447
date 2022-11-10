// Project 4: ST7735R Color LCD
// By: Dylan Dang & Hoang Nguyen

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected 
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO)
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "ST7735.h"
#include "PLL.h"
#include "Draw_Man.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"

int main(void) {
	uint32_t x = 40, ht = 0;	
	uint32_t Lazerx = 30, Lazery = 120,Lazerh=10,ManX=17,ManY=99;	
	int32_t l = 15, h = 15;
	int i = 0;
  PLL_Init(12);
  ST7735_InitR(INITR_REDTAB);
	SysTick_Init();
	
	// draw ground
	ST7735_FillScreen(ST7735_CYAN); // background color
	
	// draw staircase
	ST7735_FillRect(0,150,127,40,ST7735_BLACK);
	ST7735_FillRect(37, 129,30, 30, ST7735_MAGENTA);
	ST7735_FillRect(67, 99,30, 60, ST7735_RED);
	ST7735_FillRect(97, 69,38, 90, ST7735_GREEN);
	
	Man(17,99,0,ST7735_YELLOW,ST7735_RED,ST7735_BLUE);
	
	// "JUMP UP!" in the sky
	ST7735_DrawChar(1,0,'J',ST7735_RED,ST7735_WHITE,2); // (x-pos, y-pos, char, font color, bg color, font size)
	ST7735_DrawChar(13,0,'U',ST7735_MAGENTA,ST7735_WHITE,2);
	ST7735_DrawChar(26,0,'M',ST7735_RED,ST7735_WHITE,2);
	ST7735_DrawChar(40,0,'P',ST7735_BLUE,ST7735_WHITE,2);
	ST7735_DrawChar(1,20,'U',ST7735_MAGENTA,ST7735_WHITE,4);
	ST7735_DrawChar(25,20,'P',ST7735_BLUE,ST7735_WHITE,4);
	ST7735_DrawChar(45,20,'!',ST7735_GREEN,ST7735_WHITE,3);
	
	// draw sun
	ST7735_FillCircle(110,45,10,ST7735_YELLOW);
	
	
	while(1) {
		while(i < 3) { // draw arrow pointing to top right
			SysTick_Wait10ms(50);
			ST7735_DrawLine(Lazerx,Lazery,Lazerx+Lazerh,Lazery-Lazerh,ST7735_BLACK);
			i = i + 1;
			Lazerh = Lazerh + 25;		
		}

		i = 0;
		Lazerh = 30;

		ST7735_DrawLine(90,75,90,60,ST7735_BLACK); // right vertical arrow line
		ST7735_DrawLine(75,60,90,60,ST7735_BLACK); // left horizontal arrow line
		
		while(i < 5) { // jump up
			SysTick_Wait10ms(5);
			Man(ManX,ManY,0,ST7735_CYAN,ST7735_CYAN,ST7735_CYAN); // clear the previous position
			ManY = ManY -5;
			Man(ManX,ManY,0,ST7735_YELLOW,ST7735_RED,ST7735_BLUE);
			i = i + 1;		
		}

		i = 0;
		
		while(i < 5) { // jump down
			SysTick_Wait10ms(5);
			Man(ManX,ManY,0,ST7735_CYAN,ST7735_CYAN,ST7735_CYAN); // clear the previous position
			ManY = ManY +5;
			Man(ManX,ManY,0,ST7735_YELLOW,ST7735_RED,ST7735_BLUE);
			i = i + 1;		
		}
	}
}
