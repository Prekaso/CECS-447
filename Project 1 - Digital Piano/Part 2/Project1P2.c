// This is the starter file for CECS 447 Project 1 Part 2
// By Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

// Header files 
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Sound.h"
#include "ButtonLed.h"

// 2. Declarations Section

// Function Prototypes
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);

  
// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){
  DisableInterrupts();    
  DAC_Init();        // Port B 
	SysTick_Init();		 // SysTick
	ButtonLed_Init();  // Port F
  PianoKeys_Init();  // Port D
	EnableInterrupts();
	
	while(1){
		if(in_piano_mode()){
			WaitForInterrupt();
			play_a_note();
		}
		else{
			play_a_song();
		}
	}
}


