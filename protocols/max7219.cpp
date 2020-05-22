#include "../main.h"

/*
*********************************************************************************************************
* Constants
*********************************************************************************************************
*/
#define PAUSE 1

unsigned char LOAD_PORT;
unsigned char DATA_PORT;
unsigned char CLK_PORT;

void MAX7219_Init (unsigned char LOAD, unsigned char DATA, unsigned char CLOCK);
void MAX7219_ShutdownStart (void);
void MAX7219_ShutdownStop (void);
void MAX7219_DisplayTestStart (void);
void MAX7219_DisplayTestStop (void);
void MAX7219_SetBrightness (char brightness);
void MAX7219_Clear (void);
void MAX7219_DisplayChar (char digit, char character);
void MAX7219_DisplayDig (int digit);
void MAX7219_DisplayTest();

/*
*********************************************************************************************************
* Private Function Prototypes
*********************************************************************************************************
*/
void MAX7219_Write (unsigned char reg_number, unsigned char data);
void MAX7219_SendByte (unsigned char data);
unsigned char MAX7219_LookupCode (char character);


// ...................................... Public Functions ..............................................


/*
*********************************************************************************************************
* MAX7219_Init()
*
* Description: Initialize MAX7219 module; must be called before any other MAX7219 functions.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_Init (unsigned char LOAD, unsigned char DATA, unsigned char CLOCK)
{
	LOAD_PORT = LOAD;
	DATA_PORT = DATA;
	CLK_PORT = CLOCK;

  MAX7219_Write(REG_SCAN_LIMIT, 0x03);                   // set up to scan all eight digits
  MAX7219_Write(REG_DECODE, 0x00);                    // set to "no decode" for all digits
  MAX7219_ShutdownStop();                             // select normal operation (i.e. not shutdown)
  MAX7219_DisplayTestStop();                          // select normal operation (i.e. not test mode)
  MAX7219_Clear();                                    // clear all digits
  MAX7219_SetBrightness(INTENSITY_MAX);               // set to maximum intensity
}


/*
*********************************************************************************************************
* MAX7219_ShutdownStart()
*
* Description: Shut down the display.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_ShutdownStart (void)
{
  MAX7219_Write(REG_SHUTDOWN, 0);                     // put MAX7219 into "shutdown" mode
}


/*
*********************************************************************************************************
* MAX7219_ShutdownStop()
*
* Description: Take the display out of shutdown mode.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_ShutdownStop (void)
{
  MAX7219_Write(REG_SHUTDOWN, 1);                     // put MAX7219 into "normal" mode
}


/*
*********************************************************************************************************
* MAX7219_DisplayTestStart()
*
* Description: Start a display test.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_DisplayTestStart (void)
{
  MAX7219_Write(REG_DISPLAY_TEST, 1);                 // put MAX7219 into "display test" mode
}


/*
*********************************************************************************************************
* MAX7219_DisplayTestStop()
*
* Description: Stop a display test.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_DisplayTestStop (void)
{
  MAX7219_Write(REG_DISPLAY_TEST, 0);                 // put MAX7219 into "normal" mode
}


/*
*********************************************************************************************************
* MAX7219_SetBrightness()
*
* Description: Set the LED display brightness
* Arguments  : brightness (0-15)
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_SetBrightness (char brightness)
{
  brightness &= 0x0f;                                 // mask off extra bits
  MAX7219_Write(REG_INTENSITY, brightness);           // set brightness
}


/*
*********************************************************************************************************
* MAX7219_Clear()
*
* Description: Clear the display (all digits blank)
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_Clear (void)
{
  char i;
  for (i=0; i < 8; i++)
    MAX7219_Write(i, 0x00);                           // turn all segments off
}


/*
*********************************************************************************************************
* MAX7219_DisplayChar()
*
* Description: Display a character on the specified digit.
* Arguments  : digit = digit number (0-7)
*              character = character to display (0-9, A-Z)
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_DisplayChar (char digit, char character)
{
   MAX7219_Write(digit, MAX7219_LookupCode(character));
//   MAX7219_Write(digit, character);
}


// ..................................... Private Functions ..............................................


/*
*********************************************************************************************************
* LED Segments:         a
*                     ----
*                   f|    |b
*                    |  g |
*                     ----
*                   e|    |c
*                    |    |
*                     ----  o dp
*                       d
*   Register bits:
*      bit:  7  6  5  4  3  2  1  0
*           dp  a  b  c  d  e  f  g

*********************************************************************************************************
*/
struct MaxFonts{
   char   ascii;
   char   segs;
} MAX7219_Font[] = {
  {' ', 0x00},
  {'-', 0x01},
  {'_', 0x08},
  {'~', 0x40},
  {'0', 0x7e},
  {'1', 0x30},
  {'2', 0x6d},
  {'3', 0x79},
  {'4', 0x33},
  {'5', 0x5b},
  {'6', 0x5f},
  {'7', 0x70},
  {'8', 0x7f},
  {'9', 0x7b},
  {'A', 0x77},
  {'B', 0x1f},
  {'P', 0x67},
  {'C', 0x4e},
  {'D', 0x3d},
  {'H', 0x37},
  {'E', 0x4f},
  {'S', 0x5B},
  {'F', 0x47},
  {'r', 0x05},
  {'l', 0x10},
  {'L', 0x20},
  {'i', 0x04},
  {'{', 0x02},
  {'h', 0x97},
  {'.', 0x80},
  {'G', 0x63},
  {'o', 0x1D},
  {'\0', 0x00}
};

/*
*********************************************************************************************************
* MAX7219_LookupCode()
*
* Description: Convert an alphanumeric character to the corresponding 7-segment code.
* Arguments  : character to display
* Returns    : segment code
*********************************************************************************************************
*/
unsigned char MAX7219_LookupCode (char character)
{
  char i;
  for (i = 0; MAX7219_Font[i].ascii; i++)             // scan font table for ascii code
    if (character == MAX7219_Font[i].ascii)
      return MAX7219_Font[i].segs;                    // return segments code
  return 0;                                           // code not found, return null (blank)
}


/*
*********************************************************************************************************
* MAX7219_Write()
*
* Description: Write to MAX7219
* Arguments  : reg_number = register to write to
*              dataout = data to write to MAX7219
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_Write (unsigned char reg_number, unsigned char dataout)
{
   MAX7219_SendByte(reg_number);                       // write register number to MAX7219
   MAX7219_SendByte(dataout);                          // write data to MAX7219
   setGPIOState(LOAD_PORT, 1);                                           // take LOAD high to end
   delay_us(PAUSE);
   setGPIOState(LOAD_PORT, 0);                                           // take LOAD high to begin
   delay_us(PAUSE);
   setGPIOState(DATA_PORT, 0);
   delay_us(PAUSE);
}


/*
*********************************************************************************************************
* MAX7219_SendByte()
*
* Description: Send one byte to the MAX7219
* Arguments  : dataout = data to send
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_SendByte (unsigned char dataout)
{
   char i;
   for (i=8; i>0; i--)
   {
      unsigned char mask = 1 << (i - 1);
	  if (dataout & mask)
		  setGPIOState(DATA_PORT, 1);
      else
         setGPIOState(DATA_PORT, 0);
	   delay_us(PAUSE);
	  setGPIOState(CLK_PORT, 1);
	   delay_us(PAUSE);
	  setGPIOState(CLK_PORT, 0);
   	  delay_us(PAUSE);
   }
}

void MAX7219_DisplayDig (int digit)
{
  	int index;
  	char buffer[15];
  	sprintf(buffer, "%4d", digit);
  	index = strlen(buffer);
  	MAX7219_Write(REG_SCAN_LIMIT, 0x04);                   // set up to scan all eight digits
	MAX7219_DisplayChar(1, buffer[index-1]);
	MAX7219_DisplayChar(2, buffer[index-2]);
	MAX7219_DisplayChar(3, buffer[index-3]);
	MAX7219_DisplayChar(4, buffer[index-4]);
}

void MAX7219_DisplayDigStr (char* digit)
{
	char chDigit = ' ';
	for (int index=0; index < 4; index++)
	{
		if (strlen(digit) <= index)
			chDigit = ' ';
		else
        	strncpy(&chDigit, &digit[3-index], 1);
		MAX7219_DisplayChar(index+1, chDigit);
		delay_ms(1);
	}
}

void MAX7219_DisplayTest()
{
   int t=0;
   char ch[10];
   for(t=0; t<10; t++)
   {
   		sprintf(ch, "%d", t);
        MAX7219_DisplayChar(1, ch[0]);
        MAX7219_DisplayChar(2, ch[0]);
        MAX7219_DisplayChar(3, ch[0]);
        MAX7219_DisplayChar(4, ch[0]);
        delay_ms(200);
   }
}
