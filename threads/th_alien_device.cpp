#include "../main.h"

//
// Поток отвечате за контроль данных полученых с устройств слежения за "ЧУЖИМИ" устройствами
// и выполняет формирование Чеков для ККМ с учетом параметров ожидания и прочего ...
// *************************
// Работает в связке с 	RemoteCounterCtrlWatch
//						KKMWatch
//

PI_THREAD(AlienDeviceWatch)
{
	/// Общие параметры
	Settings* settings = Settings::getInstance();
	/// Если ПОТОК запрещен, то завершаемся
	if (!(settings->threadFlag.AlienDeviceThread)) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	db->Log(DB_EVENT_TYPE_THREAD_INIT, 		0, 0, "[THREAD] Alien device control: AlienDevice thread init");

	if (settings->debugFlag.AlienDeviceThread)
		printf("[DEBUG] AlienDeviceThread: Debug information is showed\n");

	if (!settings->debugFlag.AlienDeviceThread)
	{
		delay_ms(2000);
		term_clear();
		printf("******************************************************\n");
		printf("*       ");
		term_setattr(32);
		printf("beWash: Online-KKM service system v1.0");
		term_setattr(37);
		printf("       *\n");
		printf("*----------------------------------------------------*\n");
		printf("*   Serg V Kolebanov 2007-2019 All rights reserved   *\n");
		printf("*----------------------------------------------------*\n");
		printf("*       ");
		term_setattr(32);
		printf("   www.bewash.ru mail:dev@bewash.ru   ");
		term_setattr(37);
		printf("       *\n");
		printf("******************************************************\n\n\n");
	}

	while (settings->threadFlag.AlienDeviceThread)
	{
		int delayTime = 100;
		settings->workFlag.AlienDeviceThread = 0;

		for (int index = 1; index <= settings->modbus.slaveCount; index++)
		{
			delay_ms(1000);
			int slaveId = index;

			if ((remoteCounterSumm[index][0] + remoteCounterSumm[index][1]) > 0)
			{
				time_t rcv_timer_out;
				time(&rcv_timer_out);

				if ((((DWORD)rcv_timer_out) - remoteCounterSumm[index][2]) > settings->remoteCounterParam.DocumentCreationTime)
				{
					// Add information for print KKM documents
					// queueKkm->QueuePut( CashSumm, DON'T USED, DON'T USED, ServiceName);
					//
					DWORD sendedBal = remoteCounterSumm[index][0]*settings->remoteCounterParam.PriceIN1 + remoteCounterSumm[index][1]*settings->remoteCounterParam.PriceIN2;
					remoteCounterSumm[index][0] = 0;
					remoteCounterSumm[index][1] = 0;
					char serviceNote[256];
					sprintf(serviceNote, "%s (П:%d)", settings->kkmParam.ServiceName, slaveId);
					term_setattr(32);
					printf ("[ ++ ] ");
					term_setattr(37);
					printf("Создан фискальный документ. ");
					term_setattr(32);
					printf("Пост №: %2d на %3d руб.\n", slaveId, sendedBal);
					term_setattr(37);
					queueKkm->QueuePut(sendedBal, 0, 0, serviceNote);
				}
			}

			// Add to structure remoteCounterCtrl information for send to slave ctrl other modbus protocole
			//
			// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			int retryCounter = 3;
			remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
			int timeout = 1000;
			while ((remoteCtrl[slaveId].cmdResult > 0xFFFF) && (retryCounter-- > 0))
			{
				if (settings->debugFlag.AlienDeviceThread)
					printf ("          [1] READ COMMAND COUNTER 1\n");
				remoteCtrl[slaveId].cmdRead = 1;
				remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
				remoteCtrl[slaveId].doCmd = 1;
				while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
			}
			if (settings->debugFlag.AlienDeviceThread)
				printf ("          [1] COMMAND RESULT: %08X (timeout: %d) Counter [1 rur, 10 rur]: [%d, %d]\n", remoteCtrl[slaveId].cmdResult, timeout, remoteCounter[index][0], remoteCounter[index][1]);
			if ((remoteCtrl[slaveId].cmdResult > 0xFFFFUL) || (timeout < 0)) continue;

			// If we received ZERO in answer. Continue.
			if ((remoteCounter[index][0] + remoteCounter[index][1]) == 0) { delay_ms(100); continue;}


			DWORD incomeMoney = remoteCounter[index][0]*settings->remoteCounterParam.PriceIN1 + remoteCounter[index][1]*settings->remoteCounterParam.PriceIN2;
			if (incomeMoney < settings->kkmParam.MaxAmount)
			{
				term_setattr(36);
				printf ("[ => ] ");
				term_setattr(37);
				printf ("Зафиксированы поступление данных: ");
				term_setattr(36);
				printf("Пост N:%d ", slaveId);
				term_setattr(37);
				printf("Счетчики [%d rur, %d rur]: [%d, %d] = ", settings->remoteCounterParam.PriceIN1, settings->remoteCounterParam.PriceIN2, remoteCounter[index][0], remoteCounter[index][1]);
				term_setattr(36);
				printf("%d руб.\n", incomeMoney);
				term_setattr(37);
				delay_ms(500);
			}
			//
			// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			if (settings->debugFlag.AlienDeviceThread)
				printf ("              WRITE COMMAND COUNTER\n");
			remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
			remoteCtrl[slaveId].cmdWrite = 1;
			remoteCtrl[slaveId].devImpVal[0] = remoteCounter[index][0];	// 1 rur
			remoteCtrl[slaveId].devImpVal[1] = remoteCounter[index][1];	// 10  rur
			remoteCtrl[slaveId].doCmd = 1;
			timeout = 1000;
			while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
			if (settings->debugFlag.AlienDeviceThread)
				printf ("              COMMAND RESULT: %08X (timeout: %d)\n", remoteCtrl[slaveId].cmdResult, timeout);
			if ((remoteCtrl[slaveId].doCmd != 0) || (remoteCtrl[slaveId].cmdResult > 0xFFFF) || (timeout < 0))
			{
				if (settings->debugFlag.AlienDeviceThread)
					printf ("              WRITE COMMAND ERROR\n");
			}
			else
			{
				time_t rcv_timer;
				time(&rcv_timer);
				if (incomeMoney < settings->kkmParam.MaxAmount)
				{
					remoteCounterSumm[index][2] = (DWORD)rcv_timer;
					remoteCounterSumm[index][0] += remoteCounter[index][0];
					remoteCounterSumm[index][1] += remoteCounter[index][1];
					remoteCounter[index][0] = 0;
					remoteCounter[index][1] = 0;
					if (settings->debugFlag.AlienDeviceThread)
						printf ("              WRITE COMMAND OK\n");
					delay_ms(100);
				}
			}
		}

		delay_ms(2000);

	}
	return (void*)0;
}
