// SysTick.h
// Runs on TM4C123
// It supports the following example music programs: BasicTones.c, HappyBirthday.c, and SingASong.c.
// Authors: Min He
// Date: August 28, 2018

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void);
void SysTick_start(void);
void SysTick_stop(void);
void SysTick_Set_Current_Note(unsigned long n_value);
