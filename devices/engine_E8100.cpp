#include "../main.h"

extern char comports[30][60];

Engine8100* Engine8100::p_instance = NULL;

Engine8100::Engine8100()
{
}

void Engine8100::Init(Settings* settings)
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
}

bool Engine8100::IsOpened()
{
	return isOpened;
}

bool Engine8100::OpenDevice()
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

bool Engine8100::CloseDevice()
{
	serialClose(streamId);
	streamId = 0;
	isOpened = false;
	return true;
}

bool Engine8100::engineStart(int freq)
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

	//char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x06, 0xEA, 0x11, 0x13, 0x88, 0x00, 0x00 };
	char command[13] = { CTRL_ENGINE_DEVICE_ID, 0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x01, 0x01, 0xF1 , 0x00, 0x00};
	command[ 9] = (freq >> 8) & 0xFF;
	command[10] = freq & 0xFF;

	bool result = RS485_doCommandS(streamId, &command[0], sizeof(command));

//	char command2[8] = { CTRL_ENGINE_DEVICE_ID, 0x06, 0xEA, 0x10, 0x84, 0x00, 0x00, 0x00 };
//	char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x10, 0x00, 0x24, 0x10, 0x00, 0x00, 0x00 };
//	result |= RS485_doCommand(streamId, &command2[0]);
	if (startTime == 0)
		startTime = get_uptime() - workTimeSec;

	return result;
}

bool Engine8100::engineStop()
{
	if (!IsOpened()) return false;

	if (!engineTestConnection()) { engineRotates = 0; term_setattr(31); if (errorCode != ERR_ENGINE_NO_CONNECTION) printf("[ENGINE] ERROR: >> Engine test connection failed!\n"); term_setattr(0); errorCode = ERR_ENGINE_NO_CONNECTION; regState = 0x00; bypassMode = 1; return false;}

	//char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x06, 0xEA, 0x10, 0x84, 0x07, 0x00, 0x00 };
	char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x10, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00 };
	bool result = RS485_doCommand(streamId, &command[0]);

	if ((startTime > 0) && (workTimeSec == 0))
		workTimeSec = (get_uptime() - startTime);
	startTime = 0;

	engineRotates = false;
	needFreq = 0;
	return !engineRotates;
}

bool Engine8100::engineTestConnection()
{
	if (!IsOpened()) return false;
	bool testResult = false;

	int timeOut = 0;

	while (!testResult && (timeOut++ < engineCommandTryCount))
	{
		//char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x03, 0xED, 0x10, 0x00, 0x01, 0x00, 0x00 };
		char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x08, 0x00, 0x00, 0xA5, 0x37, 0x00, 0x00 };
		testResult = RS485_doCommandS(streamId, &command[0], 8);
        if (!testResult) delay_ms(50);
	}

	return testResult;
}

bool Engine8100::engineUpdate()
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
		char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x03, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00 };
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
		char command[8] = { CTRL_ENGINE_DEVICE_ID, 0x03, 0x00, 0x24, 0x00, 0x01, 0x00, 0x00 };
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

Engine* Engine8100::getInstance()
{
    if(!Engine8100::p_instance)
        Engine8100::p_instance = new Engine8100();
    return (Engine*)Engine8100::p_instance;
}
