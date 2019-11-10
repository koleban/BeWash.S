#include "../main.h"

//#pragma region Engine
PI_THREAD(EngineWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.EngineWatch) return (void*)0;
	bool engineStatus = false;
	bool stopFailed = 0;
	int requestCount = 3;
	int timeout = 500;

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] Engine: Engine control thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	settings->workFlag.EngineWatch = 0;
	engine->Init(settings);
	if (!engine->OpenDevice())
		printf("[ENG]: Communication port error!\n");

	settings->workFlag.EngineWatch = 0;
	engineStatus = false;
		timeout = requestCount;
	while (!engineStatus && (timeout-->0))
		{ engineStatus = engine->engineUpdate(); delay_ms(100); }
	engineStatus = false;
	timeout = requestCount;
		settings->workFlag.EngineWatch = 0;
	while (!engineStatus && (timeout-->0))
	{
		engineStatus |=	engine->engineStart(0);
		engineStatus |=	engine->engineStop();
		if (!engineStatus) delay(1);
	}

	if (settings->engine_relay > 0)
	{
		setGPIOState(settings->engine_relay, 0);
		setPinModeMy(settings->engine_relay, PIN_OUTPUT);
		pullUpDnControl (settings->engine_relay, PUD_DOWN) ;
	}

	term_setattr(31);
	if (timeout == 0) printf("DBG:> [ENG] Engine INIT stop FAILED!!!\n");
	term_setattr(0);
	int bypassPinNum = settings->getPinConfig(DVC_SENSOR_BYPASS, 1);
	if (!settings->getEnabledDevice(DVC_SENSOR_BYPASS)) bypassPinNum = 0xFF;
	int lastPower = 0;
	while (settings->threadFlag.EngineWatch)
	{
		requestCount = 3;
		settings->workFlag.EngineWatch = 0;
		timeout = 500;
		while ((settings->busyFlag.EngineWatch) && (timeout-- > 0)) {delay_ms(1);}
		if (timeout == 0) settings->busyFlag.EngineWatch = 0;
		settings->busyFlag.EngineWatch++;
        engineStatus = false;
		timeout = requestCount;
		while (!engineStatus && (timeout-->0))
			engineStatus = engine->engineUpdate();
		settings->intErrorCode.EngineWatch = (engine->errorCode == 0)?0:201;

		if (
			(engine->currFreq > 0) &&
			(
				(engine->needFreq < 1) ||
				(
					(
						(status.intDeviceInfo.money_currentBalance < 1) ||
						(status.intDeviceInfo.program_currentProgram < 1)
					) && (winterModeEngineActive == 0)
				)
			)
		   )
		{
			if (settings->debugFlag.EngineWatch)
				printf("[ENG] [%3d:%02d:%02d] -- [c: %d; n: %d; bal: %d]\n",
					((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60 ,
					engine->currFreq, engine->needFreq, status.intDeviceInfo.money_currentBalance);
			engineStatus = false;
			timeout = requestCount;
			while (!engineStatus && (timeout-->0))
			{
				if (settings->engine_relay > 0)
				{
					if (settings->debugFlag.EngineWatch)
						printf("[ENG] [%2d:%02d:%02d] ENGINE RELAY TURN OFF [prev: %d; pin: %d]\n",
						((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60,
						getGPIOState(settings->engine_relay), settings->engine_relay);
					setGPIOState(settings->engine_relay, 0);
				}
				engine->engineUpdate();
				delay_ms(100);
				engine->needFreq = 1;
				engineStatus |=	engine->engineStart(1);
				delay_ms(500);
				engineStatus |=	engine->engineStop();
				delay_ms(settings->commonParams.engine_StartStopTimeMs);
			}
			term_setattr(31);
			if (timeout == 0)
				if (settings->debugFlag.EngineWatch) printf("DBG:> [ENG] Timeout [FLAG: 9]\n");
			term_setattr(0);
		}

        if ((engine->currFreq != engine->needFreq) || (getGPIOState(bypassPinNum) == 0) || (lastPower != engine->powerA))
		{
			if (engine->needFreq > 0)
			{
				if (settings->debugFlag.EngineWatch)
					printf("[ENG] [%2d:%02d:%02d] >> [p:%d c:%d; n:%d; bal:%d; bp:%d] [power: %d.%02dA]\n",
						((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60,
						status.intDeviceInfo.program_currentProgram, engine->currFreq, engine->needFreq, status.intDeviceInfo.money_currentBalance, (getGPIOState(bypassPinNum) == 0),
						(int)(engine->powerA/10), engine->powerA%10);
				lastPower = engine->powerA;
				// calc time for bypass turn ON
				int checkTime = 0;
				int errTimeCount = 0;
				if (getGPIOState(bypassPinNum) == 0)
					for (int itmp=0; itmp<settings->bypassTimeMs; itmp++)
					{
						if (getGPIOState(bypassPinNum) == 0)
						{
							checkTime++;
							delay_ms(1);
						}
						else
						{
							if (errTimeCount++ > 10)
								break;
						}
					}
				// if bypass turn ON and is it not NOISE
				if ((checkTime > ((int)(settings->bypassTimeMs*0.9))) && (getGPIOState(bypassPinNum) == 0))
				{
					engineStatus = false;
					timeout = requestCount;
					while (!engineStatus && (timeout-->0))
					{
						if (settings->engine_relay > 0)
						{
							if (settings->debugFlag.EngineWatch)
								printf("[ENG] [%2d:%02d:%02d] ENGINE RELAY TURN OFF [prev: %d; pin: %d]\n",
								((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60,
								getGPIOState(settings->engine_relay), settings->engine_relay);
							setGPIOState(settings->engine_relay, 0);
						}
						engineStatus = engine->engineStart(500);
						delay_ms(50);
					}
					if (timeout == 0)
						if (settings->debugFlag.EngineWatch) printf("[DEBUG] >> [ENG] Timeout [FLAG: 1]\n");
					delay_ms(settings->commonParams.engine_StartStopTimeMs);
				}
				else
				{
					engineStatus = false;
					timeout = requestCount;
					while (!engineStatus && (timeout-->0))
					{
						if (settings->engine_relay > 0)
						{
							if (settings->debugFlag.EngineWatch)
								printf("[ENG] [%2d:%02d:%02d] ENGINE RELAY TURN ON [prev: %d; pin: %d]\n",
								((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60,
								getGPIOState(settings->engine_relay), settings->engine_relay);
							setGPIOState(settings->engine_relay, 1);
						}
						engineStatus = engine->engineStart(engine->needFreq);
						delay_ms(100);
					}
					term_setattr(31);
					if (timeout == 0)
						if (settings->debugFlag.EngineWatch) printf("[DEBUG] >> [ENG] Timeout [FLAG: 2]\n");
					term_setattr(0);
					delay_ms(settings->commonParams.engine_StartStopTimeMs);
				}
			}
		}

		if (settings->busyFlag.EngineWatch > 0)
			settings->busyFlag.EngineWatch--;
		delay_ms(100);
	}
	engine->engineStop();
	engine->CloseDevice();
	printf("[ENG]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
