#include "../main.h"

Monitor* Monitor::p_instance = 0;

Monitor::Monitor()
{
}

void Monitor::Init(Settings* settings)
{
	BYTE currentDeviceID = DVC_LED_DISPLAY_EXT;

	bool deviceStatus = settings->getEnabledDevice(currentDeviceID);

	if (!deviceStatus) 
	{ 
		settings->pinDevice[DVC_LED_DISPLAY_EXT] = 0x00030200;
		settings->enabledDevice[DVC_LED_DISPLAY_EXT] = 1;

	}

	BYTE pinDIN = settings->getPinConfig(currentDeviceID, 1);
	BYTE pinLOAD = settings->getPinConfig(currentDeviceID, 2);
	BYTE pinCLOCK = settings->getPinConfig(currentDeviceID, 3);

	CLOCK = pinCLOCK;
	DIN = pinDIN;
	LOAD = pinLOAD;

	setPinModeMy(pinCLOCK, 0);
	setPinModeMy(pinLOAD, 0);
	setPinModeMy(pinDIN, 0);

	setGPIOState(pinCLOCK, 0);
	setGPIOState(pinLOAD, 1);
	setGPIOState(pinDIN, 0);

	setGPIOState(pinCLOCK, 0);
	setGPIOState(pinLOAD, 0);
	setGPIOState(pinDIN, 0);

	setGPIOState(pinCLOCK, 0);
	setGPIOState(pinLOAD, 1);
	setGPIOState(pinDIN, 0);

	MAX7219_Init(LOAD, DIN, CLOCK);

	setGPIOState(pinCLOCK, 0);
	setGPIOState(pinLOAD, 0);
	setGPIOState(pinDIN, 0);

	setGPIOState(pinCLOCK, 0);
	setGPIOState(pinLOAD, 1);
	setGPIOState(pinDIN, 0);

	MAX7219_Init(LOAD, DIN, CLOCK);
}

void Monitor::ReInit()
{
	  MAX7219_Write(REG_SCAN_LIMIT, 0x04);
  	  MAX7219_Write(REG_DECODE, 0x00);
	  MAX7219_ShutdownStop();
	  MAX7219_SetBrightness(settings->monitorLight);
}

void Monitor::setWorkPin(BYTE pinDIN, BYTE pinCLOCK, BYTE pinLOAD)
{
	CLOCK = pinCLOCK;
	DIN = pinDIN;
	LOAD = pinLOAD;

	setPinModeMy(pinCLOCK, 0);
	setPinModeMy(pinLOAD, 0);
	setPinModeMy(pinDIN, 0);

	setGPIOState(pinCLOCK, 0);
	setGPIOState(pinLOAD, 1);
	setGPIOState(pinDIN, 0);

	MAX7219_Init(LOAD, DIN, CLOCK);
}

void Monitor::showText(char* textStr)
{
	setPinModeMy(CLOCK, 0);
	setPinModeMy(LOAD, 0);
	setPinModeMy(DIN, 0);
	MAX7219_DisplayTestStop();                          // select normal operation (i.e. not test mode)
	MAX7219_DisplayDigStr(textStr);
}

void Monitor::testDisplay()
{
	MAX7219_DisplayTest();
}
