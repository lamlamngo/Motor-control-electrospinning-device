#pragma CODE	//generate assembly code in the LST file
//------------------------------------------------------------------------------------
// Program: C
// Author: Lam And Sophia
// Date: 6/6/16
// Purpose: Utilize A to D converter to generate PWM. Configure motor driver and motor  
//------------------------------------------------------------------------------------
// Include  files
#include <c8051f020.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1
//----------------------------------------------------------
// 16-bit SFE definitions for 'F02x
//----------------------------------------------------------

sfr16 DP  = 0x82; 	//data pionter
sfr16 TMR3RL  = 0x92; 	//Timer3 reload value
sfr16 TMR3  = 0x94; 	//Timer3 counter
sfr16 ADC0  = 0xbe; 	//ADC0 data
sfr16 ADC0GT  = 0xc4; 	//ADC0 greater than window
sfr16 ADC0LT  = 0xc6; 	//ADC0 less than window
sfr16 RCAP2  = 0xca; 	//Timer2 capture/reload
sfr16 T2  = 0xcc; 	//Timer2
sfr16 RCAP4 = 0xe4; 	//Timer4 capture/reload
sfr16 T4  = 0xf4; 	//Timer4
sfr16 DAC0  = 0xd2; 	//DAC0 data
sfr16 DAC1  = 0xd5; 	//DAC1 data
//------------------------------------------------------------------------------------
// Global CONSTANTS
//------------------------------------------------------------------------------------

#define SYSCLK  	22118400 	//SYSCLK frequency in Hz
#define BAUDRATE	9600     	//Baud rate of UART in bps
#define CMD 254
#define CLS  0x01

sbit motor_pin_1 = P0^0;
sbit motor_pin_2 = P0^1;

sbit LED = P1^6;             	//LED
//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
void delay();
void config(void);
void PWMInit(void);
void SYSCLK_Init(void);
void UART1_Init(void);
void ADC0Init(void);
char getUART0Char();
unsigned char num_to_ascii(unsigned char);
unsigned int getADC0Value(void);
void num_to_string(unsigned int number, char *num_str);

//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------
void main (void) {
unsigned int value;
   	WDTCN = 0xde;    	// disable watchdog timer
   	WDTCN = 0xad;

 	SYSCLK_Init();   //initialize oscillaor
 	config();    	// setup ports
    	UART1_Init();	// initialize UART 1
    	ADC0Init();   	// initialize ADC0
    	PWMInit();
    	for(;;){
        	value = getADC0Value();
        	PCA0CPH0 = value/6;
   	 }   
}

// convert the A/D value to an ascii string
void num_to_string(unsigned int number, char *num_str) {
int i, j;
unsigned int remainder;
	j = 3;
	for (i = 0; i < 4; i++) {
   	remainder = number % 10;
   	number = number / 10;
   	num_str[j] = num_to_ascii(remainder);
   	j--;
	}
	num_str[4] = 0x00;
}
// convert a number to a single ascii char
unsigned char num_to_ascii(unsigned char num) {
unsigned char ascii_char;

	ascii_char = num + 0x30;
	return(ascii_char);
}
// get a value from ADC0
unsigned int getADC0Value(void) {
unsigned int value;

  	// start conversion
  	AD0BUSY = 1;
  	// wait for confversion to be complete
  	while (AD0INT == 0);
  	// read value
  	value = ADC0;
  	return(value);

}
// send a single character to UART 1
void sendUART1Char( unsigned char ch) {
   SBUF1 = ch; // send character to UART transmit buffer
   while (!(SCON1 & 0x02)); // wait for transmit interupt flag to be set
                        	// indicating that the character has been sent
   SCON1 &= 0xFC;   // interupt flag must be reset to 0 by software
}
void delay(void)
{
  int i,j;
  for(i=0;i<1000;i++)
  {
	for(j=0;j<1000;j++)
	{
	}
  }
}
//------------------------------------------------------------------------------------
// PORT_Init
//------------------------------------------------------------------------------------
void PWMInit(void){
	PCA0CN = 0x40;            	//Enalbe PCA counter  
	PCA0CPM0 = 0x42;          	//Configure module for 8-bit PWN mode
	PCA0CPL0 = 0x40;          	//duty cycle
	PCA0CPH0 = 0x40;
	PCA0L = 0x00;
	PCA0H = 0x00;
}

void config (void)
{    
	P0MDOUT   = 0x15;
	XBR0  	= 0x0C;
	XBR1   	= 0x01;
	XBR2  	= 0x44;
}

void SYSCLK_Init(void)   
{
	int i;                   	//delay counter
	OSCXCN = 0x67;          	//start external oscillator with //22.1184 MHz crystal
	for (i=0; i<256; i++);  	//wait for oscillator to start
	while(!(OSCXCN & 0x88));	//wait for crystal oscillator to settle
	OSCICN = 0x88;          	//select external oscillator as SYSCLK
                            	//source and enable missing clock detector
}
// initialize ADC0
void ADC0Init(void){
	//use DAC0 for Vref enable internal Vref and internal bias generator
	REF0CN = 0x13;  
	// ADC configuration - enable ADC0, use AD0BUSY, right justified and AIN0 and AIN1
	// are single ended, PGA gain = 1
	AMX0CF = 0x60;
	AMX0SL = 0x00;   // select AIN0
	ADC0CF = 0xA8;	// ADC Configuration Register 22.118400MHz / MHz - 1 = 1, gain = 1
	ADC0CN = 0x80; // enable ADC0
	// configure DAC
	DAC0CN = 0x80;
	DAC0L = 0xFF;
	DAC0H = 0x0F;
}

/*UART1_Init
Configure the UART1 using Timer 4, for <baudrate> and 8-N-1
UART 1 uses P0 pin 2 for TX ansd P0 pin 3 for RCV
*/
void UART1_Init(void)
{
	SCON1 = 0x50;            	//SCON1: mode 1, 8-bit UART, enable RX
	SCON1 &= 0xFC;          	// clear interupt pending flags
	T4CON = 0x30;
	RCAP4 = -(SYSCLK/BAUDRATE/32);
	T4 = RCAP4;
	CKCON |= 0x40;
	PCON |= 0x10;
	T4CON |= 0x04;
}
