#include "../main.h"

DBParam_DiscountDate	DiscountDates[MAX_PARAM_ARRAY_ITEM + 1];
DBParam_DiscountParam	DiscountParams[MAX_PARAM_ARRAY_ITEM + 1];
int dateTimeNeedSync = 0;
int loadCashAndEngWorkTimeNeed = 0;
extern int dayLightWork;

PI_THREAD(DiscountWatch)
{
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.TimeTickThread) { 	pthread_detach(pthread_self()); return (void*)0;}
	while (settings->threadFlag.TimeTickThread)
	{
		int indext = 0;
		int dateDiscount = 0;
		int timeDiscount = 0;
		time_t currTime = time(NULL);
  		struct tm* timeInfo;
  		timeInfo = localtime(&currTime);
  		for (indext=1; indext <= MAX_PARAM_ARRAY_ITEM; indext++)
  		{
			if ((timeInfo->tm_mday == DiscountDates[indext].DiscountDay) && (timeInfo->tm_mon+1 == DiscountDates[indext].DiscountMonth))
				if (dateDiscount < DiscountDates[indext].DiscountSize)
					dateDiscount = DiscountDates[indext].DiscountSize;
  		}

  		for (indext=1; indext <= MAX_PARAM_ARRAY_ITEM; indext++)
  		{
			if (
				((DiscountParams[indext].DiscountDayOfWeek >> timeInfo->tm_wday) & 1)
				 && (
			 			((timeInfo->tm_hour*60+timeInfo->tm_min)
			 				>= (DiscountParams[indext].DiscountStartHour*60+DiscountParams[indext].DiscountStartMin)) &&
			 			((timeInfo->tm_hour*60+timeInfo->tm_min)
			 				<= (DiscountParams[indext].DiscountStopHour*60+DiscountParams[indext].DiscountStopMin))
				 	)
				)
				{
					if (timeDiscount < DiscountParams[indext].DiscountSize)
						timeDiscount = DiscountParams[indext].DiscountSize;
				}
  		}

  		if (timeDiscount > dateDiscount)
  			settings->discountSize = timeDiscount;
		else
  			settings->discountSize = dateDiscount;
  		delay(5);
	}
	pthread_detach(pthread_self());
	return (void*)0;
}

PI_THREAD(TurnLightWatch)
{
	lightThreadActive = 1;
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->getEnabledDevice(DVC_RELAY_LIGHT))
		{lightThreadActive = 1; pthread_detach(pthread_self()); return (void*)0;}
	int pinNum = settings->getPinConfig(DVC_RELAY_LIGHT, 1);
	if ((pinNum == 0xFF) || (pinNum == 0x00)) {lightThreadActive = 1; 	pthread_detach(pthread_self()); return (void*)0;}

	int delaySize = settings->LightTimeOff*10;
	printf("[DEBUG] Turn light thread. Wait %d sec\n", (int)(delaySize/10));

	while (delaySize-- > 0)
	{
		if (status.intDeviceInfo.money_currentBalance > 0) { lightThreadActive = 0; 	pthread_detach(pthread_self()); return (void*)0;}
		if (dayLightWork)
			setGPIOState(pinNum, 1);
		delay_ms(100);
	}
	printf("[DEBUG] Turn light thread. Turn off light\n");

	setPinModeMy(pinNum, 0);
	pullUpDnControl (pinNum, PUD_DOWN) ;
	setGPIOState(pinNum, 0);
	lightThreadActive = 0;
	pthread_detach(pthread_self());
	return (void*)0;
}

int thServiceSetupSystemDateTime(Settings* settings)
{
	if (!settings->useDatabase) return 1;

	printf ("   >> Init GLOBAL database ... \n");
	Database* gDb = new Database();
	gDb->Init(&settings->gdatabaseSettings);
	if (gDb->Open())
	{
		printf("  ===> GLOBAL IB ERROR: %s\n", gDb->lastErrorMessage);
		return 0;
	}
	else
	{
		IBPP::Timestamp sysEmptyTime;
		IBPP::Timestamp sysTime;
		if (gDb->Query(DB_QUERY_TYPE_SYSTIME, NULL, &sysTime))
			{ printf("  ===> IB ERROR: %s\n", gDb->lastErrorMessage); return 0;}
		else
		{
			if (sysEmptyTime != sysTime)
			{
				IBPP::Date sysdate = sysTime;
				IBPP::Time systime = sysTime;
				int sy = 0, sm = 0, sd = 0;
				sysdate.GetDate(sy, sm, sd);
				int sh = 0, si = 0, ss = 0;
				systime.GetTime(sh, si, ss);
				char serverDateTime[100];
				sprintf(serverDateTime, "%02d/%02d/%04d %02d:%02d:%02d", sm, sd, sy, sh, si, ss);
				printf("[Database server]: Server time %s\n", serverDateTime);
				sprintf(serverDateTime, "sudo date -s\"%02d/%02d/%04d %02d:%02d:%02d\"", sm, sd, sy, sh, si, ss);
				if ((settings->useDatabaseDateTime)  && (!settings->useHWClock)) {printf("DateTime service: [DEBUG] Setting date and time from Database server"); system(serverDateTime);}
				// Инициализация переменных времени
   				prgStartTimer = time(NULL);							// Время запуска программы
    			winterCurrTime = prgStartTimer;						// Зимний режим, время последней операции для отсчета простоя
			}
			printf("done.");
		}
		gDb->Close();
	}
	return 1;
}

int thServiceSetupCashEngWork(Settings* settings)
{
	///
	/// Инициализируем локальную БД и получаем активную кассу
	///
	int wrkOpened = 0;
	double engFullWorkTime = 0;
	int tryCount = 5;
	if (settings->useDatabase)
	{
		if(commonDb->Open())
		{printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage); return 0;}

		IBPP::Timestamp param1[2];
		IBPP::Timestamp emptyTime;

		printf("  ===> Get opened workspace\n");
		if (commonDb->Query(DB_QUERY_TYPE_GET_OPENED_WORKSPACE, NULL, param1))
			{ printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage); return 0; }

		if (emptyTime != param1[1])
		{
			IBPP::Date mydate = param1[1];
			IBPP::Time mytime = param1[1];
			int y = 0, m = 0, d = 0;
			mydate.GetDate(y, m, d);
			wrkOpened = (y > 2019);
		}
		if (wrkOpened)
		{
			wrkOpenedFlag = 1;
			printf("  ===> Workspace is opened\n");
			IBPP::Date mydate = param1[0];
			IBPP::Time mytime = param1[0];
			int y = 0, m = 0, d = 0;
			int h = 0, i = 0, s = 0;
			mydate.GetDate(y, m, d);
			mytime.GetTime(h, i, s);
			sprintf(status.intDeviceInfo.wrkOpened, "%02d.%02d.%04d %02d:%02d:%02d", d, m, y, h, i, s);
		}
		else
		{
			printf("  ===> Open new workspace\n");
			if (commonDb->Query(DB_QUERY_TYPE_WORKSPACE_OPEN, NULL, NULL))
				{ printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage); return 0; }
			else if (commonDb->Query(DB_QUERY_TYPE_GET_OPENED_WORKSPACE, NULL, param1))
				{ printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage); return 0; }
			else
			{
				wrkOpenedFlag = 1;
				wrkOpened = 1;
				IBPP::Date mydate = param1[0];
				IBPP::Time mytime = param1[0];
				int y = 0, m = 0, d = 0;
				int h = 0, i = 0, s = 0;
				mydate.GetDate(y, m, d);
				mytime.GetTime(h, i, s);
				sprintf(status.intDeviceInfo.wrkOpened, "%02d.%02d.%04d %02d:%02d:%02d", d, m, y, h, i, s);
			}
		}
		double moneyStore = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_CASH_STORE, param1, &moneyStore))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);

		if (!settings->useEeprom)
		{
			status.intDeviceInfo.allMoney = status.intDeviceInfo.allMoney + (DWORD)moneyStore;
			printf("  ===> Load cash store (database): $%d - %s\n", status.intDeviceInfo.allMoney, (char*)&status.intDeviceInfo.wrkOpened[0]);
		}

		if (commonDb->Query(DB_QUERY_TYPE_GET_ENGINE_WORK_TIME, NULL, &engFullWorkTime))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		printf ("  ===> Engine full work time (database): %d sec\n", (unsigned long)engFullWorkTime);
		if (!settings->useEeprom)
			gEngineFullWorkTime = gEngineFullWorkTime + (unsigned long)engFullWorkTime;
	}
	return 1;
}

PI_THREAD(load_params_from_db)
{
	int timeOut = 0;
	int maxIdParamsDB = -214452;
	int indext = 0;
	int indexd = 0;
	Settings* settings = Settings::getInstance();
	commonDb 			= new Database();
	commonDb->Init(settings);
	while (1)
	{
		if (dateTimeNeedSync == 0)
			dateTimeNeedSync = thServiceSetupSystemDateTime(settings);
		if (!settings->useDatabase) {delay_ms(60000); continue; }
		if (commonDb->Open() != DB_OK) {delay_ms(5000); continue; }
		if (!commonDb->IsOpened())
		{
			printf("[DEBUG] Load parameters from db ... Database open error!\n");
			commonDb->Close();
			timeOut = 60;
			while (settings->threadFlag.MainWatch && (timeOut-- > 0))
				delay(1);
			continue;
		}
		if (loadCashAndEngWorkTimeNeed == 0)
			loadCashAndEngWorkTimeNeed = thServiceSetupCashEngWork(settings);

		double qAnswer = -1;
		double qParam[2] = {0, 0};
		double qArray[MAX_PARAM_ARRAY_ITEM + 1];

		if (commonDb->Query(DB_QUERY_TYPE_GET_NEW_PARAM, NULL, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			if (maxIdParamsDB == qAnswer) {sleep(5); continue;}
		maxIdParamsDB = qAnswer;
		printf(" loading params from DB in progress ... \n");
		///
		///----------------------------------
		qParam[0] = DB_PARAM_COMMON_USE_AUTOSTOP;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->useAutoStop = (qAnswer >= 0)?(int)qAnswer:settings->useAutoStop;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_COMMON_COIN_AUTOSTOP;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->countAutoStop = (qAnswer >= 0)?(int)qAnswer:settings->countAutoStop;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_THERMAL_TEMP_ON;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->thermalParam.tempOn = (qAnswer >= 0)?(int)qAnswer:settings->thermalParam.tempOn;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_THERMAL_TEMP_OFF;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->thermalParam.tempOff = (qAnswer >= 0)?(int)qAnswer:settings->thermalParam.tempOff;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_ANTIFROST;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->winterMode.winterMode = (qAnswer >= 0)?(int)qAnswer:settings->winterMode.winterMode;
		for (indext=0; indext < 32; indext++)
		{
			qParam[0] = DB_PARAM_COMMON_PRG_PRICE;
			qParam[1] = indext;
			if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
				printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
			else
				settings->progPrice[indext] = (qAnswer >= 0)?(int)qAnswer:settings->progPrice[indext];
			///-------------------------------
			qParam[0] = DB_PARAM_COMMON_PRG_RPM;
			qParam[1] = indext;
			if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
				printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
			else
				settings->progRPM[indext] = (qAnswer >= 0)?(int)qAnswer:settings->progRPM[indext];
		}
		for (indext=0; indext < MONEY_COIN_TYPE_COUNT; indext++)
		{
			qParam[0] = DB_PARAM_COIN_WEIGHT;
			qParam[1] = indext;
			if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
				printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
			else
				settings->coinWeight.Weight[indext] = (qAnswer >= 0)?(int)qAnswer:settings->coinWeight.Weight[indext];
			///-------------------------------
			qParam[0] = DB_PARAM_BILL_WEIGHT;
			qParam[1] = indext;
			if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
				printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
			else
				settings->moneyWeight.Weight[indext] = (qAnswer >= 0)?(int)qAnswer:settings->moneyWeight.Weight[indext];
		}
		///
		///----------------------------------
		qParam[0] = DB_PARAM_ANTIFROST_DELAY;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->winterMode.winterWaitTime = (qAnswer >= 0)?(int)qAnswer:settings->winterMode.winterWaitTime;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_ANTIFROST_TEMP;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->winterMode.winterOnTemperature = (qAnswer >= 0)?(int)qAnswer:settings->winterMode.winterOnTemperature;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_ANTIFROST_ENGINE;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->winterMode.winterEngineWork = (qAnswer >= 0)?(int)qAnswer:settings->winterMode.winterEngineWork;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_ANTIFROST_ENGINE_TIME;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->winterMode.winterEngineWorkTime = (qAnswer >= 0)?(int)qAnswer:settings->winterMode.winterEngineWorkTime;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_ANTIFROST_ENGINE_FREQ;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->winterMode.winterEngineWorkFreq = (qAnswer >= 0)?(int)qAnswer:settings->winterMode.winterEngineWorkFreq;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_ANTIFROST_ENGINE_WAIT;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->winterMode.winterEngineWorkWait = (qAnswer >= 0)?(int)qAnswer:settings->winterMode.winterEngineWorkWait;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_COMMON_USE_RESET_USB;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->resetUsbDevice = (qAnswer >= 0)?1:0;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_DISCOUNT_CARD_DEPOSIT;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->discountCardDeposit = (qAnswer >= 0)?(unsigned char)qAnswer:0;
		///
		///----------------------------------
		qParam[0] = DB_PARAM_DISCOUNT_DAY_OF_MONTH;
		qParam[1] = MAX_PARAM_ARRAY_ITEM + 1;
		memset(&qArray[0], 0x00, sizeof(qArray));
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM_ARRAY, qParam, &qArray[0]))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
		{
			memset(&DiscountDates[0], 0x00, sizeof(DiscountDates));
			printf ("[DEBUG] Loading discount days: Items count: %f\n", qArray[0]);
			DiscountDates[0].DiscountSize = (unsigned char)qArray[0];
			indexd = 1;
			for(indext = 1; (indext < qArray[0]*2) && (indext <= MAX_PARAM_ARRAY_ITEM); indext+=2)
			{
				DiscountDates[indexd].DiscountSize = ((long)qArray[indext] & 0xFF0000) >> 16;
				DiscountDates[indexd].DiscountDay = ((long)qArray[indext] & 0xFF00) >> 8;
				DiscountDates[indexd].DiscountMonth = ((long)qArray[indext] & 0xFF);
				printf ("                Item: Discount: %3d%% Date: %02d.%02d\n",
					DiscountDates[indexd].DiscountSize, DiscountDates[indexd].DiscountDay,
					DiscountDates[indexd].DiscountMonth);
				indexd++;
			}
		}
		///
		///----------------------------------
		qParam[0] = DB_PARAM_DISCOUNT_PARAM;
		qParam[1] = MAX_PARAM_ARRAY_ITEM + 1;
		memset(&qArray[0], 0x00, sizeof(qArray));
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM_ARRAY, qParam, &qArray[0]))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
		{
			printf ("[DEBUG] Loading discount params: Items count: %f\n", qArray[0]);
			memset(&DiscountParams[0], 0x00, sizeof(DiscountParams));
			DiscountParams[0].DiscountSize = (unsigned char)qArray[0];
			indexd = 1;
			for(int indext = 1; (indext < qArray[0]*2) && (indext <= MAX_PARAM_ARRAY_ITEM); indext+=2)
			{
				DiscountParams[indexd].DiscountDayOfWeek = ((long)qArray[indext] & 0xFF00) >> 8;
				DiscountParams[indexd].DiscountSize = ((long)qArray[indext] & 0xFF);

				DiscountParams[indexd].DiscountStartHour = ((long)qArray[indext+1] & 0xFF000000) >> 24;
				DiscountParams[indexd].DiscountStartMin = ((long)qArray[indext+1] & 0xFF0000) >> 16;
				DiscountParams[indexd].DiscountStopHour = ((long)qArray[indext+1] & 0xFF00) >> 8;
				DiscountParams[indexd].DiscountStopMin = ((long)qArray[indext+1] & 0xFF);
				printf ("                Item: Discount: %d%% Start: %02d:%02d Stop: %02d:%02d, DoW: %d\n",
					DiscountParams[indexd].DiscountSize,
					DiscountParams[indexd].DiscountStartHour, DiscountParams[indexd].DiscountStartMin,
					DiscountParams[indexd].DiscountStopHour, DiscountParams[indexd].DiscountStopMin,
					DiscountParams[indexd].DiscountDayOfWeek);
				indexd++;
			}
		}
		///
		///----------------------------------
		qParam[0] = DB_PARAM_ANTIFROST_ENGINE_WAIT;
		qParam[1] = 0;
		if (commonDb->Query(DB_QUERY_TYPE_GET_PARAM, qParam, &qAnswer))
			printf("  ===> IB ERROR: %s\n", commonDb->lastErrorMessage);
		else
			settings->winterMode.winterEngineWorkWait = (qAnswer >= 0)?(int)qAnswer:settings->winterMode.winterEngineWorkWait;

		if (settings->threadFlag.MainWatch == 0) break;
		printf(" loading params from DB ... done\n");
		sleep(60);
	}
	pthread_detach(pthread_self());
	return (void*)0;
}

PI_THREAD(ClearQueueLog)
{
	int sleeptime = 10000;
	printf ("[DEBUG] QueueLog: Starting thread. Sleep time for system init: %d sec", ((int)(sleeptime/1000)));
	delay_ms(sleeptime);
	QueueType valueLog;
	Settings* settings = Settings::getInstance();
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR [ClearQueueLog]: %s\n", db->lastErrorMessage);

	while(1)
	{
	    int timeout = 100;
		if ((db->IsOpened()) && (queueLog->QueueCount > 0))
		{
			db->Close();
			delay_ms(1000);
			db->Open();
			if (!db->IsOpened()) { delay_ms(60000); continue; }
		    while ((timeout-->0) && (settings->busyFlag.QueueLog)) delay_ms(1);
			while(queueLog->QueueGet(&valueLog) >= 0)
			{
				timeout = 100;
				if (db->LogDb(valueLog.eventTime, valueLog.eventId, valueLog.data1, valueLog.data2, valueLog.note) != DB_OK)
					{ printf("IB ERROR: %s\n", db->lastErrorMessage); break; }
			    while ((timeout-->0) && (settings->busyFlag.QueueLog)) delay_ms(1);
			}
		}
		else
			{ delay_ms(10000); db->Open(); }
		delay_ms(10000);
	}
	pthread_detach(pthread_self());
	return (void*)0;
}

PI_THREAD(gpioext_wd)
{
	if (!settings->threadFlag.GPIOExtWatch) return (void*)0;
	delay_ms(100);
	Settings* settings = Settings::getInstance();
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR [gpioext_wd]: %s\n", db->lastErrorMessage);

	int pinChange = 0;
	int pinReset = 0;
	int pinSens1 = 0;
	int pinSens2 = 0;
	int chechDirection = 0;
	int errorCount = 0;
	int errorShowed = 0;
	char errorNote[] = "[THREAD] GPIO_EXT_WATCHDOG: mcp23017 failed. Reseting device.\x0";
	while(settings->threadFlag.GPIOExtWatch)
	{
		if (!settings->getEnabledDevice(DVC_GPIO_EXTENDER_RESET))
		{delay_ms(10000); return (void*)0;}

		pinReset = settings->getPinConfig(DVC_GPIO_EXTENDER_RESET, 1);
		pinSens1 = settings->getPinConfig(DVC_GPIO_EXTENDER_SENSOR1, 1);
		pinSens2 = settings->getPinConfig(DVC_GPIO_EXTENDER_SENSOR2, 1);

		setPinModeMy(pinReset, 0);
		setGPIOState(pinReset, 1);
		bool result = 1;
		if ((pinReset != 0xFF) && (pinReset != 0x00) &&
			(pinSens1 != 0xFF) && (pinSens1 != 0x00) &&
			(pinSens2 != 0xFF) && (pinSens2 != 0x00))
		{
			if (chechDirection)
			{pinChange = pinSens1; pinSens1 = pinSens2; pinSens2 = pinChange; }
			chechDirection != chechDirection;
			setPinModeMy(pinReset, 0);
			setPinModeMy(pinSens1, 0);
			setPinModeMy(pinSens2, 1);
			///////////
			BYTE state = 0;
			setGPIOState(pinSens1, state);
			delay_ms(25);
			result &= (getGPIOState(pinSens2) == state);
			delay_ms(100);
			state = 1;
			setGPIOState(pinSens1, state);
			delay_ms(25);
			result &= (getGPIOState(pinSens2) == state);
			if (!result)
				errorCount++;
			else
			{
				MCPErrorCount = 0;
				errorCount = 0;
			}

			if (errorCount > 10)
			{
				settings->busyFlag.ButtonWatch++;
				errorCount=11;
				if (errorShowed == 0)
				{
					MCPErrorCount++;
					db->Log(DB_EVENT_TYPE_GPIO_EXTENDER_FAIL, errorCount, errorCount, errorNote);
					printf("[DEBUG] mcp23017 comunication failed! Reseting device.\n");
					errorShowed = 1;
				}
				setGPIOState(pinReset, 0);
				delay_ms(5);
				setGPIOState(pinReset, 1);
				settings->busyFlag.ButtonWatch--;
				continue;
			}
			else
				errorShowed = 0;
		}
		delay_ms(500);
	}
	pthread_detach(pthread_self());
	return (void*)0;
}
