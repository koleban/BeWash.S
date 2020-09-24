#include "../main.h"

/*
	- ��������� ������� ���������������
	TODO: ������� ��������� ������ � ���������� ����������!
*/
PI_THREAD(BV_MoneyWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.MoneyCCTalkWatch) return (void*)0;
	bool casseteRemoved = 0;
	bool billError = 0;
	int billErrorCounter = 0;
	unsigned int counter = 0;
	int timeout = 50;
	CCTalkBillDevice* 	billDevice 	= CCTalkBillDevice::getInstance();

	Database* db = new Database();
	db->Init(settings);
	char myNote[200];
	sprintf(myNote, "[THREAD] Money: Bill validator thread init [%s %s]", billDevice->model, billDevice->serial);
	db->Log( 0, DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote);

	billDevice->Detect();

	while (settings->threadFlag.MoneyCCTalkWatch)
	{
		settings->workFlag.MoneyCCTalkWatch = 0;
		billDevice->OpenDevice();
		if (billDevice->IsOpened())
		{
			if (settings->debugFlag.MoneyCCTalkWatch)
				printf("[DEBUG] BV_MoneyWatch: Device is opened\n");
		
			timeout = 10;
			while ((timeout--) && !billDevice->cmdModifyInhibit())
				{ billDevice->CloseDevice(); delay_ms(100); }
			timeout = 10;
			while ((timeout--) && !billDevice->cmdModifyMasterInhibit())
				{ billDevice->CloseDevice(); delay_ms(100); }
			timeout = 10;
			while ((timeout--) && !billDevice->cmdModifyBillMode())
				{ billDevice->CloseDevice(); delay_ms(100); }

			if (settings->debugFlag.MoneyCCTalkWatch)
				printf("[DEBUG] BV_MoneyWatch: Reset device and set init params [%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
			int t = 0;
			while (settings->threadFlag.MoneyCCTalkWatch)
			{
				settings->workFlag.MoneyCCTalkWatch = 0;
				int timeout = 6;
				while ((settings->busyFlag.MoneyCCTalkWatch) && (timeout--)) {delay_ms(500); continue;}
				if (timeout == 0)
				{
					settings->busyFlag.MoneyCCTalkWatch = 0;
					printf("[DEBUG] BV_MoneyWatch: Timeout detected!\n");
				}
				if (((globalThreadId == 0)  || ((externalCtrl < 4) && (globalThreadId > 0))) && (settings->threadFlag.IntCommonThread == 0)) { delay(1); continue;}
				DWORD result = billDevice->cmdPoll();
				if ((result & 0xFF00) == 0x0C00)
				{
					if (settings->debugFlag.MoneyCCTalkWatch)
						printf("[DEBUG] BV_MoneyWatch: Bill validator DROP CASSETE INSERTED![%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
					casseteRemoved = 0;
					billError = 0;
					billErrorCounter = 0;
				}
				if ((result & 0xFF00) == 0x0B00)
				{
					billErrorCounter = 0;
					if (casseteRemoved == 0)
					{
						if (settings->debugFlag.MoneyCCTalkWatch)
							printf("[DEBUG] BV_MoneyWatch: Bill validator DROP CASSETE REMOVED![%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
						db->Log( 0, DB_EVENT_TYPE_DVC_BILL_WARNING, result, casseteRemoved, "Bill validator DROP CASSETE REMOVED!");
					}
					casseteRemoved = 1;
				}
				if ((result&0xFF00) == 0x0300)
				{
					billErrorCounter = 0;
					if (billError == 0)
						if (settings->debugFlag.MoneyCCTalkWatch)
						{
							db->Log( 0, DB_EVENT_TYPE_DVC_BILL_ERROR, result, 0, "REMOVE SHEET FROM BILL VALIDATOR");
							printf("[DEBUG] BV_MoneyWatch: REMOVE SHEET FROM BILL VALIDATOR!!! 0x(%04X) [%3d:%02d:%02d]\n", result, ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
						}
					billError = 1;
				}
				if ((result&0xFF00) == 0x0200)
				{
					billErrorCounter = 0;
					if (billError == 0)
						if (settings->debugFlag.MoneyCCTalkWatch)
						{
							db->Log( 0, DB_EVENT_TYPE_DVC_BILL_ERROR, result, 0, "UNKNOWN BILL: VALIDATION FAIL");
							printf("[DEBUG] BV_MoneyWatch: UNKNOWN BILL: VALIDATION FAIL 0x(%04X) [%3d:%02d:%02d]\n", result, ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
						}
					billError = 1;
				}
				if (result == 0xFFFF)
				{
					db->Log( 0, DB_EVENT_TYPE_DVC_BILL_ERROR, result, 0, "Bill validator connection error!");
					if (billErrorCounter++ < 3) continue;
					else
					{
						printf("BILL ERROR \n");
						db->Log( 0, DB_EVENT_TYPE_DVC_BILL_ERROR, 9, 0, "BILL READ ERROR");
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
						db->Log( 0, DB_EVENT_TYPE_EXT_MONEY_EVENT, d1, d2, "MONEY INCOME");
					}
				}
				delay_ms(200);
			}
		}
		db->Log( 0, DB_EVENT_TYPE_DVC_CLOSE, 0, 0, (char*)((billDevice->IsOpened())?"Bill validator device closed":"BV port incorrect. Close port."));
		billDevice->CloseDevice();
		int tmp = 0;
		while (tmp++ < 5) { settings->workFlag.MoneyCCTalkWatch = 0; delay(1);}
		if (settings->threadFlag.MoneyCCTalkWatch)
			billDevice->Detect();
	}
	if (db->Close())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	printf("[BILL]: Thread ended.\n");
	return (void*)0;
}
