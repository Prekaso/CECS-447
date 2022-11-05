/////////////////////////////////////////////////////////////////////////////
// Names: Dylan Dang & Hoang Nguyen
// Course Number: CECS 447
// Assignment: Project 3 - UART
// Description: Utilitze Bluetooth to control robot car
/////////////////////////////////////////////////////////////////////////////

#include "tm4c123gh6pm.h"
#include "PWM.h"
#include "GPIO.h"

// Function Prototypes
void DisableInterrupts(void);
void EnableInterrupts(void);
void WaitForInterrupt(void);
void Delay(void);
void UART_Init(void);
unsigned char UART1_InChar(void);
void BLT_InString(unsigned char *bufPt);
void UART0_OutChar(unsigned char data);
void UART0_OutString(unsigned char *pt);
void forw(void);
void back(void);
void lef(void);
void righ(void);
void blef(void);
void brigh(void);
void stop(void);

// MAIN: This main is meant for the command configuration of the hc05.
int main(void){ 
  LED_Init();
  Car_Dir_Init();
  PWM_PB76_Init();
  PWM_PB76_Duty(START_SPEED, START_SPEED);
	
		unsigned char control_symbol; // for bluetooth controlled LEDs
//  unsigned char str[30];      // for testing strings from Bluetooth

	UART_Init(); // Initialize UART1
	UART0_OutString((unsigned char *)">>> Welcome to Bluetooth Controlled LED App <<<\n\r");
	
		while(1) {
    control_symbol = UART1_InChar();
    UART0_OutChar(control_symbol);
		UART0_OutChar(CR);
    UART0_OutChar(LF);

    switch (control_symbol){
      case 'F':
      case'f': 
				forw();
        break; 
      case 'B':
      case 'b':
        back();
        break; 
      case 'L':
      case 'l':
			if( WHEEL_DIR == BACKWARD){
				  blef();
			}
			else{
				  lef();
			}
        break; 
      case 'R':
      case'r': 
			if( WHEEL_DIR == BACKWARD){
					brigh();
			}
			else{
					righ();
			}
        break; 
      case 'S':
      case 's':
					stop();
        break; 
      case 'U':
      case 'u':
        PWM_PB76_Duty(START_SPEED + 1600, START_SPEED + 1600);
        break; 
      case 'D':
      case 'd':
        PWM_PB76_Duty(START_SPEED - 1600, START_SPEED - 1600);
        break; 
      default:
        break;
    }
	}
    
}

void forw(void){
	LED = Green;
	WHEEL_DIR = FORWARD;
	PWM0_ENABLE_R |= 0x00000003; // enable both wheels
}

void back(void){
	LED = Blue;
	WHEEL_DIR = BACKWARD;
	PWM0_ENABLE_R |= 0x00000003; // enable both wheels
}

void lef(void){
				LED = Yellow;
				WHEEL_DIR=FORWARD;
				PWM0_ENABLE_R &= ~0x00000002; // Enable right wheel
				PWM0_ENABLE_R |= 0x00000001; // Disable left wheel
}

void blef(void){
				LED = Yellow;
			  PWM0_ENABLE_R |= 0x00000002; // Enable right wheel
	      PWM0_ENABLE_R &= ~0x00000001; // Disable left wheel
}

void righ(void){
				LED = Purple;
				WHEEL_DIR=FORWARD;
				PWM0_ENABLE_R |= 0x00000002; // Enable right wheel
				PWM0_ENABLE_R &= ~0x00000001; // Disable left wheel
}

void brigh(void){
				LED = Purple;
			  PWM0_ENABLE_R &= ~0x00000002; // Enable right wheel
	      PWM0_ENABLE_R |= 0x00000001; // Disable left wheel
}

void stop(void){
	LED = Dark;
	PWM0_ENABLE_R &= ~0x00000003; // stop both wheels
}

// Subroutine to wait 0.25 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){
	unsigned long volatile time;
  time = 727240*500/91;  // 0.25sec
  while(time){
		time--;
  }
}

//------------UART_Init------------
// Initialize the UART for 19200 baud rate (assuming 16 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void){
	// Activate Clocks
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART1; // activate UART1
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate port B
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0; // activate UART0
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // activate port A
	
	
	UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART0_IBRD_R = 17;                    // IBRD = int(16,000,000 / (16 * 57600)) = int(17.3611111)
  UART0_FBRD_R = 23;                     // FBRD = round(3611111 * 64) = 27
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= 0x301;                 // enable UART for both Rx and Tx

  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1,PA0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1,PA0
                                        // configure PA1,PA0 as UART0
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA1,PA0
	
  UART1_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
	
	// Data Communication Mode, Buad Rate = 57600
  UART1_IBRD_R = 17;                    // IBRD = int(16,000,000 / (16 * 57600)) = int(17.3611111)
  UART1_FBRD_R = 23;                     // FBRD = round(3611111 * 64) = 27
	
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART1_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART1_CTL_R |= 0x301;                 // enable UART for both Rx and Tx
  
  GPIO_PORTB_AFSEL_R |= 0x03;           // enable alt funct on PB1,PB0
  GPIO_PORTB_DEN_R |= 0x03;             // enable digital I/O on PB1,PB0
                                        // configure PB1,PB0 as UART1
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTB_AMSEL_R &= ~0x03;          // disable analog functionality on PB1,PB0

}

//------------UART0_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART0_OutChar(unsigned char data){
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}

//------------UART0_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART0_OutString(unsigned char *pt){
  while(*pt){
    UART0_OutChar(*pt);
    pt++;
  }
}

//------------UART1_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
unsigned char UART1_InChar(void){
  while((UART1_FR_R&UART_FR_RXFE) != 0);
  return((unsigned char)(UART1_DR_R&0xFF));
}

// This function reads response from HC-05 Bluetooth module.
void BLT_InString(unsigned char *bufPt) {
  unsigned char length=0;
  bufPt[length] = UART1_InChar();
  
  // Two possible endings for a reply from HC-05: OK\r\n, ERROR:(0)\r\n
  while (bufPt[length]!=LF) {
    length++;
    bufPt[length] = UART1_InChar();
  };
    
  // add null terminator
  length++;
  bufPt[length] = 0;
}

