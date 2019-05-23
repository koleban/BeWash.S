#include "../main.h"

extern char comports[30][60];

EngineEmu* EngineEmu::p_instance = NULL;

EngineEmu::EngineEmu()
{
	bypassCounter = 0;
	currFreq = 0;
}

void EngineEmu::Init(Settings* settings)
{
	// Get device pin 0 - DVC_SENSOR_BYPASS
	bypassValve 		= settings->getPinConfig(DVC_SENSOR_BYPASS, 1);
	comPortBaundRate	= settings->engine_baundRate;

	if (settings->getEnabledDevice(DVC_SENSOR_BYPASS))
	{
		if (bypassValve != 0xFF)
		{
			setGPIOState(bypassValve, 1);
			setPinModeMy(bypassValve, 1);
  			pullUpDnControl (bypassValve, PUD_DOWN) ;
  		}
	}
	if (settings->engine_relay > 0)
	{
		setGPIOState(settings->engine_relay, 0);
		setPinModeMy(settings->engine_relay, 0);
		pullUpDnControl (settings->engine_relay, PUD_DOWN) ;
	}
}

bool EngineEmu::IsOpened()
{
	return isOpened;
}

bool EngineEmu::OpenDevice()
{
	if (IsOpened()) return true;
	// Открываем COM порт
	isOpened = true;
	return true;
}

bool EngineEmu::CloseDevice()
{
	streamId = 0;
	isOpened = false;
	startTime = 0;
	return true;
}

bool EngineEmu::engineStart(int freq)
{
	bool result = true;
	needFreq = freq;
	currFreq = freq;
//	char command2[8] = { CTRL_ENGINE_DEVICE_ID, 0x06, 0xEA, 0x10, 0x84, 0x00, 0x00, 0x00 };
//	char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x10, 0x00, 0x24, 0x10, 0x00, 0x00, 0x00 };
//	result |= RS485_doCommand(streamId, &command2[0]);
	if (startTime == 0)
		startTime = get_uptime() - workTimeSec;
	if ((freq > 500) && (settings->engine_relay > 0))
	{
		setGPIOState(settings->engine_relay, 1);
	}
	else if (settings->engine_relay > 0)
	{
		setGPIOState(settings->engine_relay, 0);
	}
	return result;
}

bool EngineEmu::engineStop()
{
	if (!IsOpened()) return false;

	bool result = true;

	if ((startTime > 0) && (workTimeSec == 0))
		workTimeSec = (get_uptime() - startTime);
	startTime = 0;

	if (settings->engine_relay > 0)
	{
		setGPIOState(settings->engine_relay, 0);
	}

	engineRotates = false;
	needFreq = 0;
	currFreq = needFreq;
	return !engineRotates;
}

bool EngineEmu::engineTestConnection()
{
	if (!IsOpened()) return false;
	bool testResult = false;

	return testResult;
}

bool EngineEmu::engineUpdate()
{
	errorCode = ERR_ENGINE_NO_ERROR;

	bool testResult = false;
	int timeOut = 0;

	//currFreq = needFreq;
	engineRotates = (currFreq > 1);
	if (settings->getEnabledDevice(DVC_SENSOR_BYPASS))
	{
		if (bypassValve != 0xFF)
		{
			if (getGPIOState(bypassValve) == 0)
				bypassCounter ++;
			else
				bypassCounter = 0;
			bypassMode = (bypassCounter > 1);
		}
	}

	if (settings->engine_relay > 0)
	{
		if ((needFreq > 500) && (!bypassMode))
			setGPIOState(settings->engine_relay, 1);
		else
			setGPIOState(settings->engine_relay, 0);
	}

	if (bypassMode)
		regState = 2;
	else
		regState = 1;
	if (startTime > 0)
		workTimeSec = (get_uptime() - startTime);
	return true;
}

Engine* EngineEmu::getInstance()
{
    if(!EngineEmu::p_instance)
        EngineEmu::p_instance = new EngineEmu();
    return (Engine*)EngineEmu::p_instance;
}
