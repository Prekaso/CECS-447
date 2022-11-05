// Dylan Dang & Hoang Nguyen
// CECS 447
// Project 2 - UART - Mode 1
// PC <--> MCU1

#include "UART.h"
#include "tm4c123gh6pm.h"
#include <string.h>
#include <stdio.h>

#define	LEDS 		(*((volatile unsigned long *)0x40025038))
#define R 0x02
#define G 0x08
#define B 0x04
#define P 0x06
#define W 0x0E
#define D 0x00

#define NVIC_EN0_PORTF 0x40000000

char MENU_CHOICE;
char LED_CHOICE;
unsigned int i;
unsigned int INTERRUPT;

void PortF_Init(void);
extern void DisableInterrupts(void);
extern void EnableInterrupts(void);  

void UART0_CRLF(void) {
	UART0_OutChar(CR);
	UART0_OutChar(LF);
}

void UART1_CRLF(void) {
	UART1_OutChar(CR);
	UART1_OutChar(LF);
}

int main(void){
	char string[255]; // holds original user inputted message
	char string2[255]; // holds MCU2 response message
	unsigned char COLOR;
	
  UART_Init(); // initialize UART
  UART0_CRLF();
	UART1_CRLF();
	PortF_Init();
	UART_Init();
	EnableInterrupts();
  
	while(1){
		UART0_OutString("Welcome to CECS 447 Project 2 - UART");
		UART0_CRLF();
		UART0_OutString("Please choose a communication mode(type 1 or 2 or 3):");
		UART0_CRLF();
		UART0_OutString("1. PC <-> MCU1 Only");
		UART0_CRLF();
		UART0_OutString("2. MCU1 <-> MCU2 LED Control");
		UART0_CRLF();
		UART0_OutString("3. PC <-> MCU1 <-> MCU2 Messenger");
		UART0_CRLF();

		MENU_CHOICE = UART0_InChar(); 
		UART1_OutChar(MENU_CHOICE);   
		UART0_CRLF();
		
		// MODE 1
		if(MENU_CHOICE == '1') {
		UART0_OutString("Type letter for LED color: r,b,g,p,w, or d");
		UART0_CRLF();
		LED_CHOICE = UART0_InChar(); //Receives led choice from terminal
		UART0_CRLF();
			if(LED_CHOICE == 'r') {
				LEDS = R;
				UART0_OutString("Red LED is on \n");
				UART0_CRLF();
			}
			else if(LED_CHOICE == 'b') {
				LEDS = B;
				UART0_OutString("Blue LED is on \n");
				UART0_CRLF();
			}
			else if(LED_CHOICE == 'g') {
				LEDS = G;
				UART0_OutString("Green LED is on \n");
				UART0_CRLF();
			}
			else if(LED_CHOICE == 'p') {
				LEDS = P;
				UART0_OutString("Purple LED is on \n");
				UART0_CRLF();
			}
			else if(LED_CHOICE == 'w') {
				LEDS = W;
				UART0_OutString("White LED is on \n");
				UART0_CRLF();
			}
			else if(LED_CHOICE == 'd') {
				LEDS = D;
				UART0_OutString("LED is off \n");
				UART0_CRLF();
			}
		}
		
		// MODE 2
		if(MENU_CHOICE == '2') {
			UART0_OutString("MODE 2 ACTIVATED");
			UART0_CRLF();
			INTERRUPT = 2;
			LEDS = D;
			COLOR = UART1_InChar();
			LEDS = COLOR;
		}
		
		// MODE 3
		if(MENU_CHOICE == '3') {
			LEDS = G; // starts with Green LED when in mmode 3
			UART0_OutString("Enter a message: "); // prompts user to enter string from PC
			UART0_CRLF();
			UART0_InString(string, 254); // receives message from PC and stores it in string
			UART0_CRLF();
			UART1_OutString(string); //sends the string to MCU2
			UART1_CRLF();
			UART1_InString(string, 254); //receives acknowledgement from MCU2
			UART1_InString(string2, 254); // receives empty string from MCU2
			UART0_OutString(string2); // sends empty string to PC
			UART1_InString(string2, 254); //receives 2nd message from MCU2
			UART0_OutString(string2); // sends 2nd message to PC
			UART0_CRLF();
			LEDS = D;
		}
	}
}

void PortF_Init(void){ 
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // (a) activate clock for port F
  while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x11;           // allow changes to PF4-0
    
  GPIO_PORTF_AMSEL_R &= ~0x11;        // 3) disable analog function
  GPIO_PORTF_PCTL_R &= 0x000F000F;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R &= ~0x11;          // 5) PF4,PF0 input   
  GPIO_PORTF_AFSEL_R &= ~0x11;        // 6) no alternate function
  GPIO_PORTF_PUR_R |= 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R |= 0x11;          // 7) enable digital pins PF4-PF0        
	GPIO_PORTF_DIR_R |= 0x0E;    // (c) make PF3-1 output (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x0E;  //     disable alt funct on PF3-1
	GPIO_PORTF_DEN_R |= 0x0E;     //     enable digital I/O on PF3-1
		
	GPIO_PORTF_IS_R &= ~0x11;     // (d) PF0,4 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF0,4 is not both edges
  GPIO_PORTF_IEV_R |= 0x11;    //     PF4 falling edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flag4
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF0FFFFF)|0x00C00000; // (g) priority 6
  NVIC_EN0_R |= NVIC_EN0_PORTF;      // (h) enable interrupt 30 in NVIC
}

void GPIOPortF_Handler(void) {	
	for(i = 0; i <= 200000; i++){} //delay for button press
	if(GPIO_PORTF_RIS_R & 0x10) { // switch 1, cycles through LEDs
		if(INTERRUPT == 2) {
			switch(LEDS) {
				case D:
					LEDS = R;
					break;
				case R:
					LEDS = G;
					break;
				case G:
					LEDS = B;
					break;
				case B:
					LEDS = P;
					break;
				case P:
					LEDS = W;
					break;
				case W:
					LEDS = D;
					break;
				default: 
					break;
			}
		}
		GPIO_PORTF_ICR_R = 0x10;
	}
	if(GPIO_PORTF_RIS_R & 0x01) { // switch 2
		if(INTERRUPT == 2) {
			UART1_OutChar('1'); //sends flag value to MC2
			UART1_OutChar(LEDS);		
			GPIO_PORTF_ICR_R = 0x01;
			UART1_OutChar(CR);
			INTERRUPT = 0;
		}
	}
	GPIO_PORTF_ICR_R = 0x01;
}

