// Dylan Dang & Hoang Nguyen
// Runs on TM4C123, 
// Dr. Min He
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

#include <stdint.h>

// Constants
#define PIANO     0  // piano mode: press a key to play a tone
#define AUTO_PLAY 1  // auto play mode: automatic playing a song

//---------------------Switch_Init---------------------
// initialize switch interface
// Input: none
// Output: none 
void ButtonLed_Init(void);

//---------------------PianoKeys_Init---------------------
// initialize onboard Piano keys interface: PORT D 0 - 3 are used to generate 
// tones: CDEF:doe ray mi fa
// No need to unlock. Only PD7 needs to be unlocked.
// Input: none
// Output: none 
void PianoKeys_Init(void);

uint8_t noteC_playing(void);

uint8_t noteD_playing(void);

uint8_t noteE_playing(void);

uint8_t noteF_playing(void);

uint8_t get_current_mode(void);