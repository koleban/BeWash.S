#include "../main.h"

/*
	- Обработка событий купюроприемника
	TODO: Сделать обработку ошибок и перезапуск устройства!
*/
PI_THREAD(MoneyWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.MoneyWatch) return (void*)0;
	bool casseteRemoved = 0;
	bool billError = 0;
	int billErrorCounter = 0;
	int errorDisplay = 0;
	unsigned int counter = 0;
	CCBillDevice* 	billDevice 	= CCBillDevice::getInstance();

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[200];
	sprintf(myNote, "[THREAD] Money: CashCode bill validator thread init [%s %s %04X%04X]", billDevice->model, billDevice->serial, billDevice->cashCodeID.digId[0], billDevice->cashCodeID.digId[1]);
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	while (settings->threadFlag.MoneyWatch)
	{
		settings->workFlag.MoneyWatch = 0;
		billDevice->OpenDevice();
		if (billDevice->IsOpened())
		{
			if (settings->debugFlag.MoneyWatch)
				printf("[DEBUG] MoneyWatch: Device if opened\n");
			if (billErrorCounter == 0)
				billDevice->cmdReset();
			billDevice->cmdGetStatus();
			billDevice->cmdSetAcceptBillType(0xFF, 0xFF, 0xFF);
			if (settings->debugFlag.MoneyWatch)
				printf("[DEBUG] MoneyWatch: Reset device and set init params [%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
			int t = 0;
			while (settings->threadFlag.MoneyWatch)
			{
				settings->workFlag.MoneyWatch = 0;
				int timeout = 6;
				while ((settings->busyFlag.MoneyWatch) && (timeout--)) {delay_ms(500); continue;}
				if (timeout == 0)
				{
					settings->busyFlag.MoneyWatch = 0;
					printf("[DEBUG] MoneyWatch: Timeout detected!\n");
				}
				// koleban
				// >>>>
				//if (status.extDeviceInfo.bill_Present) continue;
				// <<<<
				if (((globalThreadId == 0)  || ((externalCtrl < 4) && (globalThreadId > 0))) && (settings->threadFlag.IntCommonThread == 0)) { delay(1); continue;}
				DWORD result = billDevice->cmdPoll();
				if ((result&0xFF00) == 0x8100)
				{
					casseteRemoved = 0;
					billError = 0;
					billDevice->cmdASK();
					billErrorCounter = 0;
				}
				if ((result&0xFF00) == 0x4200)
				{
					billErrorCounter = 0;
					if (casseteRemoved == 0)
					{
						if (settings->debugFlag.MoneyWatch)
						{
							printf("[DEBUG] MoneyWatch: Bill validator DROP CASSETE REMOVED![%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
						}
						db->Log(DB_EVENT_TYPE_DVC_BILL_WARNING, result, casseteRemoved, "Bill validator DROP CASSETE REMOVED!");
					}
					casseteRemoved = 1;
				}
				if ((result&0xFF00) == 0x1C00)
				{
					billErrorCounter = 0;
					if (billError == 0)
						if (settings->debugFlag.MoneyWatch)
						{
							db->Log(DB_EVENT_TYPE_DVC_BILL_ERROR, result, 0, "REMOVE SHEET FROM BILL VALIDATOR");
							printf("[DEBUG] MoneyWatch: REMOVE SHEET FROM BILL VALIDATOR!!! (%02X) [%3d:%02d:%02d]\n", result&0xFF, ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
						}
					billError = 1;
				}
				if ((result&0xFF00) == 0x1900)
				{
					billErrorCounter = 0;
					if (casseteRemoved)
					{
						casseteRemoved = 0;
						billDevice->cmdDispenseBill();
					}
					billDevice->cmdGetStatus();
					billDevice->cmdSetAcceptBillType(0xFF, 0xFF, 0xFF);
					db->Log(DB_EVENT_TYPE_DVC_BILL_ERROR, result, 0, "Bill validator RESET!");
				}
				if (result == 0xFFFF)
				{
					db->Log(DB_EVENT_TYPE_DVC_BILL_ERROR, result, 0, "Bill validator RESET!");
					if (billErrorCounter++ < 3) continue;
					else
					{
						printf("BILL ERROR \n");
						db->Log(DB_EVENT_TYPE_DVC_BILL_ERROR, 9, 0, "BILL READ ERROR");
						break;
					}
				}
				if (result == 0xFF00)
				{
					db->Log(DB_EVENT_TYPE_DVC_BILL_ERROR, result, 0, "Bill validator RESET!");
					if (billErrorCounter++ < 2)	continue;
					else
					{
						char usbPath[25];
						for (int usbBus=1; usbBus < 3; usbBus++)
							for (int usbDev=1; usbDev < 3; usbDev++)
							{
								sprintf(usbPath, "/dev/bus/usb/%03d/%03d", usbBus, usbDev);
								usbreset(usbPath);
							}
						db->Log(DB_EVENT_TYPE_DVC_BILL_ERROR, 99, 0, "BILL MISSING");
						printf("BILL MISSING \n");
						delay(3);
						break;
					}
				}

				for (int index=1; index<MONEY_COIN_TYPE_COUNT; index++)
				{
					if (billDevice->moneyCoinInfo.Count[index] != 0)
					{
						printf("[DEBUG] Bill income (%d): %d [%d RUR]\n", index, billDevice->moneyCoinInfo.Count[index], settings->moneyWeight.Weight[index]);
						status.extDeviceInfo.bill_Present = 1;
						status.extDeviceInfo.bill_incomeInfo.Count[index] += billDevice->moneyCoinInfo.Count[index];
						int billCount = billDevice->moneyCoinInfo.Count[index];
						billDevice->moneyCoinInfo.Count[index] = 0;
						double d1 = (double)billCount;
						double d2 = (double)settings->moneyWeight.Weight[index];
						db->Log(DB_EVENT_TYPE_EXT_MONEY_EVENT, d1, d2, "MONEY INCOME");
					}
				}
				delay_ms(200);
			}
		}
		db->Log(DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "Bill validator device closed");
		billDevice->CloseDevice();
		int tmp = 0;
		while (tmp++ < 5) { settings->workFlag.MoneyWatch = 0; delay(1);}
		if (settings->threadFlag.MoneyWatch)
		{
			if (!errorDisplay) {errorDisplay = 1; settings->intErrorCode.MainWatch = 219;}
			billDevice->Detect();
		}
	}
	if (db->Close())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	printf("[BILL]: Thread ended.\n");
	return (void*)0;
}
