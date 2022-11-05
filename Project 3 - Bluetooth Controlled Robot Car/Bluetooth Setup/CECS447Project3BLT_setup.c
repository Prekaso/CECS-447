// CECS447Project3BLT_setup.c
// Documentation
// CECS447 Project 3 - Bluetooth Controlled Robot Car
// Description: Interface between two DC motors, a smartphone/laptop,
//								and LEDs using an HC-05 bluetooth module and LaunchPad.
// Student Names: Dylan Dang & Hoang Nguyen

// Code for Bluetooth Setup

// Inputs
//		PA0 - Connects to computer (Internal connection via usb to U0Rx)
//		PB0 - Connects to HC-05 Bluetooth module Pin TXD (U1Rx -> TXD)

// Outputs
//		PA1 - Connects to computer (Internal connection via usb to U0Tx)
//		PB1 - Connects to HC-05 Bluetooth module Pin RXD (U1Tx -> RXD)
//		Vbus - Connects to HC-05 Bluetooth module VCC Pin
//		3.3V - Connects to HC-05 Bluetooth module EN Pin
//		GND - Connects to HC-05 Bluetooth module GND Pin

#include <stdint.h> // C99 data types
#include "tm4c123gh6pm.h"
#include "UART.h"
#include "BLT_Setup.h"

void OutCRLF(void){
  UART0_OutChar(CR);
  UART0_OutChar(LF);
}

int main(void){
	UART_Init();
	BLT_Setup_Init();
	OutCRLF();
	
	UART0_OutString(">>> Welcome to Serial Terminal. <<<");
	OutCRLF();
	UART0_OutString(">>> This is the setup program for HC-05 Bluetooth module. <<<");
	OutCRLF();
	UART0_OutString(">>> You are st 'AT' Command Mode. <<<");
	OutCRLF();
	UART0_OutString(">>> Type 'AT' and followed with a command. <<<");
	OutCRLF();
	UART0_OutString(">>> Example: AT+NAME=DH <<<");
	OutCRLF();
	
	char command[30];
	char message[30];
	
	while(1){
		UART0_InString(command, 29);
		BLT_OutString(command);
		BLT_OutString("\r\n");
		while ((UART1_FR_R&UART_FR_BUSY) != 0){};
		BLT_InString(message);	
		OutCRLF();
		UART0_OutString(message);
		
		if(command[7] == '?'){
			BLT_InString(message);
			OutCRLF();
			UART0_OutString(message);
		}
		OutCRLF();
		OutCRLF();
	}
	
}