#include "../main.h"

//#pragma region Обработка событий монетоприемника
/*
	- Обработка событий монетоприемника
	TODO: Сделать обработку ошибок и перезапуск устройства!
*/
PI_THREAD(CoinWatch)
{
	if (!settings->threadFlag.CoinWatch) return (void*)0;
	Settings* settings = Settings::getInstance();
	unsigned int counter = 0;
	int errorDisplay = 0;

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] Money: CCTalk coin acceptor thread init";
	if (db->Log( 0, DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	int retryCount = 0;
	CoinDevice* 	coinDevice1 	= CoinDevice::getInstance();
	while (settings->threadFlag.CoinWatch)
	{
		settings->workFlag.CoinWatch = 0;
		coinDevice1->OpenDevice();
		if ((coinDevice1->IsOpened()) && (coinDevice1->cmdReset() == 1))
		{
			db->Log( 0, DB_EVENT_TYPE_DVC_COIN_INIT, 0, 0, "Coin acceptor device opened");
			printf("[DEBUG] Coin acceptor device opened (%d)\n", retryCount);
			coinDevice1->cmdReset();
			coinDevice1->cmdSiplePoll();
			coinDevice1->cmdSetMasterInhibitStatus();
			coinDevice1->cmdSiplePoll();
			coinDevice1->cmdModifyInhibitStatus(0xFF, 0xFF);
			for(BYTE coinIndex=1; coinIndex < 15; coinIndex++)
			{
				delay_ms(50);
				coinDevice1->cmdSetCoinSortPath(coinIndex, 0);
			}
			DWORD eventCount = 0;
			int oldEventCount = 0;
			BYTE bufData[10];
			retryCount = 0;
			while (settings->threadFlag.CoinWatch)
			{
				int timeOut = 10;
				int delayTime = 100;
				settings->workFlag.CoinWatch = 0;
				while(settings->busyFlag.CoinWatch) {delay_ms(delayTime); if (timeOut--) break; continue;}
				if (retryCount > 10) { retryCount = 0; break;}
				if (((globalThreadId == 0)  || ((externalCtrl < 4) && (globalThreadId > 0))) && (settings->threadFlag.IntCommonThread == 0)) { delay(1); continue;}
				eventCount = coinDevice1->cmdReadEventBuffer(bufData);
				if ((eventCount&0xAAAA0000))
				{
					char messageText[250];
					rejectedCoinInfo.coinRejected = 0;
					sprintf(messageText, "WARNING: COIN DOUBLE INCOME");
					db->Log( 0, DB_EVENT_TYPE_EXT_COIN_REJECTED, 999, 999, messageText);
				}
				if (eventCount == 0xFFFF) { retryCount++; delay_ms(delayTime); continue; }
				retryCount=0;
				MoneyCoinInfo inCoinInfo;
				memcpy(&inCoinInfo, &coinDevice1->moneyCoinInfo, sizeof(coinDevice1->moneyCoinInfo));
				coinDevice1->clearMoneyCoinInfo();
				for (int index=1; index<MONEY_COIN_TYPE_COUNT; index++)
				{
					if (inCoinInfo.Count[index] != 0)
					{
						printf("[DEBUG] Coin income (%d): %d [%d RUR]\n", index, inCoinInfo.Count[index], settings->coinWeight.Weight[index]);
						status.extDeviceInfo.coin_incomeInfo.Count[index] += inCoinInfo.Count[index];
						int coinCount = inCoinInfo.Count[index];
						inCoinInfo.Count[index] = 0;
						db->Log( 0, DB_EVENT_TYPE_EXT_MONEY_EVENT, coinCount, settings->coinWeight.Weight[index], "COIN INCOME");
					}
				}
				if (rejectedCoinInfo.coinRejected == 1)
				{
					char messageText[250];
					rejectedCoinInfo.coinRejected = 0;
					sprintf(messageText, "COIN REJECTED: Cnt: %d Res: %d, Crd: %d, Err: %d", rejectedCoinInfo.dataCnt, rejectedCoinInfo.dataRes, rejectedCoinInfo.dataCrd, rejectedCoinInfo.dataErr);
					db->Log( 0, DB_EVENT_TYPE_EXT_COIN_REJECTED, rejectedCoinInfo.dataCrd, rejectedCoinInfo.dataErr, messageText);
				}
				delay_ms(delayTime);
			}
		}
		else
			retryCount++;
		db->Log( 0, DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "Coin acceptor device closed");
		coinDevice1->CloseDevice();
		if (retryCount < 2) { settings->workFlag.CoinWatch = 0; delay(1);}
		else {retryCount = 0; if (!errorDisplay) {errorDisplay = 1; settings->intErrorCode.MainWatch = 220;} while (retryCount++ < 60) {delay(1); settings->workFlag.CoinWatch = 0;} }
		printf("[DEBUG] Coin device REDETECT (%d) ... \n", retryCount);
		coinDevice1->Detect();
	}
	db->Close();
	printf("[COIN]: Thread ended.\n");
	return (void*)0;
}
