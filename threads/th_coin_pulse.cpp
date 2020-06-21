#include "../main.h"

//#pragma region Обработка событий монетоприемника [PULSE MODE]
/*
	- Обработка событий монетоприемника
	TODO: Сделать обработку ошибок и перезапуск устройства!
*/
PI_THREAD(CoinPulseWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.CoinPulseWatch) return (void*)0;
	unsigned int counter = 0;

	CoinPulseDevice* 	coinDevice 	= CoinPulseDevice::getInstance();
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] Money: Impulse coin acceptor thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	while (settings->threadFlag.CoinPulseWatch)
	{
		while (settings->threadFlag.CoinPulseWatch)
		{
			settings->workFlag.CoinPulseWatch = 0;
			if (settings->workTimeDevice.UseWorkTime)
			{
				if (stopWork == 0)
				{
					coinDevice->Lock(1);
					delay_ms(200);
				}
				else
					coinDevice->Lock(0);
			}
			else
			{
				coinDevice->Lock(0);
			}
			coinDevice->cmdPoll();
			delay_ms(25);
		}
	}
	db->Log(DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "Coin acceptor [pulise] device closed");
	db->Close();
	printf("[COIN IMP]: Thread ended.\n");
	return (void*)0;
}
