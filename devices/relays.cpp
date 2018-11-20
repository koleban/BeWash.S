#include "../main.h"

RelaySwitch* RelaySwitch::p_instance = 0;

void RelaySwitch::Init(Settings* settings)
{
	DWORD currentPin = 0;
	DWORD currentDeviceID = 0;

	int index = 0;
	for (index = 0; index < 10; index++)
	{
		currentDeviceID = DVC_RELAY01 + index;
		if (!settings->getEnabledDevice(currentDeviceID)) continue;
		currentPin = settings->getPinConfig(currentDeviceID, 1);
		if (currentPin == 0xFF) continue;
		setPinModeMy(currentPin, 0);
		setGPIOState(currentPin, 0);
  		pullUpDnControl (currentPin, PUD_DOWN) ;
	}

	for (index = 0; index < 10; index++)
	{
		currentDeviceID = DVC_VALVE01 + index;
		if (!settings->getEnabledDevice(currentDeviceID)) continue;
		currentPin = settings->getPinConfig(currentDeviceID, 1);
		if (currentPin == 0xFF) continue;
		setPinModeMy(currentPin, 0);
		setGPIOState(currentPin, 0);
  		pullUpDnControl (currentPin, PUD_DOWN) ;
	}
}

void RelaySwitch::relayInit(int relayNum)
{
	DWORD  currentDeviceID = DVC_RELAY01 + relayNum - 1;
	if (!settings->getEnabledDevice(currentDeviceID)) return;
	DWORD currentPin = settings->getPinConfig(currentDeviceID, 1);
	if (currentPin == 0xFF) return;
	setPinModeMy(currentPin, 0);
}

void RelaySwitch::relayOn(int relayNum)
{
	DWORD  currentDeviceID = DVC_RELAY01 + relayNum - 1;
	if (!settings->getEnabledDevice(currentDeviceID)) return;
	DWORD currentPin = settings->getPinConfig(currentDeviceID, 1);
	if (currentPin == 0xFF) return;
	setPinModeMy(currentPin, 0);
	setGPIOState(currentPin, 1);
}

void RelaySwitch::relayOff(int relayNum)
{
	DWORD  currentDeviceID = DVC_RELAY01 + relayNum - 1;
	if (!settings->getEnabledDevice(currentDeviceID)) return;
	DWORD currentPin = settings->getPinConfig(currentDeviceID, 1);
	if (currentPin == 0xFF) return;
	setPinModeMy(currentPin, 0);
	setGPIOState(currentPin, 0);
}

void RelaySwitch::valveInit(int valveNum)
{
	DWORD  currentDeviceID = DVC_VALVE01 + valveNum - 1;
	if (!settings->getEnabledDevice(currentDeviceID)) return;
	DWORD currentPin = settings->getPinConfig(currentDeviceID, 1);
	if (currentPin == 0xFF) return;
	setPinModeMy(currentPin, 0);
}

void RelaySwitch::valveOn(int valveNum)
{
	DWORD  currentDeviceID = DVC_VALVE01 + valveNum - 1;
	if (!settings->getEnabledDevice(currentDeviceID)) return;
	DWORD currentPin = settings->getPinConfig(currentDeviceID, 1);
	if (currentPin == 0xFF) return;
	setGPIOState(currentPin, 1);
}

void RelaySwitch::valveOff(int valveNum)
{
	DWORD  currentDeviceID = DVC_VALVE01 + valveNum - 1;
	if (!settings->getEnabledDevice(currentDeviceID)) return;
	DWORD currentPin = settings->getPinConfig(currentDeviceID, 1);
	if (currentPin == 0xFF) return;
	setGPIOState(currentPin, 0);
}
