#include "../main.h"

// 37 40 (wiring 25 29)

BYTE getByteByNum(DWORD valDword, BYTE numByte);

RFIDDevice* RFIDDevice::p_instance = 0;

RFIDDevice::RFIDDevice()
{
	tempBool = false;
	lockError = false;
	deviceDelayMs = 100;
	isOpened = 0;
	comPortNumber = 0;
	comPortBaundRate = 9600;
	cardPresent = 0;
	errorCount = 0;
	digCardNumber = 0;
}

void RFIDDevice::Init(Settings* settings)
{
    pinLock		= settings->getPinConfig(DVC_RFID_CARD_LIGHT_CONTROL_RED, 1);
    pinUnlock	= settings->getPinConfig(DVC_RFID_CARD_LIGHT_CONTROL_GRN, 1);
	if (pinLock != 0xFF) 	setPinModeMy(pinLock, 0);
	if (pinUnlock != 0xFF) 	setPinModeMy(pinUnlock, 0);
	setGPIOState(pinLock, 	0);
	setGPIOState(pinUnlock, 0);
}

void RFIDDevice::Detect()
{
	bool status = 0;
    for(int index=0; index < 10; index++)
    {
		if (Cport[index] > 0)
			continue;
		comPortNumber = index;
		if (!OpenDevice()) {CloseDevice(); continue;}
		status = cmdTest();
		if (status)
		{
			comPortNumber = index;
			printf("RFID Device: %s\n", comports[index]);
			return;
		}
		CloseDevice();
		status = 1;
    }
}

bool RFIDDevice::OpenDevice()
{
	//Settings* 	settings 	= Settings::getInstance();		// Параметры приложения
	if (isOpened) return isOpened;
	isOpened = 0;

	BYTE buf[5096];
	isOpened = (RS232_OpenComport(comPortNumber, comPortBaundRate) == 0);
	if (isOpened)
	 	RS232_PollComport(comPortNumber, &buf[0], 5096);
	return isOpened;
}

bool RFIDDevice::CloseDevice()
{
	if (!isOpened) return isOpened;
	RS232_CloseComport(comPortNumber);
	isOpened = 0;
	return isOpened;
}

bool RFIDDevice::IsOpened()
{
	return isOpened;
}

DWORD RFIDDevice::cmdMultiplePoll(BYTE pollCount)
{
	DWORD crcVal = 0x00000000;
	DWORD answerBuff[pollCount];
	int index = 0;

	settings->busyFlag.RFIDWatch++;

	while (index++ < pollCount)
	{
		answerBuff[index] = cmdPoll();
		crcVal |= answerBuff[index];
		delay_ms(deviceDelayMs);
	}

	cardPresent = ((crcVal == answerBuff[pollCount]) && (crcVal != 0x00000000));

	settings->busyFlag.RFIDWatch--;
	if (settings->busyFlag.RFIDWatch > 200) settings->busyFlag.RFIDWatch = 0;

	if ((!cardPresent) && (crcVal > 0x00000000)) return 0xFFFFFFFF;

	cardNumber[5] = getByteByNum(crcVal, 0);
	cardNumber[4] = getByteByNum(crcVal, 1);
	cardNumber[3] = getByteByNum(crcVal, 2);
	cardNumber[2] = getByteByNum(crcVal, 3);
	cardNumber[1] = getByteByNum(crcVal, 4);
	cardNumber[0] = getByteByNum(crcVal, 5);

	return crcVal;
}

DWORD RFIDDevice::cmdPoll()
{
	int t =0;
	BYTE data[] = {0x67};
	DWORD newCardNumber = 0x00000000;

	if (!isOpened) OpenDevice();
	if (!isOpened) { errorCount++; return 0xFFFFFFFF;}

	RS232_SendBuf(comPortNumber, data, sizeof(data));

	delay_ms(100);

	memset(&buffer[0], 0, 1000);
	int timeout = deviceDelayMs;
	int dataSize = 0;
	memset(buffer, 0, sizeof(buffer));
	while (timeout-- > 0)
	{
		dataSize += RS232_PollComport(comPortNumber, &buffer[dataSize], 1024-dataSize);
		delay_ms(1);
	}

	if (dataSize == 0)
	{
		memset(buffer, 0, sizeof(buffer));
//		printf("[DEBUG] RFID Card reader: Read error!!!\n");
		errorCount++;
		delay_ms(deviceDelayMs*2);
		return 0xFFFFFFFF;
	}

	if (dataSize == 14)
	{
		errorCount = 0;
		newCardNumber = strtoul((const char*)buffer, NULL, 16);

		cardPresent = (newCardNumber != 0);
		if (newCardNumber != 0)
		{
			cardNumber[5] = getByteByNum(newCardNumber, 0);
			cardNumber[4] = getByteByNum(newCardNumber, 1);
			cardNumber[3] = getByteByNum(newCardNumber, 2);
			cardNumber[2] = getByteByNum(newCardNumber, 3);
			cardNumber[1] = getByteByNum(newCardNumber, 4);
			cardNumber[0] = getByteByNum(newCardNumber, 5);
			digCardNumber = newCardNumber;
		}
		else
		{
			memset (cardNumber, 0, 6);
			digCardNumber = 0;
		}
		/// Seg V Kolebanov
		/// Comment string 25.06.2015
		//delay_ms(deviceDelayMs);
	}
	else
	{
		// Clear RS232 buffer
		timeout = deviceDelayMs;
		while (timeout-- > 0)
		{
			dataSize += RS232_PollComport(comPortNumber, &buffer[dataSize], 1024-dataSize);
			delay_ms(1);
		}
		return 0xFFFFFFFF;
	}

	return newCardNumber;
}

bool RFIDDevice::cmdTest()
{
	int tryAgain = 0;
	int t =0;
	BYTE data[] = {0x67};
	DWORD newCardNumber = 0x00000000;
	int timeout = deviceDelayMs;
	int dataSize = 0;

	while (tryAgain++ < 2)
	{
		RS232_SendBuf(comPortNumber, data, sizeof(data));
		delay_ms(100);
		memset(&buffer[0], 0, 1000);
		timeout = deviceDelayMs;
		dataSize = 0;
		while (timeout-- > 0)
		{
			dataSize += RS232_PollComport(comPortNumber, &buffer[dataSize], 1024-dataSize);
			delay_ms(1);
		}

		if (dataSize != 14)
	    {
			// Clear RS232 buffer
			timeout = deviceDelayMs;
			while (timeout-- > 0)
			{
				dataSize += RS232_PollComport(comPortNumber, &buffer[dataSize], 1024-dataSize);
				delay_ms(1);
			}
		}
		else
			break;
	}

	bool st = 0;
	if (dataSize == 14)
		st = ((buffer[10] == 0x4F) && (buffer[11] == 0x4B));
	return st;
}

void RFIDDevice::Lock(bool lockState)
{
	lockError = false;
	if (!settings->getEnabledDevice(DVC_RFID_CARD_READER)) return;
	if (!lockState)
	{
		setGPIOState(pinLock, 	0);
		setGPIOState(pinUnlock, 1);
	}
	else
	{
		setGPIOState(pinLock, 	1);
		setGPIOState(pinUnlock, 0);
	}
}

void RFIDDevice::LockError()
{
	lockError = true;
	setGPIOState(pinLock, 	0);
	setGPIOState(pinUnlock, 0);
}

void RFIDDevice::ErrorBlink()
{
	if (!lockError) return;
	setGPIOState(pinLock, !getGPIOState(pinLock));
}

void RFIDDevice::OKBlink()
{
	tempBool = !tempBool;
	setPinModeMy(pinUnlock, 0);
	setGPIOState(pinUnlock, (int)tempBool);
}
