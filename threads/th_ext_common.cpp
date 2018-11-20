#include "../main.h"

//#pragma region Подсистема "ОБЩИЕ ОБРАБОТКИ ВНЕШНЕГО БОКСА"

//
// Обработка контролирует:
// 	-	Поступление монет, купюр и прочего (сбор инфы с устройств)

PI_THREAD(ExtCommonThread)
{
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.ExtCommonThread) return (void*)0;

	CoinPulseDevice* 	coinPulseDevice 	= CoinPulseDevice::getInstance();

	delay_ms(2000);

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] Data: External block thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	status.extDeviceInfo.remote_currentBalance = 0;
	while (settings->threadFlag.ExtCommonThread)
	{
		int delayTime = 300;
		settings->workFlag.ExtCommonThread = 0;
		status.extDeviceInfo.objectId = settings->commonParams.objectId;
		status.extDeviceInfo.deviceId = settings->commonParams.deviceId;
		status.extDeviceInfo.userId = settings->commonParams.userId;
		if (status.extDeviceInfo.remote_currentBalance < 0) status.extDeviceInfo.remote_currentBalance = 0;

		int timeout = 100;
		while ((settings->busyFlag.ExtCommonThread) && (timeout-- > 0)) {delayTime--; delay_ms(1); continue;}
		if (timeout == 0) printf("ExtCommonThread::timeout\n");
		settings->busyFlag.ExtCommonThread++;

		///
		/// Обработка поступления монет и банкнот от Импульсника
		/// Обработка вынесена из основного устройства потока, т.к. импульсник требует быстрой реакции
		/// а данная операци выполняется медленно и БД может чутка притормозить его
		///

		if (settings->threadFlag.CoinPulseWatch)
		{
			MoneyCoinInfo inCoinInfo;
			memcpy(&inCoinInfo, &coinPulseDevice->moneyCoinInfo, sizeof(coinPulseDevice->moneyCoinInfo));
			memset(&coinPulseDevice->moneyCoinInfo, 0, sizeof(coinPulseDevice->moneyCoinInfo));
			for (int index=1; index<MONEY_COIN_TYPE_COUNT; index++)
			{
				if (inCoinInfo.Count[index] != 0)
				{
					printf("[DEBUG] Coin pulse income (%d): %d [%d RUR] \n", index, inCoinInfo.Count[index], settings->coinWeight.Weight[index]);
					status.extDeviceInfo.coin_incomeInfo.Count[index] += inCoinInfo.Count[index];
					int coinCount = inCoinInfo.Count[index];
					inCoinInfo.Count[index] = 0;
					db->Log(DB_EVENT_TYPE_EXT_MONEY_EVENT, coinCount, settings->coinWeight.Weight[index], "COIN PULSE INCOME");
				}
			}
		}

		// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		// [07.03.2018] Закоментировал, т.к. внешний блок не включает реле согласно включеным программам
		// и мапинг реле не реле внешнего блоку не имеет смысла
		///
		/// Relay update if this ExternalBlock (IntThread DISABLED)
		///
		//if (!settings->threadFlag.IntCommonThread)
		//{
		//	for (int index=1; index <= 14; index++)
		//		if (status.extDeviceInfo.relay_currentVal[index-1])
		//			relay->relayOn(index);
		//		else
		//			relay->relayOff(index);
		//}
		///
		/// If this InternalBlock - Mapping ExtReley to IntRelay
		///
		//else
		//{
		//	for (int index=1; index <= 14; index++)
		//		status.intDeviceInfo.relay_currentVal[index-1] = status.intDeviceInfo.relay_currentVal[index-1] | status.extDeviceInfo.relay_currentVal[index-1];
		//}
		// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		///
		/// Обновление свечения кнопок. Иногда были фантомные свечения
		///
		if ((status.extDeviceInfo.button_currentLight < 250) && (status.intDeviceInfo.money_currentBalance <= 0))
		{
			int timeout = 100;
   			while ((timeout-- > 0) && (settings->busyFlag.ButtonWatch)) {delayTime--; delay_ms(1); continue;}
   			settings->busyFlag.ButtonWatch++;
			status.extDeviceInfo.button_newEvent = 255;
			delayTime -= 50;
			delay_ms(50);
			// KSV
			settings->busyFlag.ButtonWatch--;
		}

		settings->busyFlag.ExtCommonThread--;
		if (delayTime > 0) delay_ms(delayTime);
	}
	db->Close();
	printf("[EXT_TH]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
