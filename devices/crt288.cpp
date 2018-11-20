
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../main.h"

int delayVal = 300;

// 37 40 (wiring 25 29)

BYTE getByteByNum(DWORD valDword, BYTE numByte);

Crt288KDevice* Crt288KDevice::p_instance = 0;

Crt288KDevice::Crt288KDevice()
{
	tempBool = false;
	lockError = false;
	deviceDelayMs = 100;
	isOpened = 0;
	comPortNumber = 0;
	comPortBaundRate = 9600;
	cardPresent = 0;
	errorCount = 0;
}

void Crt288KDevice::Init(Settings* settings)
{
}

void Crt288KDevice::Detect()
{
	comPortNumber = 0;
}

bool Crt288KDevice::OpenDevice()
{
	//Settings* 	settings 	= Settings::getInstance();		// Параметры приложения
	if (isOpened) return isOpened;
	isOpened = 0;

	fd = open("/dev/hidraw0", O_RDWR|O_NONBLOCK);
	if (fd < 0) 
	{
		perror("Unable to open device");
		isOpened = 0;
		return 1;
	}

	printf ("Init device: CRT-288 \n");
	cmdDeviceInit( fd, 1);
	isOpened = 1;
	return isOpened;
}

bool Crt288KDevice::CloseDevice()
{
	if (!isOpened) return isOpened;
	close(fd);
	isOpened = 0;
	return isOpened;
}

bool Crt288KDevice::IsOpened()
{
	return isOpened;
}

DWORD Crt288KDevice::cmdMultiplePoll(BYTE pollCount)
{
	DWORD crcVal = cmdReadCardID(fd);
	cardPresent = ((crcVal > 0xFF) && (crcVal != 0));

	cardNumber[5] = getByteByNum(crcVal, 0);
	cardNumber[4] = getByteByNum(crcVal, 1);
	cardNumber[3] = getByteByNum(crcVal, 2);
	cardNumber[2] = getByteByNum(crcVal, 3);
	cardNumber[1] = getByteByNum(crcVal, 4);
	cardNumber[0] = getByteByNum(crcVal, 5);

	return crcVal;
}

DWORD Crt288KDevice::cmdPoll()
{
	DWORD crcVal = cmdReadCardID(fd);
	cardPresent = ((crcVal > 0xFF) && (crcVal != 0));

	cardNumber[5] = getByteByNum(crcVal, 0);
	cardNumber[4] = getByteByNum(crcVal, 1);
	cardNumber[3] = getByteByNum(crcVal, 2);
	cardNumber[2] = getByteByNum(crcVal, 3);
	cardNumber[1] = getByteByNum(crcVal, 4);
	cardNumber[0] = getByteByNum(crcVal, 5);

	return crcVal;
}

bool Crt288KDevice::cmdTest()
{
	return 1;
}

void Crt288KDevice::Lock(bool lockState)
{
}

void Crt288KDevice::LockError()
{
}

void Crt288KDevice::ErrorBlink()
{
}

void Crt288KDevice::OKBlink()
{
}

///****************************************************************************************
///****************************************************************************************
///****************************************************************************************

BYTE cmdLedCtrl(int fd, BYTE state, BYTE blink, BYTE ledNum)
{
	int res = 0;
	BYTE buffer[256];
	BYTE cmdAck[] = {0x00, 0x06};
	BYTE* command;
	int cmdSize;
	BYTE commandLight[] = {0x00, 0xF2, 0x00, 0x05, 0x43, 0x80, 0x30, 0x30, 0x30, 0x03, 0x00};
	BYTE commandBlink[] = {0x00, 0xF2, 0x00, 0x06, 0x43, 0x80, 0x31, 0x30, 0x01, 0x01, 0x03, 0x00};
	BYTE result = CRT_NOERR;

	if (ledNum == 2)
	{
		commandLight[7] = 0x31;
		commandBlink[7] = 0x31;
	}

	if ((blink == 1) && (state == 1))
	{
		command = commandBlink;
		cmdSize = sizeof(commandBlink);
	}
	else
	{
		command = commandLight;
		cmdSize = sizeof(commandLight);
		if (state == 1) 
			command[8] = 0x31;
	}

	calcCrc(command, cmdSize);

	if (write(fd, command, cmdSize) < 0) 
		return (CRT_CMDWRTERR);
	if (DEBUG_PRINT2)
	{
		printf("write(cmdLed1Ctrl) write %d bytes:\n\t", cmdSize);
		for (int i = 0; i < cmdSize; i++)
			printf("%02X ", command[i]);
		puts("\n");
	}
	usleep(COMM_DELAY*1000);

	res = read(fd, buffer, 16);
	if (res < 0) 
		return (CRT_CMDRDERR);
	if (DEBUG_PRINT2)
	{
		printf("read(cmdLed1Ctrl) read %d bytes:\n\t", res);
		for (int i = 0; i < res; i++)
			printf("%02X ", buffer[i]);
		puts("\n");
	}
	if ((res < 1)  || (buffer[0] == CRT_NAK))
		return (CRT_CMDNAKERR);
	usleep(COMM_DELAY*1000);

	res = read(fd, buffer, 16);
	if (res < 0) 
		return (CRT_CMDRDERR);
	usleep(COMM_DELAY*1000);

	if (write(fd, cmdAck, sizeof(cmdAck)) < 0) 
		return (CRT_CMDWRTERR);
	usleep(COMM_DELAY*1000);

	return result;
}

DWORD cmdReadCardID(int fd)
{
	int res = 0;
	BYTE buffer[256];
	BYTE cmdAck[] = {0x00, 0x06};
	BYTE command[] = {0x00, 0xF2, 0x00, 0x05, 0x43, 0x60, 0x30, 0x41, 0x42, 0x03, 0x00};
	DWORD result = CRT_NOERR;

	calcCrc(command, sizeof(command));

	if (write(fd, command, sizeof(command)) < 0) 
		return (CRT_CMDWRTERR);

	if (DEBUG_PRINT)
	{
		printf("write() write %d bytes:\n\t", (int)sizeof(command));
		for (int i = 0; i < sizeof(command); i++)
			printf("%02X ", command[i]);
		puts("\n");
	}
	usleep(COMM_DELAY*1000);

	res = read(fd, buffer, 16);
	if (res < 0) 
		return (CRT_CMDRDERR);
	if ((res < 1)  || (buffer[0] == CRT_NAK))
		return (CRT_CMDNAKERR);
	usleep(COMM_DELAY*1000);

	res = read(fd, buffer, 16);
	if (res < 0) 
		return (CRT_CMDRDERR);

	if (DEBUG_PRINT)
	{
		printf("read() read %d bytes:\n\t", res);
		for (int i = 0; i < res; i++)
			printf("%02X ", buffer[i]);
		puts("\n");
	}

	usleep(COMM_DELAY*1000);

	if (write(fd, cmdAck, sizeof(cmdAck)) < 0) 
		return (CRT_CMDWRTERR);
//	usleep(COMM_DELAY*1000);

	result = 	((buffer[12] & 0x7F) << 24)
				+ (buffer[13] << 16)
				+ (buffer[14] << 8)
				+ (buffer[15]);

	return result;
}

BYTE cmdDeviceInit(int fd, BYTE cardLock)
{
	int res = 0;
	BYTE buffer[256];
	BYTE cmdAck[] = {0x00, 0x06};
	BYTE command[] = {0x00, 0xF2, 0x00, 0x03, 0x43, 0x30, 0x31, 0x03, 0x00};
	BYTE result = CRT_NOERR;

	if (cardLock == 1)
		command[6] = 0x30;
	calcCrc(command, sizeof(command));

	if (write(fd, command, sizeof(command)) < 0) 
		return (CRT_CMDWRTERR);
	usleep(COMM_DELAY*1000);

	res = read(fd, buffer, 16);
	if (res < 0) 
		return (CRT_CMDRDERR);
	if ((res < 1)  || (buffer[0] == CRT_NAK))
		return (CRT_CMDNAKERR);
	usleep(COMM_DELAY*1000);

	res = read(fd, buffer, 16);
	if (res < 0) 
		return (CRT_CMDRDERR);
	usleep(COMM_DELAY*1000);

	if (write(fd, cmdAck, sizeof(cmdAck)) < 0) 
		return (CRT_CMDWRTERR);
	usleep(COMM_DELAY*1000);

	return result;
}

void calcCrc(BYTE* command, WORD bufLen)
{
	for (int i = 0; i < bufLen-1; i++)
		command[bufLen-1] ^= command[i];
}
