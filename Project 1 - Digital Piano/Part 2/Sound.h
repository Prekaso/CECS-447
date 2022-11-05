// Dylan Dang & Hoang Nguyen
// Sound.h, 
// This is the starter file for CECS 447 Project 1 Part 2
// By Dr. Min He
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

#include <stdint.h>

// define music note data structure 
struct Note {
  uint32_t tone_index;
  uint8_t delay;
};
typedef const struct Note NTyp;

// Constants


// **************DAC_Init*********************
// Initialize 3-bit DAC 
// Input: none
// Output: none
void DAC_Init(void);

// **************Sound_Start*********************
// Set reload value and enable systick timer
// Input: time duration to be generated in number of machine cycles
// Output: none
void Sound_Start(uint32_t period);
void Sound_stop(void);


void Delay(void);

void SysTick_Init(void);

void SysTick_start(void);

void SysTick_stop(void);

void SysTick_Set_Current_Note(unsigned long n_value);

void turn_off_music(void);

void turn_on_music(void);

uint8_t is_music_on(void);

uint8_t in_piano_mode(void);

void set_unset_piano_mode(void);

void play_a_note();

void next_octave(void);

uint8_t toneI(void);

// play the current song once
void play_a_song(void);

// Move to the next song
void next_song(void);
