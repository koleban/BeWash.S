// !!! TODO: Обработка инкасации. Проверить откуда беруться деньги

#include "../main.h"

//#pragma region Подситема "ОБМЕН ДАННЫМИ"
PI_THREAD(DataExchangeThread)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.DataExchangeThread) return (void*)0;
	int lastCurrentProgram =  -1;
	int lastButtonEvent = 0;
	int lastCollectionButton = 0;
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] Data: Data exchange thread init";
	if (db->Log( 0, DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	while (settings->threadFlag.DataExchangeThread)
	{
		settings->workFlag.DataExchangeThread = 0;
		//
		// Обработка общего обмена данными для "Внутреннего бокса"
		//
		if (settings->threadFlag.IntCommonThread)
		{
			//
			// Нажали кнопку
			//
			if ((lastButtonEvent != status.extDeviceInfo.button_lastEvent) && (status.extDeviceInfo.button_lastEvent >= 0))
			{
				if (settings->debugFlag.DataExchangeThread)
					printf("[DEBUG] DataExchangeThread: New program %d --> %d\n", status.intDeviceInfo.program_currentProgram, status.extDeviceInfo.button_lastEvent);
				if (status.extDeviceInfo.button_lastEvent < 16)
					if (settings->progEnabled[status.extDeviceInfo.button_lastEvent])
						status.intDeviceInfo.program_currentProgram = status.extDeviceInfo.button_lastEvent;

				if (status.extDeviceInfo.button_lastEvent > 250)
					status.intDeviceInfo.program_currentProgram = 0;

				lastButtonEvent = status.extDeviceInfo.button_lastEvent;
			}
			//
			// Изменилась программа на внутреннем боксе
			//
			if (lastCurrentProgram != status.intDeviceInfo.program_currentProgram)
			{
				//
				int u = 0;
				char noteBuffer[1024];
				sprintf(noteBuffer, "Prg change [BAL: %2d, BTN: %2d, CRD: %08X]", status.intDeviceInfo.money_currentBalance, status.extDeviceInfo.button_lastEvent, getCardIDFromBytes(status.extDeviceInfo.rfid_incomeCardNumber));
				db->Log( 0, DB_EVENT_TYPE_INT_PROGRAM_CHANGED, lastCurrentProgram, status.intDeviceInfo.program_currentProgram, noteBuffer);
				lastCurrentProgram = status.intDeviceInfo.program_currentProgram;
			}

			///
			/// Нажата кнопка инкасации (ЗАКРОЕМ СМЕНУ) ИЛИ (ОБРАБОТАЕМ ОБЩИЙ СЧЕТЧИК)
			///
			if (settings->CollectionMode == 0)
			{
				if (status.extDeviceInfo.collectionButton && !lastCollectionButton)
				{
					lastCollectionButton = status.extDeviceInfo.collectionButton;
					printf("  ===> Close workspace (%d)\n", status.intDeviceInfo.allMoney);
					if (settings->useDatabase)
					{
						IBPP::Timestamp param1[2];
						IBPP::Timestamp emptyTime;

						printf("  ===> Get opened workspace\n");
						if (db->Query(DB_QUERY_TYPE_GET_OPENED_WORKSPACE, NULL, param1))
							printf("  ===> IB ERROR: %s\n", db->lastErrorMessage);

						double moneyStore[2];
						moneyStore[1] = status.intDeviceInfo.allMoney;
						if (db->Query(DB_QUERY_TYPE_GET_CASH_STORE, param1, &moneyStore[0]))
							printf("  ===> IB ERROR: %s\n", db->lastErrorMessage);
						status.intDeviceInfo.allMoney = (DWORD)moneyStore[0];

						if (db->Query(DB_QUERY_TYPE_WORKSPACE_CLOSE, &moneyStore[0], NULL))
							printf("  ===> IB ERROR: %s\n", db->lastErrorMessage);
						else
							printf("  ===> done\n");
					}
					wrkOpenedFlag = 0;
				}
					///
				/// Отжата кнопка инкасации (ОТКРОЕМ НОВУЮ СМЕНУ)
				///
				if (!status.extDeviceInfo.collectionButton && !wrkOpenedFlag)
				{
					lastCollectionButton = status.extDeviceInfo.collectionButton;
					printf("  ===> Open new workspace\n");
					if (settings->useDatabase)
					{
						IBPP::Timestamp param1[2];
						IBPP::Timestamp emptyTime;

						if (commonDb->Query(DB_QUERY_TYPE_WORKSPACE_OPEN, NULL, NULL))
							printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);

						if (commonDb->Query(DB_QUERY_TYPE_GET_OPENED_WORKSPACE, NULL, param1))
							printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);

						printf("  ===> Workspace is opened\n");
						IBPP::Date mydate = param1[0];
						IBPP::Time mytime = param1[0];
						int y = 0, m = 0, d = 0;
						int h = 0, i = 0, s = 0;
						mydate.GetDate(y, m, d);
						mytime.GetTime(h, i, s);
						sprintf(status.intDeviceInfo.wrkOpened, "%02d.%02d.%04d %02d:%02d:%02d", d, m, y, h, i, s);

						time(&wrkOpenedDateTime);
						struct tm localTime = *localtime(&wrkOpenedDateTime);
						sprintf(status.intDeviceInfo.wrkOpened, "%02d.%02d.%02d %02d:%02d    ",
						localTime.tm_mday, localTime.tm_mon+1, localTime.tm_year-100,
						localTime.tm_hour, localTime.tm_min);
					}
					else
					{
						time(&wrkOpenedDateTime);
						struct tm localTime = *localtime(&wrkOpenedDateTime);
						sprintf(status.intDeviceInfo.wrkOpened, "%02d.%02d.%02d %02d:%02d    ",
						localTime.tm_mday, localTime.tm_mon+1, localTime.tm_year-100,
						localTime.tm_hour, localTime.tm_min);
					}
					wrkOpenedFlag = 1;
					status.intDeviceInfo.allMoney = 0;
				}
			}

		}

		//
		// Обработка общего обмена данными для "Внешнего бокса"
		//
		if (settings->threadFlag.ExtCommonThread)
		{
			if (status.extDeviceInfo.remote_currentBalance != status.intDeviceInfo.money_currentBalance)
			{
				status.extDeviceInfo.remote_currentBalance = status.intDeviceInfo.money_currentBalance;
				if (status.extDeviceInfo.remote_currentBalance < 0)
				{
					status.intDeviceInfo.money_currentBalance = 0;
					status.extDeviceInfo.remote_currentBalance = 0;
				}
			}
		}

		//
		// Автоматическая перезагрузка по времени
		//
		if ((settings->autoRebootParam.AutoReboot) && (status.intDeviceInfo.money_currentBalance == 0))
		{
			time_t arTime;
			time(&arTime);
			struct tm localTime = *localtime(&arTime);
			struct sysinfo info;
			sysinfo(&info);
			if ((info.uptime >= 3600) && (settings->autoRebootParam.AutoRebootHour == localTime.tm_hour))
			{
				system("sudo reboot now");
			}
		}

		delay_ms(2000);
	}
	db->Close();
	printf("[EXCHANGE]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
