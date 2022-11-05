// UART.h
// Runs on TM4C123 or LM4F120
// this connection occurs in the USB debugging cable
// U1Rx (PB0)
// U1Tx (PB1)
// Ground connected ground in the USB cable

#include <stdint.h>

// standard ASCII symbols
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F

//------------UART_Init------------
// Initialize the UART for 115200 baud rate (assuming 80 MHz clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void);

//------------UART0_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART0_OutChar(unsigned char data);

//------------UART1_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART1_OutChar(unsigned char data);

//------------UART0_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART0_OutString(char *pt);

//------------UART1_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART1_OutString(char *pt);

//------------UART0_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
unsigned char UART0_InChar(void);

//------------UART1_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
unsigned char UART1_InChar(void);

//------------UART0_InString------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until <enter> is typed
//    or until max length of the string is reached.
// It echoes each character as it is inputted.
// If a backspace is inputted, the string is modified
//    and the backspace is echoed
// terminates the string with a null character
// uses busy-waiting synchronization on RDRF
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
int UART0_InString(char *bufPt, uint16_t max);
//int UART1_InString(char *bufPt, uint16_t max);

//void UART0_InString(char *bufPt, unsigned short max);
void UART1_InString(char *bufPt, unsigned short max);
//------------BLT_InString------------
// Gets reply from HC-05 Bluetooth module 
void BLT_InString(char *bufPt);
