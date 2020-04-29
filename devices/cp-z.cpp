
#include <linux/types.h>
#include <linux/input.h>

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <memory.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../main.h"

#define WIEGANDMAXDATA 32
#define WIEGANDTIMEOUT 3000000

unsigned char* __wiegandData;    // can capture upto 32 bytes of data -- FIXME: Make this dynamically allocated in init?
unsigned long __wiegandBitCount;                // number of bits currently captured
struct timespec __wiegandBitTime;               // timestamp of the last bit received (used for timeouts)

BYTE getByteByNum(DWORD valDword, BYTE numByte);

RFID_CPZDevice* RFID_CPZDevice::p_instance = 0;

void data0Pulse(void) {
    if (__wiegandBitCount / 8 < WIEGANDMAXDATA) {
        __wiegandData[__wiegandBitCount / 8] <<= 1;
        __wiegandBitCount++;
    }
    clock_gettime(CLOCK_MONOTONIC, &__wiegandBitTime);
}

void data1Pulse(void) {
    if (__wiegandBitCount / 8 < WIEGANDMAXDATA) {
        __wiegandData[__wiegandBitCount / 8] <<= 1;
        __wiegandData[__wiegandBitCount / 8] |= 1;
        __wiegandBitCount++;
    }
    clock_gettime(CLOCK_MONOTONIC, &__wiegandBitTime);
}

void wiegandReset() {
    memset((void *)__wiegandData, 0, WIEGANDMAXDATA);
    __wiegandBitCount = 0;
}

int wiegandGetPendingBitCount() {
    struct timespec now, delta;
    clock_gettime(CLOCK_MONOTONIC, &now);
    delta.tv_sec = now.tv_sec - __wiegandBitTime.tv_sec;
    delta.tv_nsec = now.tv_nsec - __wiegandBitTime.tv_nsec;

    if ((delta.tv_sec > 1) || (delta.tv_nsec > WIEGANDTIMEOUT))
        return __wiegandBitCount;

    return 0;
}

/*
 * wiegandReadData is a simple, non-blocking method to retrieve the last code
 * processed by the API.
 * data : is a pointer to a block of memory where the decoded data will be stored.
 * dataMaxLen : is the maximum number of -bytes- that can be read and stored in data.
 * Result : returns the number of -bits- in the current message, 0 if there is no
 * data available to be read, or -1 if there was an error.
 * Notes : this function clears the read data when called. On subsequent calls,
 * without subsequent data, this will return 0.
 */
 
int wiegandReadData(void* data, int dataMaxLen) {
    if (wiegandGetPendingBitCount() > 0) {
        int bitCount = __wiegandBitCount;
        int byteCount = (__wiegandBitCount / 8) + 1;
        memcpy(data, (void *)__wiegandData, ((byteCount > dataMaxLen) ? dataMaxLen : byteCount));

        wiegandReset();
        return bitCount;
    }
    return 0;
}

RFID_CPZDevice::RFID_CPZDevice()
{
	deviceDelayMs = 100;
	isOpened = 0;
}

void RFID_CPZDevice::Init(Settings* settings)
{
	__wiegandData = (unsigned char*)malloc(WIEGANDMAXDATA);
	memset(__wiegandData, 0, WIEGANDMAXDATA);
	int d0pin = settings->rfidParam.D0;
	int d1pin = settings->rfidParam.D1;

    pinMode(d0pin, INPUT);
    pinMode(d1pin, INPUT);

    pullUpDnControl(d0pin, PUD_UP);
    pullUpDnControl(d1pin, PUD_UP);

    wiringPiISR(d0pin, INT_EDGE_FALLING, data0Pulse);
    wiringPiISR(d1pin, INT_EDGE_FALLING, data1Pulse);
}

void RFID_CPZDevice::Detect()
{
}

bool RFID_CPZDevice::OpenDevice()
{
	printf ("Init device: RFID CP-Z-2 \n");
	isOpened = 1;
	return isOpened;
}

bool RFID_CPZDevice::CloseDevice()
{
	isOpened = 0;
	return isOpened;
}

bool RFID_CPZDevice::IsOpened()
{
	return isOpened;
}

DWORD RFID_CPZDevice::cmdMultiplePoll(BYTE pollCount)
{
	return cmdPoll();
}

DWORD RFID_CPZDevice::cmdPoll()
{
	DWORD crcVal = 0x00;
	int bitLen = wiegandGetPendingBitCount();
	if (bitLen == 0) 
	{
		delay_ms(500);
		memset(cardNumber, 0, sizeof(cardNumber));
	} 
	else 
	{
		char data[100] = {0};
		bitLen = wiegandReadData((void *)data, 100);
		crcVal = *((DWORD*)data) & 0x1FFFFF;
		int bytes = bitLen / 8 + 1;
		if (bytes >= 4)
		{
			printf("RFID: %08X\n", crcVal);
			cardNumber[5] = getByteByNum(crcVal, 0);
			cardNumber[4] = getByteByNum(crcVal, 1);
			cardNumber[3] = getByteByNum(crcVal, 2);
			cardNumber[2] = getByteByNum(crcVal, 3);
			cardNumber[1] = getByteByNum(crcVal, 4);
			cardNumber[0] = getByteByNum(crcVal, 5);
		}
	}
	if (digCardNumber > 0x1FFFFF)
		digCardNumber = 0;
	else
		digCardNumber = crcVal;
	cardPresent = ((crcVal > 0xFF) && (crcVal != 0));
	return crcVal;
}

bool RFID_CPZDevice::cmdTest()
{
	return 1;
}

void RFID_CPZDevice::Lock(bool lockState)
{
}

void RFID_CPZDevice::LockError()
{
}

void RFID_CPZDevice::ErrorBlink()
{
}

void RFID_CPZDevice::OKBlink()
{
}

