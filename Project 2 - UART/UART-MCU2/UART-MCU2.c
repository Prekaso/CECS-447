// Dylan Dang & Hoang Nguyen
// CECS 447
// Project 2 - UART - Mode 2 & 3
// MCU1 <--> MCU2_LED_CONTROL
// PC <--> MCU1 <--> MCU2 <--> Nokia5110 Messenger

#include "UART.h"
#include "Nokia5110.h"
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
char RETURN_COLOR;
unsigned int i;
unsigned int INTERRUPT;
char string[255];  
char ack[255] = "I received: ";  
unsigned char COLOR;

void PortF_Init(void);
void Nokia5110_Init(void);
void Nokia5110_OutChar(unsigned char data);
void Nokia5110_OutString(unsigned char *ptr);
void Nokia5110_Clear(void);
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
	Nokia5110_Init();
  Nokia5110_Clear();
	Nokia5110_SetCursor(0, 0); 
  UART_Init();            
  UART0_CRLF();
	UART1_CRLF();
	PortF_Init();
	EnableInterrupts();

	
  while(1){
		MENU_CHOICE = UART1_InChar(); // Receives menu choice from MC1
		if(MENU_CHOICE == '2') {
			RETURN_COLOR = UART1_InChar(); //flag to allow MC2 to change COLORs only when MC1 has sent a COLOR
			if(RETURN_COLOR == '1') {
				INTERRUPT = 2;
				COLOR = UART1_InChar();//Receives COLOR from MCU1
				LEDS = COLOR;
			}
	}
		if(MENU_CHOICE == '3') {
			EnableInterrupts();
			LEDS = D;
			UART1_InString(string, 254); //Receives Input from MCU1
			UART0_CRLF();
			Nokia5110_Clear();
			Nokia5110_OutString((unsigned char*)string); //outputs string on Nokia LCD
			while (MENU_CHOICE == '3') {}
			DisableInterrupts();
			Nokia5110_Clear();
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
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm INTERRUPT on PF4
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF0FFFFF)|0x00C00000; // (g) priority 6
  NVIC_EN0_R |= NVIC_EN0_PORTF;      // (h) enable INTERRUPT 30 in NVIC
}

void GPIOPortF_Handler(void)
{	
	for(i=0;i<=200000;i++){}//delay for button press to stabilize
  if(GPIO_PORTF_RIS_R & 0x10) { // switch 1
		if(INTERRUPT == 2) {
			switch (LEDS) {
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
		if(MENU_CHOICE == '3') {
				UART1_OutString(""); // sends empty string to MCU1 to clear previous message
				UART0_CRLF();
				UART1_OutString(ack); // sends 2nd message to MCU1
				UART0_CRLF();
				UART1_OutString(string); //Sends back 1st message to MCU1
				UART0_CRLF();
		}
		GPIO_PORTF_ICR_R = 0x10;
	}

	if(GPIO_PORTF_RIS_R & 0x01) { // switch 2
		if(INTERRUPT == 2) {
		UART1_OutChar(LEDS);	//sends LED COLOR to MCU1	
		GPIO_PORTF_ICR_R = 0x01;
		INTERRUPT = 0;
		RETURN_COLOR = '0'; 
		}
	}
	GPIO_PORTF_ICR_R = 0x01;
}

