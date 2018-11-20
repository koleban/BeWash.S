#ifndef _MAX7219_H_
#define _MAX7219_H_

#define REG_DECODE        0x09                        // "decode mode" register
#define REG_INTENSITY     0x0a                        // "intensity" register
#define REG_SCAN_LIMIT    0x0b                        // "scan limit" register
#define REG_SHUTDOWN      0x0c                        // "shutdown" register
#define REG_DISPLAY_TEST  0x0f                        // "display test" register

#define INTENSITY_MIN     0x00                        // minimum display intensity
#define INTENSITY_MAX     0x0E                        // maximum display intensity

void MAX7219_Init (BYTE LOAD, BYTE DATA, BYTE CLOCK);
void MAX7219_ShutdownStart (void);
void MAX7219_ShutdownStop (void);
void MAX7219_DisplayTestStart (void);
void MAX7219_DisplayTestStop (void);
void MAX7219_SetBrightness (char brightness);
void MAX7219_Clear (void);
void MAX7219_DisplayChar (char digit, char character);
void MAX7219_DisplayDig (unsigned long digit);
void MAX7219_DisplayDigStr (char* digit);
void MAX7219_Write (BYTE reg_number, BYTE data);
void MAX7219_SendByte (BYTE data);
BYTE MAX7219_LookupCode (char character);
void MAX7219_DisplayTest();

#endif