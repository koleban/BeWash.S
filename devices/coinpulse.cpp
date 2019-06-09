#include "../main.h"

CoinPulseDevice* exchangeDevice = 0;

int globalCounter = 0;
unsigned long warningPulseCount[4];
unsigned long warningPulseLength[4];

void intCoin1()
{
	int sigTime = 0;
	BYTE coinNum = 4;
	int fallCount = 0;
	while ((fallCount < 10) && (sigTime++ < 1000)) 
	{
		if (digitalRead(exchangeDevice->pinCoinType1)) fallCount++;
		delay_ms(1);
	}
//	while ((!digitalRead(exchangeDevice->pinCoinType1)) && (sigTime++ < 500)) delay_ms(1);
	if (sigTime < 5) return;
	if (sigTime >= exchangeDevice->sigWidth)
	{
		exchangeDevice->moneyCoinInfo.Count[coinNum]++;
		exchangeDevice->allMoneyCoinInfo.Count[coinNum]++;
		printf("[DEBUG] CoinPulseDevice: interrupt 1 handle: TIME: COIN_ID: %d [time: %d] [glblCounter: %d]\n", coinNum, sigTime, globalCounter++);
	}
	else
	{
		warningPulseCount[0]++;
		warningPulseLength[0] = sigTime;
		//printf("[WARNING] CoinPulseDevice: interrupt handle: WARNING !!!: %d [%d]\n", coinNum, sigTime);
	}
}

void intCoin2()
{
	int sigTime = 0;
	BYTE coinNum = 6;
	int fallCount = 0;
	while ((fallCount < 10) && (sigTime++ < 1000)) 
	{
		if (digitalRead(exchangeDevice->pinCoinType2)) fallCount++;
		delay_ms(1);
	}
	if (sigTime < 5) return;
	if (sigTime >= exchangeDevice->sigWidth)
	{
		exchangeDevice->moneyCoinInfo.Count[coinNum]++;
		exchangeDevice->allMoneyCoinInfo.Count[coinNum]++;
		printf("[DEBUG] CoinPulseDevice: interrupt 2 handle: TIME: COIN_ID: %d [time: %d] [glblCounter: %d]\n", coinNum, sigTime, globalCounter++);
	}
	else
	{
		warningPulseCount[1]++;
		warningPulseLength[1] = sigTime;
		//printf("[WARNING] CoinPulseDevice: interrupt handle: WARNING !!!: %d [%d]\n", coinNum, sigTime);
	}
}

void intCoin3()
{
	int sigTime = 0;
	BYTE coinNum = 8;
	int fallCount = 0;
	while ((fallCount < 10) && (sigTime++ < 1000)) 
	{
		if (digitalRead(exchangeDevice->pinCoinType3)) fallCount++;
		delay_ms(1);
	}
	if (sigTime < 5) return;
	if (sigTime >= exchangeDevice->sigWidth)
	{
		exchangeDevice->moneyCoinInfo.Count[coinNum]++;
		exchangeDevice->allMoneyCoinInfo.Count[coinNum]++;
		printf("[DEBUG] CoinPulseDevice: interrupt 3 handle: TIME: COIN_ID: %d [time: %d] [glblCounter: %d]\n", coinNum, sigTime, globalCounter++);
	}
	else
	{
		warningPulseCount[2]++;
		warningPulseLength[2] = sigTime;
		//printf("[WARNING] CoinPulseDevice: interrupt handle: WARNING !!!: %d [%d]\n", coinNum, sigTime);
	}
}

void intCoin4()
{
	int sigTime = 0;
	BYTE coinNum = 10;
	int fallCount = 0;
	while ((fallCount < 10) && (sigTime++ < 1000)) 
	{
		if (digitalRead(exchangeDevice->pinCoinType4)) fallCount++;
		delay_ms(1);
	}
	if (sigTime < 5) return;
	if (sigTime >= exchangeDevice->sigWidth)
	{
		exchangeDevice->moneyCoinInfo.Count[coinNum]++;
		exchangeDevice->allMoneyCoinInfo.Count[coinNum]++;
		printf("[DEBUG] CoinPulseDevice: interrupt 4 handle: TIME: COIN_ID: %d [time: %d] [glblCounter: %d]\n", coinNum, sigTime, globalCounter++);
	}
	else
	{
		warningPulseCount[3]++;
		warningPulseLength[3] = sigTime;
		//printf("[WARNING] CoinPulseDevice: interrupt handle: WARNING !!!: %d [%d]\n", coinNum, sigTime);
	}
}

CoinPulseDevice* CoinPulseDevice::p_instance = 0;

CoinPulseDevice::CoinPulseDevice()
{
    pinCoinType1 = 0xFF;
    pinCoinType2 = 0xFF;
    pinCoinType3 = 0xFF;
    pinCoinType4 = 0xFF;
    pinCoinLock  = 0xFF;
    sigWidth = settings->coinPulseWidth;
	memset(&moneyCoinInfo, 0, sizeof(moneyCoinInfo));
	exchangeDevice = this;
}

void CoinPulseDevice::Init(Settings* settings)
{
	BYTE currentDeviceID = DVC_COIN_PULSE_ACCEPTOR;
	BYTE currentDeviceIDAdd = DVC_COIN_PULSE_ACCEPTOR_INHIBIT;

	printf("[CoinPulse Device] Init ...\n[CoinPulse Device] PIN[ 4  3  2  1] = CoinWeigth [ 4  6  8  10 ]\n");
	printf("[CoinPulse Device] PIN[4] CoinWeigth[ 4] = %d rur \n", settings->coinWeight.Weight[4]);
	printf("[CoinPulse Device] PIN[3] CoinWeigth[ 6] = %d rur \n", settings->coinWeight.Weight[6]);
	printf("[CoinPulse Device] PIN[2] CoinWeigth[ 8] = %d rur \n", settings->coinWeight.Weight[8]);
	printf("[CoinPulse Device] PIN[1] CoinWeigth[10] = %d rur \n", settings->coinWeight.Weight[10]);

	bool deviceStatus = settings->getEnabledDevice(currentDeviceID) | settings->getEnabledDevice(currentDeviceIDAdd);

	if (!deviceStatus) { return; }

	memset(&warningPulseCount, 0, sizeof(warningPulseCount));
	memset(&warningPulseLength, 0, sizeof(warningPulseLength));

	pinCoinType1 = settings->getPinConfig(currentDeviceID, 1); 		// 1 RUR
	pinCoinType2 = settings->getPinConfig(currentDeviceID, 2);		// 2 RUR
	pinCoinType3 = settings->getPinConfig(currentDeviceID, 3);		// 5 RUR
	pinCoinType4 = settings->getPinConfig(currentDeviceID, 4);		// 10 RUR
	pinCoinLock  = settings->getPinConfig(currentDeviceIDAdd, 4);	// CoinLock
	if (pinCoinLock >= 0x5F) settings->getPinConfig(currentDeviceIDAdd, 1);

	if ((pinCoinType1 != 0xFF) && (pinCoinType1 != 0x00))
		{ pullUpDnControl (pinCoinType1, PUD_UP); setPinModeMy(pinCoinType1, 1); wiringPiISR( pinCoinType1, INT_EDGE_FALLING, &intCoin1);}
	if ((pinCoinType2 != 0xFF) && (pinCoinType2 != 0x00))
		{ pullUpDnControl (pinCoinType2, PUD_UP); setPinModeMy(pinCoinType2, 1); wiringPiISR( pinCoinType2, INT_EDGE_FALLING, &intCoin2);}
	if ((pinCoinType3 != 0xFF) && (pinCoinType3 != 0x00))
		{ pullUpDnControl (pinCoinType3, PUD_UP); setPinModeMy(pinCoinType3, 1); wiringPiISR( pinCoinType3, INT_EDGE_FALLING, &intCoin3);}
	if ((pinCoinType4 != 0xFF) && (pinCoinType4 != 0x00))
		{ pullUpDnControl (pinCoinType4, PUD_UP); setPinModeMy(pinCoinType4, 1); wiringPiISR( pinCoinType4, INT_EDGE_FALLING, &intCoin4);}
	if (pinCoinLock != 0xFF)  { setPinModeMy(pinCoinLock, 0); }

	setGPIOState(pinCoinLock, 1);
}

bool CoinPulseDevice::cmdPoll()
{
	DWORD result = 1;

    sigWidth = settings->coinPulseWidth;
	BYTE currentDeviceID = DVC_COIN_PULSE_ACCEPTOR;
	BYTE currentDeviceIDAdd = DVC_COIN_PULSE_ACCEPTOR_ADD;
	bool deviceStatus = settings->getEnabledDevice(currentDeviceID) | settings->getEnabledDevice(currentDeviceIDAdd);
	if (!deviceStatus) { return 0; }

	delay_ms(10);

	return result;
}

bool CoinPulseDevice::Lock(bool newLockState)
{
	if ((settings->getEnabledDevice(DVC_COIN_PULSE_ACCEPTOR_ADD)) && (pinCoinLock != 0xFF))
		setGPIOState(pinCoinLock, !newLockState);
	return newLockState;
}
