#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "Draw_Man.h"
#include "ST7735.h"

/**************Man***************
 Function the draw the man
 Inputs:  x   x co-ordinate of the figure
          y   y co-ordinate of the figure
          h   height of the figure
          color:   color to be given 
 Outputs: none
*/




void Man(int32_t x, int32_t y,int32_t h, uint16_t color,uint16_t color1,uint16_t color2){
		ST7735_FillCircle(x , y, 10,color);							//face
		ST7735_FillRect(x-15, y+11,30, 5, color1);      //shoulder
		ST7735_FillRect(x-8, y+16,16, 20, color1);      //body
		ST7735_FillRect(x-8, y+36,16, 20, color2);      //leg




}