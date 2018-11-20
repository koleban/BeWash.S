#include "../main.h"

CoinPulseDevice* exchangeDevice = 0;

int globalCounter = 0;

void intCoin1()
{
	int sigTime = 0;
	BYTE coinNum = 4;
	while ((!digitalRead(exchangeDevice->pinCoinType1)) && (sigTime++ < 1000)) delay_ms(1);
	if (sigTime < 5) return;
	printf("[DEBUG] CoinPulseDevice: interrupt 1 handle: TIME: %d [%d]\n", coinNum, sigTime);
	if (sigTime >= exchangeDevice->sigWidth)
	{
		exchangeDevice->moneyCoinInfo.Count[coinNum]++;
		exchangeDevice->allMoneyCoinInfo.Count[coinNum]++;
		printf("Interrupt >>>> : %d [%d]\n", coinNum, globalCounter++);
	}
	else
		printf("[WARNING] CoinPulseDevice: interrupt handle: WARNING !!!: %d [%d]\n", coinNum, sigTime);
}

void intCoin2()
{
	int sigTime = 0;
	BYTE coinNum = 6;
	while ((!digitalRead(exchangeDevice->pinCoinType2)) && (sigTime++ < 1000)) delay_ms(1);
	if (sigTime < 5) return;
	printf("[DEBUG] CoinPulseDevice: interrupt 2 handle: TIME: %d [%d]\n", coinNum, sigTime);
	if (sigTime >= exchangeDevice->sigWidth)
	{
		exchangeDevice->moneyCoinInfo.Count[coinNum]++;
		exchangeDevice->allMoneyCoinInfo.Count[coinNum]++;
		printf("Interrupt >>>> : %d [%d]\n", coinNum, globalCounter++);
	}
	else
		printf("[WARNING] CoinPulseDevice: interrupt handle: WARNING !!!: %d [%d]\n", coinNum, sigTime);
}

void intCoin3()
{
	int sigTime = 0;
	BYTE coinNum = 8;
	while ((!digitalRead(exchangeDevice->pinCoinType3)) && (sigTime++ < 1000)) delay_ms(1);
	if (sigTime < 5) return;
	printf("[DEBUG] CoinPulseDevice: interrupt 3 handle: TIME: %d [%d]\n", coinNum, sigTime);
	if (sigTime >= exchangeDevice->sigWidth)
	{
		exchangeDevice->moneyCoinInfo.Count[coinNum]++;
		exchangeDevice->allMoneyCoinInfo.Count[coinNum]++;
		printf("Interrupt >>>> : %d [%d]\n", coinNum, globalCounter++);
	}
	else
		printf("[WARNING] CoinPulseDevice: interrupt handle: WARNING !!!: %d [%d]\n", coinNum, sigTime);
}

void intCoin4()
{
	int sigTime = 0;
	BYTE coinNum = 10;
	while ((!digitalRead(exchangeDevice->pinCoinType4)) && (sigTime++ < 1000)) delay_ms(1);
	if (sigTime < 5) return;
	printf("[DEBUG] CoinPulseDevice: interrupt 4 handle: TIME: %d [%d]\n", coinNum, sigTime);
	if (sigTime >= exchangeDevice->sigWidth)
	{
		exchangeDevice->moneyCoinInfo.Count[coinNum]++;
		exchangeDevice->allMoneyCoinInfo.Count[coinNum]++;
		printf("Interrupt >>>> : %d [%d]\n", coinNum, globalCounter++);
	}
	else
		printf("[WARNING] CoinPulseDevice: interrupt handle: WARNING !!!: %d [%d]\n", coinNum, sigTime);
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
	BYTE currentDeviceIDAdd = DVC_COIN_PULSE_ACCEPTOR_ADD;

	bool deviceStatus = settings->getEnabledDevice(currentDeviceID) | settings->getEnabledDevice(currentDeviceIDAdd);

	if (!deviceStatus) { return; }

	pinCoinType1 = settings->getPinConfig(currentDeviceID, 1); 		// 1 RUR
	pinCoinType2 = settings->getPinConfig(currentDeviceID, 2);		// 2 RUR
	pinCoinType3 = settings->getPinConfig(currentDeviceID, 3);		// 5 RUR
	pinCoinType4 = settings->getPinConfig(currentDeviceID, 4);		// 10 RUR
	pinCoinLock  = settings->getPinConfig(currentDeviceIDAdd, 4);	// CoinLock
	if (pinCoinLock >= 0x5F) settings->getPinConfig(currentDeviceIDAdd, 1);

	if ((pinCoinType1 != 0xFF) && (pinCoinType1 != 0x00))
		{ setPinModeMy(pinCoinType1, 1); wiringPiISR( pinCoinType1, INT_EDGE_FALLING, &intCoin1);}
	if ((pinCoinType2 != 0xFF) && (pinCoinType2 != 0x00))
		{ setPinModeMy(pinCoinType2, 1); wiringPiISR( pinCoinType2, INT_EDGE_FALLING, &intCoin2);}
	if ((pinCoinType3 != 0xFF) && (pinCoinType3 != 0x00))
		{ setPinModeMy(pinCoinType3, 1); wiringPiISR( pinCoinType3, INT_EDGE_FALLING, &intCoin3);}
	if ((pinCoinType4 != 0xFF) && (pinCoinType4 != 0x00))
		{ setPinModeMy(pinCoinType4, 1); wiringPiISR( pinCoinType4, INT_EDGE_FALLING, &intCoin4);}
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
