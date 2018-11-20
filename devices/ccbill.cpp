#include "../main.h"

//#define DEBUG_PACKET_PRINT

extern char str[100];

CCBillDevice* CCBillDevice::p_instance = 0;

CCBillDevice::CCBillDevice()
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
}

void CCBillDevice::Init(void* settings)
{
	// TODO: Load config
}

bool CCBillDevice::IsOpened() { return isOpened;}

int CCBillDevice::Detect()
{
	bool status = 0;
	while (detectInProgress) delay_ms(10);
	detectInProgress = true;
	printf("[BILL Device]: Detecting device ...\n");
    for(int index=0; index < 13; index++)
    {
		if (Cport[index] > 0)
		{
			printf("[BILL Device]: Port is busy: %s\n", comports[index]);
			continue;
		}
		comPortNumber = index;
		if (!OpenDevice()) {CloseDevice(); continue;}
		status = 0;
		printf("[BILL Device]: Checking device: %s\n", comports[index]);
		if (cmdGetStatus())
			printf("[BILL Device]: Get device information - success\n");
		else
			printf("[BILL Device]: Get device information - failed\n");
		delay_ms(200);
		if (cmdReset())
		{
			delay(5);
			status = cmdCheck();
			int tryCount = 10;
			while ((tryCount--) && (!status))
			{
				printf("[BILL Device]: Check device connection - failed\n");
				delay(1);
				status = cmdCheck();
			}
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

bool CCBillDevice::OpenDevice()
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

bool CCBillDevice::CloseDevice()
{
	RS232_CloseComport(comPortNumber);
	isOpened = 0;
	return isOpened;
}

bool CCBillDevice::cmdReset()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	CcNetCommand* command = new CcNetCommand(0x03, 0x30, 0, 0);
	command->BuildCommand();

	int cmdLength 	= command->GetCmdLength();
	BYTE cmdData[1024];
	memcpy(cmdData, command->GetCmdData(), cmdLength);
	delete command;

	int tryCount = 3;
	while (tryCount--)
	{
		RS232_SendBuf(comPortNumber, cmdData, cmdLength);
		if (settings->debugFlag.MoneyWatch)
		{
			#ifdef DEBUG_PACKET_PRINT
			printf("RESET S: ");
			for (int i=0; i < cmdLength; i++)
				printf(" %02X", cmdData[i]);
			printf("\n");
			#endif
		}
		delay_ms(50);

		BYTE buffer[1024];
		int dataSize = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));

		if (settings->debugFlag.MoneyWatch)
		{
			#ifdef DEBUG_PACKET_PRINT
			BYTE* data = &buffer[0];
			printf("      R: ");
			for (int i=0; i < dataSize; i++)
				printf(" %02X", *(data+i));
			printf("\n");
			#endif
		}

		if (dataSize > 0) return 1;
	}
	return 0;
}

bool CCBillDevice::cmdCheck()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	CcNetCommand* command = new CcNetCommand(0x03, 0x37, 0, 0);
	command->BuildCommand();

	int cmdLength = command->GetCmdLength();
	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());
	if (settings->debugFlag.MoneyWatch)
	{
		#ifdef DEBUG_PACKET_PRINT
		BYTE* data = command->GetCmdData();
		printf("CHECK S: ");
		for (int i=0; i < command->GetCmdLength(); i++)
			printf(" %02X", *(data+i));
		printf("\n");
		#endif
	}

	delay_ms(50);

	BYTE buffer[512];
	int dataSize = RS232_PollComport(comPortNumber, buffer, sizeof(buffer));

	if (settings->debugFlag.MoneyWatch)
	{
		#ifdef DEBUG_PACKET_PRINT
		BYTE* data = &buffer[0];
		printf("      R: ");
		for (int i=0; i < dataSize; i++)
			printf(" %02X", *(data+i));
		printf("\n");
		#endif
	}

	if (dataSize > 37)
	{
		memcpy(model, &buffer[4], 15);
		memcpy(serial, &buffer[4+15], 12);
		memcpy(cashCodeID.billId, &buffer[4+27], 7);
	}
	delete command;
	return (dataSize > 37);
}

bool CCBillDevice::cmdGetStatus()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	CcNetCommand* command = new CcNetCommand(0x03, 0x31, 0, 0);
	command->BuildCommand();

	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());
	if (settings->debugFlag.MoneyWatch)
	{
		#ifdef DEBUG_PACKET_PRINT
		BYTE* data = command->GetCmdData();
		printf("STAT  S: ");
		for (int i=0; i < command->GetCmdLength(); i++)
			printf(" %02X", *(data+i));
		printf("\n");
		#endif
	}

	delete command;
	delay_ms(20);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

	if (settings->debugFlag.MoneyWatch)
	{
		#ifdef DEBUG_PACKET_PRINT
		BYTE* data = &buffer[0];
		printf("      R: ");
		for (int i=0; i < dataSize; i++)
			printf(" %02X", *(data+i));
		printf("\n");
		#endif
	}

	return (dataSize > 0);
}

bool CCBillDevice::cmdSetAcceptBillType(BYTE billType1, BYTE billType2, BYTE billType3)
{
	BYTE data[] = {billType1, billType2, billType3, 0x00, 0x00, 0x00};

	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	CcNetCommand* command = new CcNetCommand(0x03, 0x34, data, sizeof(data));
	command->BuildCommand();

	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delete command;
	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

	return (dataSize > 0);
}

bool CCBillDevice::cmdASK()
{
	if (!isOpened) OpenDevice();
	if (!isOpened) return false;

	CcNetCommand* command = new CcNetCommand(0x03, 0x00, 0, 0);
	command->BuildCommand();

	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delete command;
	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

	return (dataSize > 0);
}

void CCBillDevice::cmdDispenseBill()
{
	Settings* 	settings 	= Settings::getInstance();		// ��������� ����������
	int t =0;
	WORD result = 0;
	BYTE data[50];
	memset(data, 0, 50);

	if (!isOpened) OpenDevice();
	if (!isOpened) return ;

	CcNetCommand* command = new CcNetCommand(0x03, 0x66, 0, 0);
	command->BuildCommand();

	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

	if (dataSize <= 0) { delete command; return;}

	bool status = command->ParseCommand(&buffer[0], dataSize);
	if (!status) { delete command; return;}

	result = (command->CMD<< 8);
	if (command->dataLength > 0)
	{
		result += command->DATA[1];
	}

	delete command;
	delay_ms(delay_size);
	return ;
}

WORD CCBillDevice::cmdPoll()	// [FLAG][BILL_TYPE] - 2 bytes
{
	Settings* 	settings 	= Settings::getInstance();		// ��������� ����������
	int t =0;
	WORD result = 0;

	if (!isOpened) OpenDevice();
	if (!isOpened) return 0xFFFF;

	CcNetCommand* command = new CcNetCommand(0x03, 0x33, 0, 0);
	command->BuildCommand();

	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

	if (dataSize <= 0) { delete command; return 0xFF00;}

	bool status = command->ParseCommand(&buffer[0], dataSize);
	if (!status) {delete command; return 0xFF00;}

	result = (command->CMD<< 8);
	if (command->dataLength > 0)
	{
		settings->busyFlag.ExtCommonThread++;
		result += command->DATA[0];
		if ((command->CMD == 0x81) && (command->DATA[0] > 0) && (command->DATA[0] <= 16))
		{
			moneyCoinInfo.Count[command->DATA[0]]++;
			allMoneyCoinInfo.Count[command->DATA[0]]++;
		}
		settings->busyFlag.ExtCommonThread--;
		if (settings->busyFlag.ExtCommonThread > 200) settings->busyFlag.ExtCommonThread = 0;
	}

	delete command;
	delay_ms(delay_size);
	return result;
}

WORD CCBillDevice::cmdUnload()
{
	Settings* 	settings 	= Settings::getInstance();		// ��������� ����������
	int t =0;
	WORD result = 0;

	if (!isOpened) OpenDevice();
	if (!isOpened) return 0xFFFF;

	CcNetCommand* command = new CcNetCommand(0x03, 0x3D, 0, 0);
	command->BuildCommand();

	RS232_SendBuf(comPortNumber, command->GetCmdData(), command->GetCmdLength());

	delay_ms(50);

	BYTE buffer[1024];
	memset(&buffer[0], 0, 1000);
	int dataSize = RS232_PollComport(comPortNumber, &buffer[0], 1024);

//	for (t=0; t<dataSize; t++)
//		printf("buffer[%d] = %04X\n", t, buffer[t]);
//	printf("\n\n");

	if (dataSize <= 0) { delete command; return 0xFF00;}

	bool status = command->ParseCommand(&buffer[0], dataSize);
	if (!status) { delete command; return 0xFF00;}

	result = (command->CMD<< 8);
	if (command->dataLength > 0)
	{
		result += command->DATA[1];
	}

	delete command;
	delay_ms(delay_size);
	return result;
}
