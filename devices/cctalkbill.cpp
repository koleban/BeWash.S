#include "../main.h"

char billErrors[256][256];

//#define DEBUG_PACKET_PRINT
#define CCTALK_DEST_ADDR	0x00
#define CCTALK_DATA_LEN		0x01
#define CCTALK_SRC_ADDR		0x02
#define CCTALK_CMD			0x03
#define CCTALK_CRC8			(cmdLength-1)

extern char str[100];

CCTalkBillDevice* CCTalkBillDevice::p_instance = 0;

CCTalkBillDevice::CCTalkBillDevice()
{
	comPortNumber = 1;
	eventId = 0;
	delay_size = 200;
	isOpened = false;
	comPortBaundRate = 9600;
	memset(&moneyCoinInfo, 0, sizeof(moneyCoinInfo));
	memset(&allMoneyCoinInfo, 0, sizeof(allMoneyCoinInfo));
	memset(model, 0, sizeof(model));
	memset(serial, 0, sizeof(serial));
	currentEventId = 0;
}

void CCTalkBillDevice::Init(void* settings)
{
	memset(&billErrors, 0x00, sizeof(billErrors));
	strcpy(&billErrors[0][0], "Master inhibit active ");
	strcpy(&billErrors[1][0], "Bill returned from escrow");
	strcpy(&billErrors[2][0], "Invalid bill ( due to validation fail )");
	strcpy(&billErrors[3][0], "Invalid bill ( due to transport problem )");
	strcpy(&billErrors[4][0], "Inhibited bill ( on serial ) ");
	strcpy(&billErrors[5][0], "Inhibited bill ( on DIP switches )");
	strcpy(&billErrors[6][0], "Bill jammed in transport ( unsafe mode )");
	strcpy(&billErrors[7][0], "Bill jammed in stacker");
	strcpy(&billErrors[8][0], "Bill pulled backwards");
	strcpy(&billErrors[9][0], "Bill tamper");
	strcpy(&billErrors[10][0], "Stacker OK");
	strcpy(&billErrors[11][0], "Stacker removed");
	strcpy(&billErrors[12][0], "Stacker inserted ");
	strcpy(&billErrors[13][0], "Stacker faulty ");
	strcpy(&billErrors[14][0], "Stacker full");
	strcpy(&billErrors[15][0], "Stacker jammed");
	strcpy(&billErrors[16][0], "Bill jammed in transport ( safe mode )");
	strcpy(&billErrors[17][0], "Opto fraud detected");
	strcpy(&billErrors[18][0], "String fraud detected");
	strcpy(&billErrors[19][0], "Anti-string mechanism faulty");
	strcpy(&billErrors[20][0], "Barcode detected");
	strcpy(&billErrors[255][0], "Unknown error");
}

bool CCTalkBillDevice::IsOpened() { return isOpened;}

int CCTalkBillDevice::Detect()
{
	bool status = 0;
	while (detectInProgress) delay_ms(10);
	detectInProgress = true;
	printf("[BILL Device]: Detecting device ...\n");
    for(int index=0; index < 15; index++)
    {
		if (Cport[index] > 0)
		{
			printf("[BILL Device]: Port is busy: %s\n", comports[index]);
			continue;
		}
		comPortNumber = index;
		if (!OpenDevice()) {CloseDevice(); continue;}
		status = 0;
		printf("[BILL ccTalk Device]: Checking device: %s\n", comports[index]);
		status = cmdGetStatus();
		if (cmdReset())
		{
			CloseDevice();
			delay_ms(500);
			OpenDevice();
			int timeout = 5;
			while (1 && (timeout--)) { delay_ms(500);}
			status = cmdGetStatus();
			cmdPoll();
		}

		if (status)
		{
			comPortNumber = index;
			printf("[BILL Device]: Device - %s\n", comports[index]);
			detectInProgress = false;
			return 0;
		}
		CloseDevice();
		status = 0;
    }
	detectInProgress = false;
    return 1;
}

bool CCTalkBillDevice::OpenDevice()
{
	if (isOpened) return isOpened;
	isOpened = 0;
	isOpened = (RS232_OpenComport(comPortNumber, comPortBaundRate) == 0);
	BYTE buffer[2048];
	int garbage = 0;
	if (isOpened)
	 	garbage = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));
	return isOpened;
}

bool CCTalkBillDevice::CloseDevice()
{
	RS232_CloseComport(comPortNumber);
	isOpened = 0;
	return isOpened;
}

BYTE CCTalkBillDevice::getCrc8(BYTE *data, BYTE dataLength)
{
	BYTE result = 0x00;
	for (BYTE index=0; index < dataLength; index++)
		result -= *(data + index);
	return result;
}

bool CCTalkBillDevice::cmdReset()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	BYTE dataLength = 0;
	BYTE cmdLength = 5 + dataLength;
	BYTE *cctalkCmd = (BYTE*)malloc(cmdLength);
	*(cctalkCmd + CCTALK_DEST_ADDR) 	= 40;
	*(cctalkCmd + CCTALK_DATA_LEN) 		= dataLength;
	*(cctalkCmd + CCTALK_SRC_ADDR) 		= 0x01;
	*(cctalkCmd + CCTALK_CMD) 	= 0x01;
	*(cctalkCmd + CCTALK_CRC8) 	= getCrc8(cctalkCmd, cmdLength-1);

	RS232_SendBuf(comPortNumber, cctalkCmd, cmdLength);
	#ifdef DEBUG_PACKET_PRINT
	printf("RESET S: ");
	for (int i=0; i < cmdLength; i++)
		printf(" %02X", cctalkCmd[i]);
	printf("\n");
	#endif
	delay_ms(150);

	BYTE buffer[1024];
	int dataSize = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));

	#ifdef DEBUG_PACKET_PRINT
	BYTE* data = &buffer[0];
	printf("      R: ");
	for (int i=0; i < dataSize; i++)
		printf(" %02X", *(data+i));
	printf("\n");
	#endif

	if (dataSize == 10) { free((void*)cctalkCmd); return 1; }

	free((void*)cctalkCmd);
	return 0;
}

bool CCTalkBillDevice::cmdGetStatus()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	BYTE dataLength = 0;
	BYTE cmdLength = 5 + dataLength;
	BYTE *cctalkCmd = (BYTE*)malloc(cmdLength);
	*(cctalkCmd + CCTALK_DEST_ADDR) 	= 40;
	*(cctalkCmd + CCTALK_DATA_LEN) 		= dataLength;
	*(cctalkCmd + CCTALK_SRC_ADDR) 		= 0x01;
	*(cctalkCmd + CCTALK_CMD) 	= 0xF2;
	*(cctalkCmd + CCTALK_CRC8) 	= getCrc8(cctalkCmd, cmdLength-1);

	RS232_SendBuf(comPortNumber, cctalkCmd, cmdLength);
	#ifdef DEBUG_PACKET_PRINT
	printf("GETST S: ");
	for (int i=0; i < cmdLength; i++)
		printf(" %02X", cctalkCmd[i]);
	printf("\n");
	#endif
	delay_ms(150);

	BYTE buffer[1024];
	int dataSize = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));
	#ifdef DEBUG_PACKET_PRINT
	BYTE* data = &buffer[0];
	printf("      R: ");
	for (int i=0; i < dataSize; i++)
		printf(" %02X", *(data+i));
	printf("\n");
	#endif

    if (dataSize == 14)
	{
		sprintf(model, "ITL BV-100");
		memcpy(serial, &buffer[9], 4);
	}
	free((void*)cctalkCmd);
	return (dataSize > 12);
}

bool CCTalkBillDevice::cmdSetAcceptBillType(BYTE billType1, BYTE billType2, BYTE billType3)
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;
	int dataSize  = 0;

	return (dataSize > 0);
}

bool CCTalkBillDevice::cmdASK()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;
	int dataSize  = 0;

	return (dataSize > 0);
}

void CCTalkBillDevice::cmdDispenseBill()
{
	if (!isOpened) OpenDevice();
	int dataSize  = 0;
}

DWORD CCTalkBillDevice::cmdPoll()	// [FLAG][BILL_TYPE] - 2 bytes
{
	DWORD result = CCTB_ERROR_CODE_DEVICE_ERROR_CONNECTION;
	if (!isOpened) OpenDevice();
	if (!isOpened) return result;

	BYTE dataLength = 0;
	BYTE cmdLength = 5 + dataLength;
	BYTE *cctalkCmd = (BYTE*)malloc(cmdLength);
	*(cctalkCmd + CCTALK_DEST_ADDR) 	= 40;
	*(cctalkCmd + CCTALK_DATA_LEN) 		= dataLength;
	*(cctalkCmd + CCTALK_SRC_ADDR) 		= 0x01;
	*(cctalkCmd + CCTALK_CMD) 	= 159;
	*(cctalkCmd + CCTALK_CRC8) 	= getCrc8(cctalkCmd, cmdLength-1);

	RS232_SendBuf(comPortNumber, cctalkCmd, cmdLength);
	#ifdef DEBUG_PACKET_PRINT
	printf("POLL  S: ");
	for (int i=0; i < cmdLength; i++)
		printf(" %02X", cctalkCmd[i]);
	printf("\n");
	#endif
	delay_ms(150);

	BYTE buffer[1024];
	int dataSize = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));
	BYTE* data = &buffer[0];
	#ifdef DEBUG_PACKET_PRINT
	printf("      R: (%3d)", dataSize);
	for (int i=0; i < dataSize; i++)
		printf(" %02X", *(data+i));
	printf("\n");
	#endif
	if (dataSize >= 20)
	{
		BYTE commandId = *(data+3);
		BYTE eventId = *(data+9);
		BYTE errorId = *(data+11);
		BYTE coinId = *(data+10);
		if (currentEventId != eventId)
		{
			char billInfo[512];
			currentEventId = eventId;
			result = (eventId << 24) + (errorId << 8) + coinId;
			sprintf(billInfo, "[DEBUG DEVICE] Income bill in BILL DEVICE [Event: %d from CntEvent: %d, Dt1(Crd):%d, Dt2[Err]:%d] RESULT: %04X", eventId, currentEventId, coinId, errorId, result);
			if ((errorId == 0) && ((coinId > 0) && (coinId < 16)))
			{
				moneyCoinInfo.Count[coinId]++;
				allMoneyCoinInfo.Count[coinId]++;
			}
			else
			{
				if (errorId > 0)
				{
					printf("     ========> BILL error: %s\n", billErrors[errorId]);
				}
			}
		}
		else
		{
			result = CCTB_ERROR_CODE_DEVICE_NO_ERROR;
		}
	}
	else
	{
		result = CCTB_ERROR_CODE_DEVICE_ERROR_ANSWER;
	}

	free((void*)cctalkCmd);
	return result;
}

char* CCTalkBillDevice::getErrorInfo(BYTE errorId)
{
	if ((errorId >= 0) && (errorId <= 20))
		return billErrors[errorId];
	return NULL;
}

bool CCTalkBillDevice::cmdModifyMasterInhibit()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	BYTE dataLength = 1;
	BYTE cmdLength = 5 + dataLength;
	BYTE *cctalkCmd = (BYTE*)malloc(cmdLength);
	*(cctalkCmd + CCTALK_DEST_ADDR) 	= 40;
	*(cctalkCmd + CCTALK_DATA_LEN) 		= dataLength;
	*(cctalkCmd + CCTALK_SRC_ADDR) 		= 0x01;
	*(cctalkCmd + CCTALK_CMD) 	= 228;
	*(cctalkCmd + CCTALK_CMD+1)	= 255;
	*(cctalkCmd + CCTALK_CRC8) 	= getCrc8(cctalkCmd, cmdLength-1);
	//40 2 1 231 63 0 175

	RS232_SendBuf(comPortNumber, cctalkCmd, cmdLength);
	#ifdef DEBUG_PACKET_PRINT
	printf("MMISt S: ");
	for (int i=0; i < cmdLength; i++)
		printf(" %02X", cctalkCmd[i]);
	printf("\n");
	#endif
	delay_ms(150);

	BYTE buffer[1024];
	int dataSize = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));
	#ifdef DEBUG_PACKET_PRINT
	BYTE* data = &buffer[0];
	printf("      R: ");
	for (int i=0; i < dataSize; i++)
		printf(" %02X", *(data+i));
	printf("\n");
	#endif

	free((void*)cctalkCmd);
	return (dataSize > 4);
}

bool CCTalkBillDevice::cmdModifyInhibit()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	BYTE dataLength = 2;
	BYTE cmdLength = 5 + dataLength;
	BYTE *cctalkCmd = (BYTE*)malloc(cmdLength);
	*(cctalkCmd + CCTALK_DEST_ADDR) 	= 40;
	*(cctalkCmd + CCTALK_DATA_LEN) 		= dataLength;
	*(cctalkCmd + CCTALK_SRC_ADDR) 		= 0x01;
	*(cctalkCmd + CCTALK_CMD) 	= 231;
	*(cctalkCmd + CCTALK_CMD+1)	= 63;
	*(cctalkCmd + CCTALK_CMD+2)	= 0;
	*(cctalkCmd + CCTALK_CRC8) 	= getCrc8(cctalkCmd, cmdLength-1);
	//40 2 1 231 63 0 175

	RS232_SendBuf(comPortNumber, cctalkCmd, cmdLength);
	#ifdef DEBUG_PACKET_PRINT
	printf("MMISt S: ");
	for (int i=0; i < cmdLength; i++)
		printf(" %02X", cctalkCmd[i]);
	printf("\n");
	#endif
	delay_ms(150);

	BYTE buffer[1024];
	int dataSize = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));
	#ifdef DEBUG_PACKET_PRINT
	BYTE* data = &buffer[0];
	printf("      R: ");
	for (int i=0; i < dataSize; i++)
		printf(" %02X", *(data+i));
	printf("\n");
	#endif

	free((void*)cctalkCmd);
	return (dataSize > 4);
}

bool CCTalkBillDevice::cmdModifyBillMode()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	BYTE dataLength = 1;
	BYTE cmdLength = 5 + dataLength;
	BYTE *cctalkCmd = (BYTE*)malloc(cmdLength);
	*(cctalkCmd + CCTALK_DEST_ADDR) 	= 40;
	*(cctalkCmd + CCTALK_DATA_LEN) 		= dataLength;
	*(cctalkCmd + CCTALK_SRC_ADDR) 		= 0x01;
	*(cctalkCmd + CCTALK_CMD) 	= 153;
	*(cctalkCmd + CCTALK_CMD+1)	= 0;
	*(cctalkCmd + CCTALK_CRC8) 	= getCrc8(cctalkCmd, cmdLength-1);
	//40 2 1 231 63 0 175

	RS232_SendBuf(comPortNumber, cctalkCmd, cmdLength);
	#ifdef DEBUG_PACKET_PRINT
	printf("MMISt S: ");
	for (int i=0; i < cmdLength; i++)
		printf(" %02X", cctalkCmd[i]);
	printf("\n");
	#endif
	delay_ms(150);

	BYTE buffer[1024];
	int dataSize = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));
	#ifdef DEBUG_PACKET_PRINT
	BYTE* data = &buffer[0];
	printf("      R: ");
	for (int i=0; i < dataSize; i++)
		printf(" %02X", *(data+i));
	printf("\n");
	#endif

	free((void*)cctalkCmd);
	return (dataSize > 4);
}
