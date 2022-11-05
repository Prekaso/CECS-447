// CECS 447
// BluetoothSetUp.c
// Main program for setting up bluetooth module
// Settings that are set include name, UART configuration
// passcode, and mode


// Header files 
#include "tm4c123gh6pm.h"
#include "UART.h"
#include "string.h"

void OutCRLF(void){
  UART0_OutChar(CR);
  UART0_OutChar(LF);
}

int main(void) {
  UART_Init();        //initialize UART
	
  OutCRLF();
	UART0_OutString(">>> Welcome to Serial Terminal <<<");
	OutCRLF();
	UART0_OutString(">>> This is the setup program for HC-05 Bluetooth module <<<");
	OutCRLF();
	UART0_OutString(">>> You are at 'AT' Command Mode. <<<");
	OutCRLF();
	UART0_OutString(">>> Type 'AT' and follow with a command <<<");
	OutCRLF();
	
	char command[30];
	char message[30];
	
	while(1) {
		// setup the HC-05 bluetooth module
		UART0_InString(command, 29); //User Input from terminal
		//strcat(string,"\r\n");
		UART1_OutString(command); 		//Outputs string to bluetooth module
		UART1_OutString("\r\n"); 		//Ends command sent to bluetooth module
		
		while ((UART1_FR_R&UART_FR_BUSY) != 0){};
		BLT_InString(message);       //Confirmation from bluetooth module
		OutCRLF();
		UART0_OutString(message);    //Outputs message from bluetooth module
			
		if(command[7] == '?') {
			BLT_InString(message);
			OutCRLF();
			UART0_OutString(message);
		}
		OutCRLF();
		OutCRLF();
	}
}
