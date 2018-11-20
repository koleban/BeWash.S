#include "../main.h"

int fd = 0;

char coinErrors[260][260];

extern char str[100];

CoinDevice* CoinDevice::p_instance = 0;

CoinDevice::CoinDevice()
{
	comPortNumber = 0;
	eventId = 0;
	delay_size = 200;
	memset(&moneyCoinInfo, 0, sizeof(moneyCoinInfo));
	memset(&allMoneyCoinInfo, 0, sizeof(allMoneyCoinInfo));
}

void CoinDevice::Init(void* settings)
{
	isOpened = false;
	memset(&coinErrors, 0x00, sizeof(coinErrors));
	strcpy(&coinErrors[0][0], "No errors");
	strcpy(&coinErrors[1][0], "Reject coin");
	strcpy(&coinErrors[2][0], "Inhibited coin");
	strcpy(&coinErrors[3][0], "Multiple window");
	strcpy(&coinErrors[4][0], "Wake-up timeout");
	strcpy(&coinErrors[5][0], "Validation timeout");
	strcpy(&coinErrors[6][0], "Credit sensor timeout");
	strcpy(&coinErrors[7][0], "Sorter opto timeout");
	strcpy(&coinErrors[8][0], "2nd close coin error");
	strcpy(&coinErrors[9][0], "Accept gate not ready");
	strcpy(&coinErrors[10][0], "Credit sensor not ready");
	strcpy(&coinErrors[11][0], "Sorter not ready");
	strcpy(&coinErrors[12][0], "Reject coin not cleared");
	strcpy(&coinErrors[13][0], "Validation sensor not ready");
	strcpy(&coinErrors[14][0], "Credit sensor blocked");
	strcpy(&coinErrors[15][0], "Sorter opto blocked");
	strcpy(&coinErrors[16][0], "Credit sequence error");
	strcpy(&coinErrors[17][0], "Coin going backwards");
	strcpy(&coinErrors[18][0], "Coin too fast ( over credit sensor )");
	strcpy(&coinErrors[19][0], "Coin too slow ( over credit sensor )");
	strcpy(&coinErrors[20][0], "C.O.S. mechanism activated");
	strcpy(&coinErrors[21][0], "DCE opto timeout");
	strcpy(&coinErrors[22][0], "DCE opto not seen");
	strcpy(&coinErrors[23][0], "Credit sensor reached too early");
	strcpy(&coinErrors[24][0], "Reject coin ( repeated sequential trip )");
	strcpy(&coinErrors[25][0], "Reject slug");
	strcpy(&coinErrors[26][0], "Reject sensor blocked");
	strcpy(&coinErrors[27][0], "Games overload");
	strcpy(&coinErrors[28][0], "Max. coin meter pulses exceeded");
	strcpy(&coinErrors[29][0], "Accept gate open not closed");
	strcpy(&coinErrors[30][0], "Accept gate closed not open");
	strcpy(&coinErrors[31][0], "Manifold opto timeout");
	strcpy(&coinErrors[32][0], "Manifold opto blocked");
	strcpy(&coinErrors[33][0], "Manifold not ready");
	strcpy(&coinErrors[34][0], "Security status changed");
	strcpy(&coinErrors[35][0], "Motor exception");
	strcpy(&coinErrors[36][0], "Swallowed coin");
	strcpy(&coinErrors[37][0], "Coin too fast ( over validation sensor )");
	strcpy(&coinErrors[38][0], "Coin too slow ( over validation sensor )");
	strcpy(&coinErrors[39][0], "Coin incorrectly sorted");
	strcpy(&coinErrors[39][0], "External light attack");
	strcpy(&coinErrors[254][0], "Coin return mechanism activated");
}

bool CoinDevice::IsOpened()
{
	return isOpened;
}

int CoinDevice::Detect()
{
	bool status = 0;
	while (detectInProgress) delay_ms(10);
	detectInProgress = true;
	printf("[COIN Device]: Detecting device ...\n");
    for(int index=0; index < 10; index++)
    {
		if (Cport[index] > 0)
			continue;
		comPortNumber = index;
		if (!OpenDevice()) {CloseDevice(); continue;}
		status = 0;
		printf("[COIN Device]: Checking device: %s\n", comports[index]);
		if (cmdReset())
		{
			delay_ms(200);
			status = cmdCheck();
		}
		if (status)
		{
			comPortNumber = index;
			printf("[COIN Device]: Device - %s\n", comports[index]);
			detectInProgress = false;
			return 0;
		}
		CloseDevice();
		status = 1;
    }
	detectInProgress = false;
    return 1;
}

bool CoinDevice::OpenDevice()
{
	if (isOpened) return isOpened;
	isOpened = 0;

	isOpened = (RS232_OpenComport(comPortNumber, 9600) == 0);
	BYTE buffer[2048];
	int garbage = 0;
	if (isOpened)
	{
//		if (settings->debugFlag.CoinWatch)
//			printf("[CoinDevice::OpenDevice()] - Opened\n");
	 	garbage = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));
	}
	eventId = 0;
	return isOpened;
}

bool CoinDevice::CloseDevice()
{
	RS232_CloseComport(comPortNumber);
	fd = 0;
	isOpened = 0;
	return isOpened;
	eventId = 0;
}

bool CoinDevice::cmdReset()
{
	int t =0;
	DWORD result = 1;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return isOpened;

//	if (settings->debugFlag.CoinWatch)
//		printf("[bool CoinDevice::cmdReset()] - >>>\n");
	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 1, 0, 0);
	command->BuildCommand();
	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delete command;
	delay_ms(delay_size);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);
	delay_ms(delay_size);
	eventId = 0;
//	if (settings->debugFlag.CoinWatch)
//		printf("[bool CoinDevice::cmdReset()] - <<< %d bytes\n", dataSize);

	return result & (dataSize > 0);
}

bool CoinDevice::cmdCheck()
{
	int t =0;
	DWORD result = 1;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return isOpened;

	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 245, 0, 0);
	command->BuildCommand();
	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delete command;
	delay_ms(delay_size);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);
	delay_ms(delay_size);

//	if (settings->debugFlag.CoinWatch)
//		printf("[bool CoinDevice::cmdCheck()] - <<< %d bytes\n", dataSize);
	return result & (dataSize > 15);
}

bool CoinDevice::cmdSiplePoll()
{
	int t =0;
	DWORD result = 1;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return isOpened;

	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 254, 0, 0);
	command->BuildCommand();
	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delete command;

	delay_ms(delay_size);

//	if (settings->debugFlag.CoinWatch)
//		printf("[bool CoinDevice::cmdSiplePool()] - >>>\n");

	return result;
}


DWORD CoinDevice::cmdRequestPollingPriority()
{
	int t =0;
	DWORD result = 0;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return isOpened;

	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 249, 0, 0);
	command->BuildCommand();

	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

	if (dataSize <= 0) { delete command; return 0;}

	bool status = command->ParseCommand(&buffer[0], dataSize);

	result = command->data[1] * 10;

	delete command;
	delay_ms(delay_size);
	return result;
}

BYTE CoinDevice::cmdRequestStatus()
{
	int t =0;
	BYTE result = 0;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return isOpened;

	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 248, 0, 0);
	command->BuildCommand();

	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

	if (dataSize <= 0) { delete command; return 0xFF;}

	bool status = command->ParseCommand(&buffer[0], dataSize);

	result = command->data[0];
	delete command;
	delay_ms(delay_size);

	return result;
}

DWORD CoinDevice::cmdReadEventBuffer(BYTE* bufData)
{
	int t =0;
	BYTE currentEventId = 0;
	DWORD result = 0;

	Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return isOpened;

	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 229, 0, 0);
	command->BuildCommand();

	int dataSize = RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());
	if (dataSize <= 0) { delete command; return 0xFFFF;}

	delay_ms(50);

	dataSize = 0;
	BYTE buffer[1024];
	memset(buffer, 0 ,sizeof(buffer));
	dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

	if (dataSize == 0) { delete command; return 0;}
	if (dataSize < 0) { delete command; return 0xFFFF;}

	result = 0x00;
	bool status = command->ParseCommand(&buffer[0], dataSize);
	if ((status == 1) && (command->dataLength == 0x0b))
	{
    	currentEventId = command->data[0];
		if (eventId != currentEventId)
		{
			settings->busyFlag.ExtCommonThread++;
			result = ((BYTE)(command->data[1]) << 8) + (BYTE)command->data[2];
			if (((currentEventId - eventId) != 1) && (currentEventId != 0))
			{
				printf("[DEBUG DEVICE] COIN FALSE DETECTED\n");
				result |= 0xAAAA0000;
			}

			int first = 1;
			int counter = 0;
			while ((counter <= 6) && (currentEventId != eventId))
			{
				char coinInfo[512];
				if (first)
				{
					sprintf(coinInfo, "[DEBUG DEVICE] Income coin in COIN DEVICE [Event: %d from CntEvent: %d, Dt1(Crd):%d, Dt2[Err]:%d] RESULT: %04X", eventId, currentEventId, command->data[counter+1], command->data[counter+2], result);
					queueLog->QueuePut(DB_EVENT_TYPE_EXT_MONEY_EVENT, 0, 0, coinInfo);
					printf("%s\n", coinInfo);
					if (command->data[counter+2] > 0)
					{
						queueLog->QueuePut(DB_EVENT_TYPE_EXT_COIN_REJECTED, 0, 0, coinErrors[command->data[counter+2]]);
						printf("     *=======> Coin error: %s\n",coinErrors[command->data[counter+2]]);
					}
				}
				else
				{
					sprintf(coinInfo, "[DEBUG DEVICE] Income coin in COIN DEVICE [Event: %d from CntEvent: %d, Dt1(Crd):%d, Dt2[Err]:%d] RESULT: %04X", eventId, currentEventId, command->data[counter+1], command->data[counter+2], result);
					queueLog->QueuePut(DB_EVENT_TYPE_EXT_MONEY_EVENT, 0, 0, coinInfo);
					printf("%s\n", coinInfo);
					if (command->data[counter+1] > 0)
					{
						queueLog->QueuePut(DB_EVENT_TYPE_EXT_COIN_REJECTED, 0, 0, coinErrors[command->data[counter+1]]);
						printf("     ========> Coin error: %s\n",coinErrors[command->data[counter+1]]);
					}
				}
				first = 0;
				eventId++;
				counter++;
				if ((command->data[counter] != 0) && (command->data[counter] < MONEY_COIN_TYPE_COUNT))
				{
					moneyCoinInfo.Count[command->data[counter]]++;
					allMoneyCoinInfo.Count[command->data[counter]]++;
					printf(" ... accept.\n");
				}
				else
				{
					rejectedCoinInfo.coinRejected = 1;
					rejectedCoinInfo.dataCnt = currentEventId;
					rejectedCoinInfo.dataCrd = command->data[counter];
					rejectedCoinInfo.dataErr = command->data[counter+1];
					rejectedCoinInfo.dataRes = result;
					printf(" ... rejected.\n");
				}
				counter++;
			}

			eventId = currentEventId;
			settings->busyFlag.ExtCommonThread--;
			if (settings->busyFlag.ExtCommonThread > 200) settings->busyFlag.ExtCommonThread = 0;
		}
	}
	delete command;
	delay_ms(delay_size);

	return result;
}

bool CoinDevice::cmdGetMasterInhibitStatus()
{
	int t =0;
	DWORD result = 0;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return isOpened;

	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 227, 0, 0);
	command->BuildCommand();
	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);
	if (dataSize <= 0) { delete command; return 0;}
	bool status = command->ParseCommand(&buffer[0], dataSize);
	result = ((command->data[0] & 0x01) == 0);
	delete command;
	delay_ms(delay_size);
	return result & (dataSize > 0);
}

bool CoinDevice::cmdSetMasterInhibitStatus()
{
	int t =0;
	DWORD result = 0;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return isOpened;

	BYTE newData[] = {0x01};
	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 228, newData, 1);
	command->BuildCommand();
	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);
	if (dataSize <= 0) { delete command; return 0;}
	bool status = command->ParseCommand(&buffer[0], dataSize);
	result = command->data[0];
	delete command;
	delay_ms(delay_size);
	return result;
}

void CoinDevice::cmdModifyInhibitStatus(BYTE data1, BYTE data2)
{
	int t =0;
	DWORD result = 0;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return;

//	if (settings->debugFlag.CoinWatch)
//		printf("[CoinDevice::cmdModifyInhibitStatus(BYTE data1, BYTE data2)] - >>>\n");
	BYTE newData[] = {data1, data2};
	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 231, newData, 2);
	command->BuildCommand();
	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);
//	if (settings->debugFlag.CoinWatch)
//		printf("[CoinDevice::cmdModifyInhibitStatus(BYTE data1, BYTE data2)] - <<< %d bytes\n", dataSize);
	delete command;
	delay_ms(delay_size);
}

void CoinDevice::cmdSetCoinSortPath(BYTE coinIndex, BYTE sortPath)
{
	int t =0;
	DWORD result = 0;

	//Settings* 	settings 	= Settings::getInstance();		// ��������� ����������

	if (!isOpened) OpenDevice();
	if (!isOpened) return;

//	if (settings->debugFlag.CoinWatch)
//		printf("[CoinDevice::cmdModifyInhibitStatus(BYTE data1, BYTE data2)] - >>>\n");
	BYTE newData[] = {coinIndex, sortPath};
	CcTalkCommand* command = new CcTalkCommand(0x02, 0x01, 210, newData, 2);
	command->BuildCommand();
	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);
//	if (settings->debugFlag.CoinWatch)
//		printf("[CoinDevice::cmdModifyInhibitStatus(BYTE data1, BYTE data2)] - <<< %d bytes\n", dataSize);
	delete command;
	delay_ms(delay_size);
}

void CoinDevice::clearMoneyCoinInfo()
{
	memset(&moneyCoinInfo, 0, sizeof(moneyCoinInfo));
}