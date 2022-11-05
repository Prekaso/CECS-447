// This is an example program for music programming.
// You will run this program without modification.
// The program will repeatedly play a specified song.

// The program will continuously play one of the song  
// defined in mysong on a speaker connected to pin PA2.
// Authors: Min He
// Date: August 26, 2018

// 1. Header files 
#include "tm4c123gh6pm.h"
#include "SysTick.h"
#define NVIC_EN0_PORTF 0x40000000

// 2. Declarations Section
unsigned int play_flag = 0;
unsigned int change_flag = 0;
unsigned int i;
unsigned int current_song_index = 0;

// define music data structure 
struct Note {
  unsigned char tone_index;
  unsigned char delay;
};
typedef const struct Note NTyp;

// initial values for piano major tones.
// Assume SysTick clock frequency is 16MHz.
const unsigned long Tone_Tab[] =
// Notes:  C, D, E, F, G, A, B
// Offset: 0, 1, 2, 3, 4, 5, 6
{30534,27211,24242,22923,20408,18182,16194, // C4 Major notes
15289,13621,12135,11454,10204,9091,8099, // C5 Major notes
7645,6810,6067,5727,5102,4545,4050}; // C6 Major notes

#define PAUSE 255				// assume there are less than 255 tones used in any song

// indexes for notes used in music scores
#define C5 0+7
#define D5 1+7
#define E5 2+7
#define F5 3+7
#define G5 4+7
#define A5 5+7
#define B5 6+7
#define C6 0+2*7
#define D6 1+2*7
#define E6 2+2*7
#define F6 3+2*7
#define G6 4+2*7
#define A6 5+2*7
#define B6 6+2*7

// doe ray mi fa so la ti 
// C   D   E  F  G  A  B
NTyp mysong[][50] =
{
// score table for Mary Had A Little Lamb
{E6, 4, D6, 4, C6, 4, D6, 4, E6, 4, E6, 4, E6, 8, 
 D6, 4, D6, 4, D6, 8, E6, 4, G6, 4, G6, 8,
 E6, 4, D6, 4, C6, 4, D6, 4, E6, 4, E6, 4, E6, 8, 
 D6, 4, D6, 4, E6, 4, D6, 4, C6, 8, 0, 0 },

// score table for Twinkle Twinkle Little Stars
{C6,4,C6,4,G6,4,G6,4,A6,4,A6,4,G6,8,F6,4,F6,4,E6,4,E6,4,D6,4,D6,4,C6,8, 
 G6,4,G6,4,F6,4,F6,4,E6,4,E6,4,D6,8,G6,4,G6,4,F6,4,F6,4,E6,4,E6,4,D6,8, 
 C6,4,C6,4,G6,4,G6,4,A6,4,A6,4,G6,8,F6,4,F6,4,E6,4,E6,4,D6,4,D6,4,C6,8,0,0},

// score table for Happy Birthday
{//so   so   la   so   doe' ti
   G5,2,G5,2,A5,4,G5,4,C6,4,B5,4,
// pause so   so   la   so   ray' doe'
   PAUSE,4,  G5,2,G5,2,A5,4,G5,4,D6,4,C6,4,
// pause so   so   so'  mi'  doe' ti   la
   PAUSE,4,  G5,2,G5,2,G6,4,E6,4,C6,4,B5,4,A5,8, 
// pause fa'  fa'   mi'  doe' ray' doe' stop
	 PAUSE,4,  F6,2,F6,2, E6,4,C6,4,D6,4,C6,8,0,0}
};

	// Function Prototypes
void Speaker_Init(void);
void Delay(void);
void Switch_Init(void);
void GPIOPortF_Handler(void);
extern void DisableInterrupts(void); // Disable interrupts
extern void EnableInterrupts(void);  // Enable interrupts
extern void WaitForInterrupt(void);  // low power mode
void play_a_song(NTyp scoretab[]);

unsigned char is_music_on(void) {
	return 1;
}

// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){
	unsigned char i =0;
	DisableInterrupts();
  Speaker_Init();
  SysTick_Init();
	Switch_Init();
  EnableInterrupts();  // SysTick uses interrupts
	
  while(1){
		if (is_music_on()) {
			play_a_song(mysong[current_song_index]);
		} else {
			WaitForInterrupt();
		}
	}
}

void play_a_song(NTyp scoretab[])
{
	unsigned char i=0, j;
	unsigned int current_song = current_song_index;
	
	while ((scoretab[i].delay) && (play_flag != 0) && (current_song == current_song_index)) {
		if (scoretab[i].tone_index==PAUSE) // index = 255 indicate a pause: stop systick
			SysTick_stop(); 								// silence tone, turn off SysTick timer
		else {
			SysTick_Set_Current_Note(Tone_Tab[scoretab[i].tone_index]);
			SysTick_start();
		}
		
		// tempo control: 
		// play current note for duration 
		// specified in the music score table
		for (j=0;j<scoretab[i].delay;j++) 
			Delay();
		
		SysTick_stop();
		i++;  // move to the next note
	}
	
	// pause after each play
	for (j=0;j<15;j++) 
		Delay();
}

// Subroutine to wait 0.1 sec for 16MHz system clock
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	unsigned long volatile time;
  time = 727240*20/91;  // 0.1sec
  while(time){
		time--;
  }
}

// Make PA3 an output, enable digital I/O, ensure alt. functions off
void Speaker_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x01;           // 1) activate clock for Port A
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
                                    // 2) no need to unlock PA3
  GPIO_PORTA_PCTL_R &= ~0x0000F000; // 3) regular GPIO
  GPIO_PORTA_AMSEL_R &= ~0x08;      // 4) disable analog function on PA3
  GPIO_PORTA_DIR_R |= 0x08;         // 5) set direction to output
  GPIO_PORTA_AFSEL_R &= ~0x08;      // 6) regular port function
  GPIO_PORTA_DEN_R |= 0x08;         // 7) enable digital port
  //GPIO_PORTA_DR8R_R |= 0x08;        // 8) optional: enable 8 mA drive on PA3 to increase the voice volume
}

void Switch_Init(void){  
	unsigned long volatile delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // (a) activate clock for port F
  delay = SYSCTL_RCGC2_R;
	//dutyA  = PERIOD/2;            // 50% duty cycle, assume system clock 16MHz, 
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // unlock GPIO Port F
  GPIO_PORTF_CR_R |= 0x11;         // allow changes to PF4,0
  GPIO_PORTF_DIR_R &= ~0x11;    // (c) make PF4,0 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x11;  //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R |= 0x11;     //     enable digital I/O on PF4,0
  GPIO_PORTF_PCTL_R &= ~0x000F000F; //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x11;  //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;    //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xF00FFFFF)|0x02000000; // (g) bits:23-21 for PORTF, set priority to 2
  NVIC_EN0_R |= NVIC_EN0_PORTF;      // (h) enable interrupt 30 in NVIC
}

void GPIOPortF_Handler(void){
	for (i=0;i<=200000;i++){}   // delay for button press to stabilize
		
  if(GPIO_PORTF_RIS_R&0x01){  // if SW2 pressed, change song
    GPIO_PORTF_ICR_R = 0x01;  // acknowledge flag0
		if (play_flag == 1) {
			current_song_index = (current_song_index + 1)%3;
		}
  }
	
  if(GPIO_PORTF_RIS_R&0x10){  	// if SW1 pressed, turn off music on/off
		GPIO_PORTF_ICR_R = 0x10;  	// acknowledge flag4
			if(play_flag == 0){
				play_flag = 1;					// music starts
			}
			else if(play_flag == 1){
				play_flag = 0;					// music stops
				current_song_index = 0; 
			}
  }
}

