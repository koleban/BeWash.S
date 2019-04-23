#include "../main.h"

PI_THREAD(OsmosWatch)
{
	/// Общие параметры
	Settings* settings = Settings::getInstance();
	/// Если ПОТОК запрещен, то завершаемся
	if (!(settings->threadFlag.OsmosThread)) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	db->Log(DB_EVENT_TYPE_THREAD_INIT, 		0, 0, "[THREAD] OSMOS: OSMOS thread init");
	db->Log(DB_EVENT_TYPE_DVC_BUTTON_INIT, 	0, 0, "OSMOS device opened");

	if (settings->debugFlag.OsmosThread)
		printf("[DEBUG] OsmosWatch: Debug information is showed\n");

	int index;
	BYTE currentPin = 0;
	BYTE currentDeviceID = 0;
	BYTE bankFull = 0;
	BYTE bankFullPrev = 0xFF;
	int prevPrg = 2;
	time_t prg_timer;
	time(&prg_timer);

	///
	/// Установим первоначальные параметры кнопок
	/// (0 - 12 кнопку)
	///
	for (index = 0; index < 12; index++)
	{
		currentDeviceID = DVC_BUTTON01 + index;
		if (!settings->getEnabledDevice(currentDeviceID))
			continue;
		currentPin = settings->getPinConfig(currentDeviceID, 1);
		if (currentPin == 0xFF) continue;
		setGPIOState(currentPin, 0);
		setPinModeMy(currentPin, PIN_INPUT);
  		pullUpDnControl (currentPin, PUD_DOWN) ;
	}

	if (settings->debugFlag.OsmosThread)
		printf("[DEBUG] OsmosThread: OSMOS subsystem init  [%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);

/*
	status.extDeviceInfo.button_newEvent = 1;
	status.extDeviceInfo.button_lastEvent = 1;
	status.extDeviceInfo.button_currentLight = 2;
	status.intDeviceInfo.program_currentProgram = 1;
	status.intDeviceInfo.extPrgNeedUpdate = 1;
*/

	int currentLight = status.intDeviceInfo.program_currentProgram;
	if ((settings->getEnabledDevice(DVC_BUTTON01 + currentLight)) && (settings->getPinConfig(DVC_BUTTON01 + currentLight, 1) != 0xFF))
	{
		currentPin = settings->getPinConfig(currentDeviceID, 1);
		setPinModeMy(DVC_BUTTON01 + currentLight, PIN_OUTPUT);
		setGPIOState(currentPin, 1);
	}

	while (settings->threadFlag.OsmosThread)
	{
		int delayTime = 100;
		settings->workFlag.OsmosThread = 0;
		status.intDeviceInfo.money_currentBalance = 100;

		///
		/// Проверяем все кнопки на нажатие
		///


		for (index = 0; index < 12; index++)
		{
			currentDeviceID = DVC_BUTTON01 + index;

			if (currentLight == index) continue;
			if ((status.intDeviceInfo.program_currentProgram == 5) && (index == 2)) continue;
			if (!settings->getEnabledDevice(currentDeviceID)) continue;
			currentPin = settings->getPinConfig(currentDeviceID, 1);
			if (currentPin == 0xFF) continue;

			setPinModeMy(currentPin, PIN_INPUT);
			if (getGPIOState(currentPin))
			{
				int timeout = 50;
				if (settings->debugFlag.OsmosThread)
					printf("[DEBUG] OsmosThread: Pressed state on %d button [PIN: %03d]\n", index, currentPin);
				while((timeout-- > 0) && getGPIOState(currentPin)) { delayTime--; delay_ms(1); }
				if (timeout <= 0)
				{

					///
					/// BUTON PRESSED OK
					///
					currentLight = index;
					setPinModeMy(currentPin, PIN_OUTPUT);
					setGPIOState(currentPin, 1);
					if ((status.intDeviceInfo.program_currentProgram == 5) && (index == 2)) break;
					if ((index == 2) && (bankFull)) break;

					status.extDeviceInfo.button_newEvent = index;
					status.extDeviceInfo.button_lastEvent = index;
					status.extDeviceInfo.button_currentLight = index+1;
					status.intDeviceInfo.program_currentProgram = index;
					status.intDeviceInfo.extPrgNeedUpdate = 1;

					delay_ms(100);
					break;
				}
				else
				{
					if (settings->debugFlag.OsmosThread)
						printf("[DEBUG] OsmosThread: Pressed state on %d button (50 ms) [PIN: %03d] - failed [%d ms]\n", index, currentPin, 50 - timeout);
					db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, index, 50 - timeout, "[OsmosThread]: Button don't detected. Failed");
				}
			}
		}

		time_t rcv_timer_out;
		time(&rcv_timer_out);
		if (settings->getEnabledDevice(DVC_OSMOS_BANK_FULL))
		{
			int pinCfg = settings->getPinConfig(DVC_OSMOS_BANK_FULL, 1);
			if (pinCfg != 0xFF)
				bankFull = (getGPIOState(pinCfg) == 0);
		}

		switch (status.intDeviceInfo.program_currentProgram)
		{
			// STOP
			case 0:
			default:
				if (bankFullPrev != bankFull)
				{
					if (!bankFull)
						printf("[OsmosThread]: (prg:STOP0) BANK EMPTY\n");
					bankFullPrev = bankFull;
					time(&prg_timer);
				}
				if (!bankFull)
				{
					if (((DWORD)rcv_timer_out - (DWORD)prg_timer) > settings->osmosParam.waitOnEmpty)
					{
						printf("[OsmosThread]: (prg:STOP0) BANK EMPTY LOAD (ADD LOAD TIME: %d sec)\n", settings->osmosParam.waitOnEmpty);
						status.extDeviceInfo.button_newEvent = 2;
						status.extDeviceInfo.button_lastEvent = 2;
						status.extDeviceInfo.button_currentLight = 3;
						status.intDeviceInfo.program_currentProgram = 2;
						status.intDeviceInfo.extPrgNeedUpdate = 1;
						time(&prg_timer);
						break;
					}
				}
				if (prevPrg == status.intDeviceInfo.program_currentProgram) break;
				prevPrg = status.intDeviceInfo.program_currentProgram;
				printf("[OsmosThread]: STOP\n");
				time(&prg_timer);
				break;
			case 1:
				delay_ms(100);
				break;
			// РАБОТА
			case 2:
				// КОНТРОЛЬ ДАТЧИКОВ
				//
				if (bankFullPrev != bankFull)
				{
					if (bankFull)
						printf("[OsmosThread]: BANK FULL\n");
					bankFullPrev = bankFull;
					time(&prg_timer);
				}
				if (bankFull)
				{
					if (((DWORD)rcv_timer_out - (DWORD)prg_timer) > settings->osmosParam.waitOnFull)
					{
						printf("[OsmosThread]: BANK FULL LOAD (ADD LOAD TIME: %d sec)\n", settings->osmosParam.waitOnFull);
						status.extDeviceInfo.button_newEvent = 6;
						status.extDeviceInfo.button_lastEvent = 6;
						status.extDeviceInfo.button_currentLight = 1;
						status.intDeviceInfo.program_currentProgram = 6;
						status.intDeviceInfo.extPrgNeedUpdate = 1;
						time(&prg_timer);
						break;
					}
				}
				if ((!bankFull) && ((DWORD)rcv_timer_out - (DWORD)prg_timer) > settings->osmosParam.gidrodynWaitTime)
				{
					status.extDeviceInfo.button_newEvent = 5;
					status.extDeviceInfo.button_lastEvent = 5;
					status.extDeviceInfo.button_currentLight = 6;
					status.intDeviceInfo.program_currentProgram = 5;
					status.intDeviceInfo.extPrgNeedUpdate = 1;
					time(&prg_timer);
					break;
				}
				if (prevPrg == status.intDeviceInfo.program_currentProgram) break;
				prevPrg = status.intDeviceInfo.program_currentProgram;
				printf("[OsmosThread]: WORK\n");
				time(&prg_timer);
				break;
			// РЕГЕНЕРАЦИЯ
			case 3:
				// КОНТРОЛЬ ДАТЧИКОВ
				//
				if (prevPrg == status.intDeviceInfo.program_currentProgram) break;
				prevPrg = status.intDeviceInfo.program_currentProgram;
				printf("[OsmosThread]: REGENERATION\n");
				time(&prg_timer);
				break;
			// СЛИВ
			case 4:
				// КОНТРОЛЬ ДАТЧИКОВ
				//
				if (prevPrg == status.intDeviceInfo.program_currentProgram) break;
				prevPrg = status.intDeviceInfo.program_currentProgram;
				printf("[OsmosThread]: OUT WATER\n");
				time(&prg_timer);
				break;
			// ГИДРОДИНАМИЧЕСКАЯ ПРОМЫВКА
			case 5:
				// КОНТРОЛЬ ДАТЧИКОВ
				//
				if (((DWORD)rcv_timer_out - (DWORD)prg_timer) > settings->osmosParam.gidrodynTurnOnTime)
				{
					if (status.extDeviceInfo.button_newEvent != 5) break;
					status.extDeviceInfo.button_newEvent = 2;
					status.extDeviceInfo.button_lastEvent = 2;
					status.extDeviceInfo.button_currentLight = 3;
					status.intDeviceInfo.program_currentProgram = 2;
					status.intDeviceInfo.extPrgNeedUpdate = 1;
					time(&prg_timer);
					break;
				}
				delay_ms(100);
				if (prevPrg == status.intDeviceInfo.program_currentProgram) break;
				prevPrg = status.intDeviceInfo.program_currentProgram;
				printf("[OsmosThread]: GIDRODINAMYC OUT WATER\n");
				time(&prg_timer);
				break;
			// ЗАПОЛНЕНИЕ ПЕРМИАТОМ
			case 6:
				if (((DWORD)rcv_timer_out - (DWORD)prg_timer) > settings->osmosParam.permiatLoadTime)
				{
					printf("[OsmosThread]: INPUT PERMIAT - OK\n");
					status.extDeviceInfo.button_newEvent = 0;
					status.extDeviceInfo.button_lastEvent = 0;
					status.extDeviceInfo.button_currentLight = 1;
					status.intDeviceInfo.program_currentProgram = 0;
					status.intDeviceInfo.extPrgNeedUpdate = 1;
					time(&prg_timer);
					break;
				}

				if (prevPrg == status.intDeviceInfo.program_currentProgram) break;
				prevPrg = status.intDeviceInfo.program_currentProgram;
				printf("[OsmosThread]: INPUT PERMIAT (%d sec)\n", settings->osmosParam.permiatLoadTime);
				fflush(stdout);
				// permiatLoadTime
				time(&prg_timer);
				break;
		}

		if (delayTime > 0) delay_ms(delayTime);
	}

	///
	/// Button first setup
	for (index = 0; index < 12; index++)
	{
		currentDeviceID = DVC_BUTTON01 + index;
		if (!settings->getEnabledDevice(currentDeviceID))
			continue;
		currentPin = settings->getPinConfig(currentDeviceID, 1);
		if (currentPin == 0xFF) continue;
		setPinModeMy(currentPin, PIN_OUTPUT);
		setGPIOState(currentPin, 1);
		delay_ms(50);
		setGPIOState(currentPin, 0);
		setPinModeMy(currentPin, PIN_INPUT);
  		pullUpDnControl (currentPin, PUD_DOWN) ;
	}

	db->Log(DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "OSMOS device is closed");
	db->Close();
	printf("[DEBUG]: OsmosThread: Thread is terminate.\n");
	return (void*)0;
}
