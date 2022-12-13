/*
 * Application Name     -   Get weather
 * Application Overview -   This is a sample application demonstrating how to
                            connect to openweathermap.org server and request for
              weather details of a city.
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_SLS_Get_Weather_Application
 *                          doc\examples\sls_get_weather.pdf
 */
 /* CC3100 booster pack connections (unused pins can be used by user application)
Pin  Signal        Direction      Pin   Signal     Direction
P1.1  3.3 VCC         IN          P2.1  Gnd   GND      IN
P1.2  PB5 UNUSED      NA          P2.2  PB2   IRQ      OUT
P1.3  PB0 UART1_TX    OUT         P2.3  PE0   SSI2_CS  IN
P1.4  PB1 UART1_RX    IN          P2.4  PF0   UNUSED   NA
P1.5  PE4 nHIB        IN          P2.5  Reset nRESET   IN
P1.6  PE5 UNUSED      NA          P2.6  PB7  SSI2_MOSI IN
P1.7  PB4 SSI2_CLK    IN          P2.7  PB6  SSI2_MISO OUT
P1.8  PA5 UNUSED      NA          P2.8  PA4   UNUSED   NA
P1.9  PA6 UNUSED      NA          P2.9  PA3   UNUSED   NA
P1.10 PA7 UNUSED      NA          P2.10 PA2   UNUSED   NA

Pin  Signal        Direction      Pin   Signal      Direction
P3.1  +5  +5 V       IN           P4.1  PF2 UNUSED      OUT
P3.2  Gnd GND        IN           P4.2  PF3 UNUSED      OUT
P3.3  PD0 UNUSED     NA           P4.3  PB3 UNUSED      NA
P3.4  PD1 UNUSED     NA           P4.4  PC4 UART1_CTS   IN
P3.5  PD2 UNUSED     NA           P4.5  PC5 UART1_RTS   OUT
P3.6  PD3 UNUSED     NA           P4.6  PC6 UNUSED      NA
P3.7  PE1 UNUSED     NA           P4.7  PC7 NWP_LOG_TX  OUT
P3.8  PE2 UNUSED     NA           P4.8  PD6 WLAN_LOG_TX OUT
P3.9  PE3 UNUSED     NA           P4.9  PD7 UNUSED      IN (see R74)
P3.10 PF1 UNUSED     NA           P4.10 PF4 UNUSED      OUT(see R75)

UART0 (PA1, PA0) sends data to the PC via the USB debug cable, 115200 baud rate
Port A, SSI0 (PA2, PA3, PA5, PA6, PA7) sends data to Nokia5110 LCD

*/
#include <stdio.h>
#include <stdlib.h>
#include "ST7735.h"
#include "tm4c123gh6pm.h"
#include "..\cc3100\simplelink\include\simplelink.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "application_commands.h"
#include "LED.h"
#include "Nokia5110.h"
#include <string.h>

// To Do: replace the following three lines with your access point information
#define SSID_NAME  "dylan" /* Access point name to connect to */
#define SEC_TYPE   SL_SEC_TYPE_WPA
#define PASSKEY    "12345678"  /* Password in case of secure AP */ 

#define BAUD_RATE   115200

void UART_Init(void){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  UARTStdioConfig(0,BAUD_RATE,50000000);
}

#define MAX_RECV_BUFF_SIZE  1024
#define MAX_SEND_BUFF_SIZE  512
#define MAX_HOSTNAME_SIZE   40
#define MAX_PASSKEY_SIZE    32
#define MAX_SSID_SIZE       32
#define SUCCESS             0
#define CONNECTION_STATUS_BIT   0
#define IP_AQUIRED_STATUS_BIT   1

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,/* Choosing this number to avoid overlap w/ host-driver's error codes */

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


/* Status bits - These are used to set/reset the corresponding bits in 'g_Status' */
typedef enum{
    STATUS_BIT_CONNECTION =  0, /* If this bit is:
                                 *      1 in 'g_Status', the device is connected to the AP
                                 *      0 in 'g_Status', the device is not connected to the AP
                                 */

    STATUS_BIT_IP_AQUIRED,       /* If this bit is:
                                 *      1 in 'g_Status', the device has acquired an IP
                                 *      0 in 'g_Status', the device has not acquired an IP
                                 */

}e_StatusBits;


#define SET_STATUS_BIT(status_variable, bit)    status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~(1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & (1<<(bit))))
#define IS_CONNECTED(status_variable)           GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_CONNECTION)
#define IS_IP_AQUIRED(status_variable)          GET_STATUS_BIT(status_variable, \
                                                               STATUS_BIT_IP_AQUIRED)

typedef struct{
    UINT8 SSID[MAX_SSID_SIZE];
    INT32 encryption;
    UINT8 password[MAX_PASSKEY_SIZE];
}UserInfo;

/*
 * GLOBAL VARIABLES -- Start
 */

char Recvbuff[MAX_RECV_BUFF_SIZE];
char SendBuff[MAX_SEND_BUFF_SIZE];
char HostName[MAX_HOSTNAME_SIZE];
unsigned long DestinationIP;
int SockID;


typedef enum{
    CONNECTED = 0x01,
    IP_AQUIRED = 0x02,
    IP_LEASED = 0x04,
    PING_DONE = 0x08

}e_Status;
UINT32  g_Status = 0;
/*
 * GLOBAL VARIABLES -- End
 */


 /*
 * STATIC FUNCTION DEFINITIONS  -- Start
 */

static int32_t configureSimpleLinkToDefaultState(char *);


/*
 * STATIC FUNCTION DEFINITIONS -- End
 */


void Crash(uint32_t time){
  while(1){
    for(int i=time;i;i--){};
    LED_RedToggle();
  }
}
/*
 * Application's entry point
 */
char string[200]="";
char stringempty[200]="";
char string2[200]="";

void ST7735_XYCloud(int32_t x, int32_t y,int32_t h, uint16_t color);
void ST7735_XYCloudRain(int32_t x, int32_t y,int32_t h, uint16_t color);
void ST7735_XYCloud(int32_t x, int32_t y,int32_t h, uint16_t color)
{
	//Bottom of cloud start
	ST7735_DrawPixel( x,  y, ST7735_WHITE);		
	ST7735_DrawPixel( x+1,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+2,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+3,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+4,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+5,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+6,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+7,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+8,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+9,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+10,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+11,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+12,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+13,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+14,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+15,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+16,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+17,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+18,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+19,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+20,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+21,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+22,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+23,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+24,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+25,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+26,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+27,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+28,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+29,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+30,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+31,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+32,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+33,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+34,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+35,  y, ST7735_WHITE);
	ST7735_DrawPixel( x+36,  y, ST7735_WHITE);
		//Bottom of cloud END
	ST7735_DrawPixel( x-1,  y-1, ST7735_WHITE);
	ST7735_DrawPixel( x-1,  y-2, ST7735_WHITE);
	ST7735_DrawPixel( x-1,  y-3, ST7735_WHITE);
	ST7735_DrawPixel( x-1,  y-4, ST7735_WHITE);
	ST7735_DrawPixel( x-1,  y-5, ST7735_WHITE);
	ST7735_DrawPixel( x-1,  y-6, ST7735_WHITE);
	ST7735_DrawPixel( x,  y-7, ST7735_WHITE);
	ST7735_DrawPixel( x,  y-8, ST7735_WHITE);
	ST7735_DrawPixel( x,  y-9, ST7735_WHITE);
	ST7735_DrawPixel( x+1,  y-9, ST7735_WHITE);
	ST7735_DrawPixel( x+1,  y-10, ST7735_WHITE);
	ST7735_DrawPixel( x+1,  y-11, ST7735_WHITE);
	ST7735_DrawPixel( x+1,  y-12, ST7735_WHITE);
	ST7735_DrawPixel( x+1,  y-13, ST7735_WHITE);
	ST7735_DrawPixel( x+2,  y-13, ST7735_WHITE);
	ST7735_DrawPixel( x+2,  y-14, ST7735_WHITE);
	ST7735_DrawPixel( x+2,  y-15, ST7735_WHITE);
	ST7735_DrawPixel( x+2,  y-16, ST7735_WHITE);
	ST7735_DrawPixel( x+2,  y-17, ST7735_WHITE);
	ST7735_DrawPixel( x+2,  y-18, ST7735_WHITE);
	ST7735_DrawPixel( x+3,  y-19, ST7735_WHITE);
	ST7735_DrawPixel( x+4,  y-20, ST7735_WHITE);
	ST7735_DrawPixel( x+4,  y-21, ST7735_WHITE);
	ST7735_DrawPixel( x+5,  y-22, ST7735_WHITE);
	ST7735_DrawPixel( x+6,  y-22, ST7735_WHITE);
	ST7735_DrawPixel( x+6,  y-23, ST7735_WHITE);
	ST7735_DrawPixel( x+7,  y-23, ST7735_WHITE);
	ST7735_DrawPixel( x+7,  y-24, ST7735_WHITE);
	ST7735_DrawPixel( x+8,  y-25, ST7735_WHITE);
	ST7735_DrawPixel( x+9,  y-25, ST7735_WHITE);
	ST7735_DrawPixel( x+10,  y-26, ST7735_WHITE);
	ST7735_DrawPixel( x+11,  y-27, ST7735_WHITE);
	ST7735_DrawPixel( x+12,  y-27, ST7735_WHITE);
	ST7735_DrawPixel( x+13,  y-28, ST7735_WHITE);
	ST7735_DrawPixel( x+14,  y-28, ST7735_WHITE);
	ST7735_DrawPixel( x+15,  y-29, ST7735_WHITE);
	ST7735_DrawPixel( x+16,  y-29, ST7735_WHITE);
	ST7735_DrawPixel( x+17,  y-30, ST7735_WHITE);
	ST7735_DrawPixel( x+18,  y-31, ST7735_WHITE);
	ST7735_DrawPixel( x+19,  y-30, ST7735_WHITE);
	ST7735_DrawPixel( x+20,  y-30, ST7735_WHITE);
	ST7735_DrawPixel( x+21,  y-30, ST7735_WHITE);
	ST7735_DrawPixel( x+22,  y-29, ST7735_WHITE);
	ST7735_DrawPixel( x+23,  y-28, ST7735_WHITE);
	ST7735_DrawPixel( x+24,  y-28, ST7735_WHITE);
	ST7735_DrawPixel( x+25,  y-27, ST7735_WHITE);
	ST7735_DrawPixel( x+26,  y-26, ST7735_WHITE);
	ST7735_DrawPixel( x+26,  y-25, ST7735_WHITE);
	ST7735_DrawPixel( x+26,  y-24, ST7735_WHITE);
	ST7735_DrawPixel( x+27,  y-23, ST7735_WHITE);
	ST7735_DrawPixel( x+27,  y-22, ST7735_WHITE);
	ST7735_DrawPixel( x+27,  y-21, ST7735_WHITE);

		ST7735_DrawPixel( x+28,  y-20, ST7735_WHITE);
	ST7735_DrawPixel( x+29,  y-19, ST7735_WHITE);
	ST7735_DrawPixel( x+29,  y-18, ST7735_WHITE);
	ST7735_DrawPixel( x+29,  y-17, ST7735_WHITE);
	ST7735_DrawPixel( x+30,  y-16, ST7735_WHITE);
	ST7735_DrawPixel( x+30,  y-15, ST7735_WHITE);
	ST7735_DrawPixel( x+30,  y-14, ST7735_WHITE);
	ST7735_DrawPixel( x+31,  y-13, ST7735_WHITE);
		ST7735_DrawPixel( x+31,  y-12, ST7735_WHITE);
			ST7735_DrawPixel( x+31,  y-11, ST7735_WHITE);
	ST7735_DrawPixel( x+32,  y-10, ST7735_WHITE);
	ST7735_DrawPixel( x+32,  y-9, ST7735_WHITE);
	ST7735_DrawPixel( x+33,  y-8, ST7735_WHITE);
	ST7735_DrawPixel( x+33,  y-7, ST7735_WHITE);
	ST7735_DrawPixel( x+34,  y-6, ST7735_WHITE);
	ST7735_DrawPixel( x+34,  y-5, ST7735_WHITE);
	ST7735_DrawPixel( x+35,  y-4, ST7735_WHITE);
	ST7735_DrawPixel( x+35,  y-3, ST7735_WHITE);
	ST7735_DrawPixel( x+35,  y-2, ST7735_WHITE);
	ST7735_DrawPixel( x+35,  y-1, ST7735_WHITE);
	ST7735_DrawPixel( x+36,  y, ST7735_WHITE);
	
}
void ST7735_XYCloudRain(int32_t x, int32_t y,int32_t h, uint16_t color)
{
	ST7735_DrawPixel( x+3,  y+11, ST7735_CYAN);	
	ST7735_DrawPixel( x+9,  y+15, ST7735_CYAN);	
	ST7735_DrawPixel( x+15,  y+6, ST7735_CYAN);	
	ST7735_DrawPixel( x+21,  y+23, ST7735_CYAN);	
	ST7735_DrawPixel( x+26,  y+19, ST7735_CYAN);	
	ST7735_DrawPixel( x+30,  y+20, ST7735_CYAN);	
	ST7735_DrawPixel( x+33,  y+21, ST7735_CYAN);	
	ST7735_DrawPixel( x+5,  y+23, ST7735_CYAN);	
	ST7735_DrawPixel( x+13,  y+2, ST7735_CYAN);	
	ST7735_DrawPixel( x+18,  y+32, ST7735_CYAN);	
	ST7735_DrawPixel( x+23,  y+2, ST7735_CYAN);	
	ST7735_DrawPixel( x+24,  y+9, ST7735_CYAN);	
	
}
void Systick_Wait10ms(uint32_t n);


void Systick_Wait10ms(uint32_t n){uint32_t volatile time;
  while(n){
//    time = 727240*2/91;  // 10msec for launchpad
    time = 7272/91;  // for simulation
    while(time){
	  	time--;
    }
    n--;
  }
}
	uint32_t x = 0,y = 100,z=100;
void Sunny();
void cloud();
void rain();
void rain(){
			ST7735_XYCloud(x,y,5,ST7735_RED);
			ST7735_XYCloudRain( x,  z, 5,  ST7735_RED);
			Systick_Wait10ms(10);
			ST7735_XYCloud(x,y,5,ST7735_RED);
			ST7735_XYCloudRain( x,  z, 5,  ST7735_RED);
			x++;
			z++;
			if(z>149){
			z = 100;
			x=0;
			}
			ST7735_FillRect(0, 60,128, 120, ST7735_BLACK);
		}
void cloud(){
			ST7735_XYCloud(x,y,5,ST7735_RED);
			Systick_Wait10ms(10);
			ST7735_XYCloud(x,y,5,ST7735_RED);
			x++;
			if(x>149){
			x=0;
			}
			ST7735_FillRect(0, 60,128, 120, ST7735_BLACK);
		}

void Sunny(){
		ST7735_FillCircle( x,  y,  12,  ST7735_YELLOW); 
		Systick_Wait10ms(10);
		ST7735_FillCircle( x,  y,  12,  ST7735_YELLOW); 
			x++;
			if(x>149){
			x=0;
			}
			ST7735_FillRect(0, 60,128, 120, ST7735_BLACK);
		}
uint32_t i;
#define MAXLEN 100
char City[MAXLEN];
char min_Temperature[MAXLEN];
char max_Temperature[MAXLEN];
char Weather[MAXLEN];
char Humidity[MAXLEN];
char WeatherID[MAXLEN];
char *pt = NULL;

// 1) change Austin Texas to your city
// 2) metric(for celsius), imperial(for fahrenheit)
// api.openweathermap.org/data/2.5/weather?q={city name},{state code}&appid={API key}

// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 7907b2abac2053aed180a74b9310b119 with your APPID
int main(void){int32_t retVal;  SlSecParams_t secParams;
	
	char wt_anim=0;
	char *pt0,*pt1,*pt2,*pt3,*pt4,*pt5,*pt6;
	
  char *pConfig = NULL; INT32 ASize = 0; SlSockAddrIn_t  Addr;
  ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(ST7735_BLACK);
	initClk();        // PLL 50 MHz
  UART_Init();      // Send data to PC, 115200 bps
  LED_Init();       // initialize LaunchPad I/O 
	char REQUESTCD[] = "GET /data/2.5/weather?q=";
	char REQUESTID[]="GET /data/2.5/weather?id=";
	char REQUESTzip[]="GET /data/2.5/weather?zip=";
	char REQUESTlat[]="GET /data/2.5/weather?lat=";
	char REQUESTlon[]="&lon=";
	char REQUEST1[] = "&APPID=1aaf3646666170bfb6b6cfa5f86c0257&units=imperial HTTP/1.1\r\nUser-Agent: Keil\r\nHost:api.openweathermap.org\r\nAccept: */*\r\n\r\n";
	char copy1[200] = "";
	char copy2[200] = "";
	char check=0;
	char result[200] = "";
	UARTprintf("Weather App\n");
  retVal = configureSimpleLinkToDefaultState(pConfig); // set policies
  if(retVal < 0)Crash(4000000);
  retVal = sl_Start(0, pConfig, 0);
  if((retVal < 0) || (ROLE_STA != retVal) ) Crash(8000000);
  secParams.Key = PASSKEY;
  secParams.KeyLen = strlen(PASSKEY);
  secParams.Type = SEC_TYPE; // OPEN, WPA, or WEP
  sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
  while((0 == (g_Status&CONNECTED)) || (0 == (g_Status&IP_AQUIRED))){
    _SlNonOsMainLoopTask();
  }
  UARTprintf("Connected\n");
  while(1){
		UARTprintf("Please choose your query criteria:\n\t 1. City Name\n\t 2. City ID\n\t 3. Geographic Coordinates\n\t 4. Zip Code\n");
				
				UARTgets(result,0);
				switch(result[0]){
				case '1':
						UARTprintf("\nInput City Name: ");
					UARTgets(string,0);
					strcpy(copy1,REQUESTCD);
					strcpy(copy2,REQUEST1);
					strcat(copy1,string);
					strcat(copy1,copy2);
					//UARTprintf(REQUEST);
					strcpy(SendBuff,copy1); 
					strcpy(copy1,"");
					strcpy(copy2,"");
					strcpy(result,"");
					break;
				case '2':
					UARTprintf("\nCity ID: ");
					UARTgets(string,0);
					strcpy(copy1,REQUESTID);
					strcpy(copy2,REQUEST1);
					strcat(copy1,string);
					strcat(copy1,copy2);
					//UARTprintf(REQUEST);
					strcpy(SendBuff,copy1); 
					strcpy(copy1,"");
					strcpy(copy2,"");
					strcpy(result,"");
					break;
				case '3':
					UARTprintf("\nGeographic Coordinates lat: ");
					UARTgets(string,0);
				UARTprintf("\nGeographic Coordinates lon: ");
					UARTgets(string2,0);
					strcpy(copy1,REQUESTlat);
					strcpy(copy2,REQUEST1);
					strcat(copy1,string);
					strcat(copy1,REQUESTlon);
					strcat(copy1,string2);
					strcat(copy1,copy2);
					UARTprintf(copy1);
					strcpy(SendBuff,copy1); 
					strcpy(copy1,"");
					strcpy(copy2,"");
					strcpy(result,"");
					break;
				case '4':
					UARTprintf("\nZip Code: ");
					UARTgets(string,0);
					strcpy(copy1,REQUESTzip);
					strcpy(copy2,REQUEST1);
					strcat(copy1,string);
					strcat(copy1,copy2);
					//UARTprintf(REQUEST);
					strcpy(SendBuff,copy1); 
					strcpy(copy1,"");
					strcpy(copy2,"");
					strcpy(result,"");
					break;
				default:
					UARTprintf("\nInvalid Selection");
					break;
			}
		
    strcpy(HostName,"api.openweathermap.org");
    retVal = sl_NetAppDnsGetHostByName(HostName,
             strlen(HostName),&DestinationIP, SL_AF_INET);
    if(retVal == 0){
      Addr.sin_family = SL_AF_INET;
      Addr.sin_port = sl_Htons(80);
      Addr.sin_addr.s_addr = sl_Htonl(DestinationIP);// IP to big endian 
      ASize = sizeof(SlSockAddrIn_t);
      SockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
      if( SockID >= 0 ){
        retVal = sl_Connect(SockID, ( SlSockAddr_t *)&Addr, ASize);
      }
      if((SockID >= 0)&&(retVal >= 0)){

				sl_Send(SockID, SendBuff, strlen(SendBuff), 0);// Send the HTTP GET 
        sl_Recv(SockID, Recvbuff, MAX_RECV_BUFF_SIZE, 0);// Receive response 
            Recvbuff[strlen(Recvbuff)] = '\0';
			}
		/* find ticker name in response*/
				pt = strstr(Recvbuff, "\"name\"");
				i = 0; 
				if( NULL != pt ){
					pt = pt + 8; // skip over "name":"
					while((i<MAXLEN)&&(*pt)&&(*pt!='\"')){
						City[i] = *pt; // copy into City string
						pt++; i++;    
					}
				}
				City[i] = 0;
			// find humidity in response 
                pt = strstr(Recvbuff, "\"humidity\"");
                i = 0; 
                if( NULL != pt ){
                    pt = pt + 11; // skip over "humidity":"
                    while((i<2)&&(*pt)&&(*pt!='"')){
                        Humidity[i] =*pt; // copy into weather string
                        pt++; i++;
                    }
                }
					Humidity[i]=0;
		// find Temperature Value in response */
				pt = strstr(Recvbuff, "\"temp_min\"");
				i = 0; 
				if( NULL != pt ){
					pt = pt + 11; // skip over "temp":
					while((i<4)&&(*pt)&&(*pt!='\"')){
						min_Temperature[i] = *pt; // copy into Temperature string
						pt++; i++;    
					}
				}
				min_Temperature[i] = 0;
								pt = strstr(Recvbuff, "\"temp_max\"");
				i = 0; 
				if( NULL != pt ){
					pt = pt + 11; // skip over "temp":
					while((i<4)&&(*pt)&&(*pt!='\"')){
						max_Temperature[i] = *pt; // copy into Temperature string
						pt++; i++;    
					}
				}
				max_Temperature[i] = 0;

		// find weather in response */
				pt = strstr(Recvbuff, "\"description\"");
				i = 0; 
				if( NULL != pt ){
					pt = pt + 15; // skip over "description":"
					while((i<MAXLEN)&&(*pt)&&(*pt!='\"')){
						Weather[i] = *pt; // copy into weather string
						
						pt++; i++;    
					}
				}
			Weather[i] = 0;   
								pt = strstr(Recvbuff, "\"icon\"");
				i = 0; 
				if( NULL != pt ){
					pt = pt + 8; // skip over "description":"
					while((i<MAXLEN)&&(*pt)&&(*pt!='\"')&&(*pt!='n')&&(*pt!='d')){
						WeatherID[i] = *pt; // copy into weather string
						
						pt++; i++;    
					}
				}
			WeatherID[i] = 0; 
				
				// Displaying info identifier to ST7735 LCD Screen
				sl_Close(SockID);
				ST7735_DrawString( 0,  0, "City:",  ST7735_GREEN);
				ST7735_DrawString( 0,  1, "Max Temp:",  ST7735_GREEN);
				ST7735_DrawString( 0,  2, "Min Temp:",  ST7735_GREEN);
				ST7735_DrawString( 0,  3, "Humidity:",  ST7735_GREEN);
				ST7735_DrawString( 0,  4, "Weather:",  ST7735_GREEN);
					
				// Displaying info to serial terminal
				UARTprintf("\nCity: ");
				UARTprintf(City);
				UARTprintf("\r\n");	
				UARTprintf("Temperature Maximum: ");
				UARTprintf(max_Temperature);
				UARTprintf("F\r\n");
				UARTprintf("Temperature Minimum: ");
				UARTprintf(min_Temperature);
				UARTprintf("F\r\n");
				UARTprintf("Humidity: ");
				UARTprintf(Humidity);
				UARTprintf("%%\r\n");
				UARTprintf("Weather: ");
				UARTprintf(Weather);
				UARTprintf("\r\n");
				
				// Displaying info to ST7735 LCD Screen
				ST7735_DrawString( 5,  0, City,  ST7735_CYAN);
				ST7735_DrawString( 9,  1, max_Temperature,  ST7735_CYAN);
				ST7735_DrawString( 9,  2, min_Temperature,  ST7735_CYAN);
				ST7735_DrawString( 9,  3, Humidity,  ST7735_CYAN);
				ST7735_DrawString( 8,  4, Weather,  ST7735_CYAN);
				ST7735_DrawString( 14, 1, "F", ST7735_CYAN);
				ST7735_DrawString( 14, 2, "F", ST7735_CYAN);
				
				LED_GreenOn();
        UARTprintf("\r\n\r\n");
        UARTprintf(Recvbuff);  UARTprintf("\r\n");
    
		}
		
		// Sunny 
		pt0=strstr(WeatherID,"01");
		
		// Cloudy
		pt1=strstr(WeatherID,"02");
		pt2=strstr(WeatherID,"03");
		pt3=strstr(WeatherID,"04");
		// Rainy
		pt4=strstr(WeatherID,"09");
		pt5=strstr(WeatherID,"10");
		pt6=strstr(WeatherID,"11");
		
    while(Board_Input()==0){
				if(pt0){
					Sunny();
				}
				else if(pt1||pt2||pt3){	
				  cloud();
				}
				else if(pt4||pt5||pt6){
					rain();
				}
		}
		LED_GreenOff();
    }
			
	}

static int32_t configureSimpleLinkToDefaultState(char *pConfig){
  SlVersionFull   ver = {0};
  UINT8           val = 1;
  UINT8           configOpt = 0;
  UINT8           configLen = 0;
  UINT8           power = 0;

  INT32           retVal = -1;
  INT32           mode = -1;

  mode = sl_Start(0, pConfig, 0);


    /* If the device is not in station-mode, try putting it in station-mode */
  if (ROLE_STA != mode){
    if (ROLE_AP == mode){
            /* If the device is in AP mode, we need to wait for this event before doing anything */
      while(!IS_IP_AQUIRED(g_Status));
    }

        /* Switch to STA role and restart */
    retVal = sl_WlanSetMode(ROLE_STA);

    retVal = sl_Stop(0xFF);

    retVal = sl_Start(0, pConfig, 0);

        /* Check if the device is in station again */
    if (ROLE_STA != retVal){
            /* We don't want to proceed if the device is not coming up in station-mode */
      return DEVICE_NOT_IN_STATION_MODE;
    }
  }
    /* Get the device's version-information */
  configOpt = SL_DEVICE_GENERAL_VERSION;
  configLen = sizeof(ver);
  retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (unsigned char *)(&ver));

    /* Set connection policy to Auto + SmartConfig (Device's default connection policy) */
  retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);

    /* Remove all profiles */
  retVal = sl_WlanProfileDel(0xFF);

  retVal = sl_WlanDisconnect();
  if(0 == retVal){
        /* Wait */
     while(IS_CONNECTED(g_Status));
  }

    /* Enable DHCP client*/
  retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&val);

    /* Disable scan */
  configOpt = SL_SCAN_POLICY(0);
  retVal = sl_WlanPolicySet(SL_POLICY_SCAN , configOpt, NULL, 0);

    /* Set Tx power level for station mode
       Number between 0-15, as dB offset from max power - 0 will set maximum power */
  power = 0;
  retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&power);

    /* Set PM policy to normal */
  retVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);

    /* TBD - Unregister mDNS services */
  retVal = sl_NetAppMDNSUnRegisterService(0, 0);


  retVal = sl_Stop(0xFF);


  g_Status = 0;
  memset(&Recvbuff,0,MAX_RECV_BUFF_SIZE);
  memset(&SendBuff,0,MAX_SEND_BUFF_SIZE);
  memset(&HostName,0,MAX_HOSTNAME_SIZE);
  DestinationIP = 0;;
  SockID = 0;


  return retVal; /* Success */
}

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent){
  switch(pWlanEvent->Event){
    case SL_WLAN_CONNECT_EVENT:
    {
      SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
    }
    break;

    case SL_WLAN_DISCONNECT_EVENT:
    {
      sl_protocol_wlanConnectAsyncResponse_t*  pEventData = NULL;

      CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
      CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_AQUIRED);

      pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            /* If the user has initiated 'Disconnect' request, 'reason_code' is SL_USER_INITIATED_DISCONNECTION */
      if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code){
        UARTprintf(" Device disconnected from the AP on application's request \r\n");
      }
      else{
        UARTprintf(" Device disconnected from the AP on an ERROR..!! \r\n");
      }
    }
    break;

    default:
    {
      UARTprintf(" [WLAN EVENT] Unexpected event \r\n");
    }
    break;
  }
}

void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent){
  switch(pNetAppEvent->Event)
  {
    case SL_NETAPP_IPV4_ACQUIRED:
    {

      SET_STATUS_BIT(g_Status, STATUS_BIT_IP_AQUIRED);
			
    }
    break;

    default:
    {
            UARTprintf(" [NETAPP EVENT] Unexpected event \r\n");
    }
    break;
  }
}

void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse){

  UARTprintf(" [HTTP EVENT] Unexpected event \r\n");
}

void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent){

  UARTprintf(" [GENERAL EVENT] \r\n");
}

void SimpleLinkSockEventHandler(SlSockEvent_t *pSock){
  switch( pSock->Event )
  {
    case SL_NETAPP_SOCKET_TX_FAILED:
    {
      switch( pSock->EventData.status )
      {
        case SL_ECLOSE:
          UARTprintf(" [SOCK EVENT] Close socket operation failed to transmit all queued packets\r\n");
          break;


        default:
          UARTprintf(" [SOCK EVENT] Unexpected event \r\n");
          break;
      }
    }
    break;

    default:
      UARTprintf(" [SOCK EVENT] Unexpected event \r\n");
    break;
  }
}

