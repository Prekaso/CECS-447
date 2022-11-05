// SysTick.c
// Runs on TM4C123. 
// It supports the following example music programs: BasicTones.c, HappyBirthday.c, and SingASong.c.
// Authors: Min He
// Date: August 28, 2018

#include "tm4c123gh6pm.h"

#define SPEAKER (*((volatile unsigned long *)0x40004020)) // define SPEAKER connects to PA3
#define Speaker_Toggle_Mask     0x00000008  // mask used to toggle the speaker output


// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC+NVIC_ST_CTRL_INTEN;
}

void SysTick_start(void)
{
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;
}
void SysTick_stop(void)
{
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
}

void SysTick_Set_Current_Note(unsigned long n_value)
{
  NVIC_ST_RELOAD_R = n_value-1;// update reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
}

// Interrupt service routine, 
// frequency is determined by current tone being played
void SysTick_Handler(void){
	NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE; //clr bit 0
	SPEAKER ^= Speaker_Toggle_Mask; // inverse bit 2	
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;  // set bit 0
}

