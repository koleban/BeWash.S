#include "../main.h"

extern char comports[30][60];

Engine8400* Engine8400::p_instance = NULL;

Engine8400::Engine8400()
{
}

void Engine8400::Init(Settings* settings)
{
	// Get device pin 0 - DVC_SENSOR_BYPASS
	bypassValve 		= settings->getPinConfig(DVC_SENSOR_BYPASS, 1);
	comPortBaundRate	= settings->engine_baundRate;
	regState = 0;
	lastRegState = 0;
	startTime = 0;

	if (settings->getEnabledDevice(DVC_SENSOR_BYPASS))
	{
		if (bypassValve != 0xFF)
		{
			setGPIOState(bypassValve, 1);
			setPinModeMy(bypassValve, 1);
  			pullUpDnControl (bypassValve, PUD_DOWN) ;
  		}
	}
	engineCommandTryCount = 10;
}

bool Engine8400::IsOpened()
{
	return isOpened;
}

bool Engine8400::OpenDevice()
{
	if (IsOpened()) return true;
	// Открываем COM порт
	if ((streamId = serialOpen(comports[comPortNumber], comPortBaundRate)) < 0)
	{
		printf("[ENGINE] ERROR: >> Unable to open serial device: (%s)\n", comports[comPortNumber]);
		return false;
	}
	isOpened = true;
	return true;
}

bool Engine8400::CloseDevice()
{
	serialClose(streamId);
	streamId = 0;
	isOpened = false;
	return true;
}

bool Engine8400::engineStart(int freq)
{
	if (!IsOpened())
	{
		CloseDevice();
		if (!OpenDevice())
		{
			term_setattr(31);
			if (errorCode != 999) printf("[ENGINE] ERROR: >> Engine open port failed!\n");
			term_setattr(0);
			errorCode = 999;
			return false;
		}
	}

	if (!engineTestConnection()) { engineRotates = 0; if (errorCode != ERR_ENGINE_NO_CONNECTION) printf("[ENGINE] ERROR: >> Engine test connection failed!\n"); errorCode = ERR_ENGINE_NO_CONNECTION; regState = 0x00; bypassMode = 1; return false;}

	/*
	CTRL_ENGINE_DEVICE_ID,
	0x10, 							// Write group register
	0x00, 0x01, 					// Start register
	0x00, 0x02, 					// Register count
	0x04, 							// Data length
	0x00, 0x01, 0x01, 0xF1 , 		// Data
									//	Reg 0x0000 - 1 Start forward
									//	Reg 0x0001 - Freq
	0x00, 0x00
	*/
	char command[13] = { CTRL_ENGINE_DEVICE_ID, 0x10, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00, 0x01, 0x01, 0xF1 , 0x00, 0x00};
	command[ 9] = (freq >> 8) & 0xFF;
	command[10] = freq & 0xFF;

	bool result = RS485_doCommandS(streamId, &command[0], sizeof(command));

	if (startTime == 0)
		startTime = get_uptime() - workTimeSec;

	return result;
}

bool Engine8400::engineStop()
{
	if (!IsOpened()) return false;

	if (!engineTestConnection()) { engineRotates = 0; term_setattr(31); if (errorCode != ERR_ENGINE_NO_CONNECTION) printf("[ENGINE] ERROR: >> Engine test connection failed!\n"); term_setattr(0); errorCode = ERR_ENGINE_NO_CONNECTION; regState = 0x00; bypassMode = 1; return false;}

	//char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x06, 0xEA, 0x10, 0x84, 0x07, 0x00, 0x00 };
	//char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x10, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00 };
	char command[13] = { CTRL_ENGINE_DEVICE_ID, 0x10, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00};
	bool result = RS485_doCommandS(streamId, &command[0], sizeof(command));

	if ((startTime > 0) && (workTimeSec == 0))
		workTimeSec = (get_uptime() - startTime);
	startTime = 0;

	engineRotates = false;
	needFreq = 0;
	return !engineRotates;
}

bool Engine8400::engineTestConnection()
{
	if (!IsOpened()) return false;
	bool testResult = false;

	int timeOut = 0;

	engineCommandTryCount = 10;
	while (!testResult && (timeOut++ < engineCommandTryCount))
	{
		char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x08, 0xAA, 0xAA, 0xBB, 0xBB, 0x00, 0x00 };
		testResult = RS485_doCommandS(streamId, &command[0], sizeof(command));
        if (!testResult) delay_ms(10);
	}

	return testResult;
}

bool Engine8400::engineUpdate()
{
	if (!engineTestConnection()) { engineRotates = 0; if (errorCode != ERR_ENGINE_NO_CONNECTION) printf("[ENGINE] ERROR: >> Engine test connection failed!\n"); errorCode = ERR_ENGINE_NO_CONNECTION; regState = 0x00; bypassMode = 1; return false;}
	errorCode = ERR_ENGINE_NO_ERROR;

	bool testResult = false;
	int timeOut = 0;


	//
	// Ask current state
	//
	while (!testResult && (timeOut++ < engineCommandTryCount))
	{
		char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x03, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00 };
		testResult = RS485_doCommand(streamId, &command[0]);
		if (!testResult) { delay_ms(50); continue; }
		if (command[1] == 0x03)
		{
			regState = (command[3] << 8) + command[4];
			if (regState == lastRegState) break;
			lastRegState = regState;
		}
	}

	timeOut = 0;
	testResult = false;

	while (!testResult && (timeOut++ < engineCommandTryCount))
	{
		char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x03, 0x00, 0x21, 0x00, 0x01, 0x00, 0x00 };
		testResult = RS485_doCommand(streamId, &command[0]);
		if (!testResult) { delay_ms(50); continue; }
		if (command[1] == 0x03)
		{
			currFreq = (command[3] << 8) + command[4];
			engineRotates = (currFreq > 1);
			bypassMode = ((currFreq > 0) && (currFreq < 550));
		}
	}

	if ((startTime > 0) && (currFreq > 1))
		workTimeSec = (get_uptime() - startTime);

	return true;
}

Engine* Engine8400::getInstance()
{
    if(!Engine8400::p_instance)
        Engine8400::p_instance = new Engine8400();
    return (Engine*)Engine8400::p_instance;
}
