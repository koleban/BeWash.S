#include "main.h"

int globalInSock = 0;
int globalThreadId = 0;
int globalNetServerPort = 3355;
int externalCtrl = 0;

WORD loadCfgFile(BYTE* buffer)
{
	char fileName[50];
	int c;
	WORD n = 0;
	memset(fileName, 0, sizeof(fileName));
    sprintf(fileName, "/home/pi/bewash/bewash.cfg");

    FILE * pFile = NULL ;
    if ((pFile=fopen(fileName, "r"))==NULL)
        return 0;
    do
    {
		c = fgetc (pFile);
		if (c != EOF)
		{
			*(buffer+n) = (BYTE)(c & 0xFF);
			n++;
		}
    } while (c != EOF);

    fclose (pFile);
    return n;
}

BYTE saveCfgFile(BYTE* buffer, WORD sizeBuffer)
{
	char sysCmd[100];
	char strTime[100];
	char fileName[50];
	int c;
	BYTE n = 0;
	memset(fileName, 0, sizeof(fileName));
    sprintf(fileName, "/home/pi/bewash/bewash.cfg");

	// Backup old cfg file
	time_t rawtime;
	time(&rawtime);
	strftime(strTime, 100, "%F.%H.%M.%S", localtime(&rawtime));
	sprintf(sysCmd, "sudo cp %s %s.%s", fileName, fileName, strTime);
	system(sysCmd);
	//
    FILE * pFile = NULL ;
    if ((pFile=fopen(fileName, "w"))==NULL)
        return 0;
	for (int index=0; index<sizeBuffer; index++)
		n ^= *(buffer+index);
	fwrite (buffer, sizeof(BYTE), sizeBuffer, pFile);
    fclose (pFile);
    return n;
}

BYTE getCrc(BYTE* packetData, WORD DataLen)
{
	BYTE crc = 0;
	for (int index=0; index<DataLen; index++)
		crc ^= *(packetData+index);
	return crc;
}

PI_THREAD(NetServerClientThread)
{
	Settings* 	settings 	= Settings::getInstance();		// Параметры приложения
	int id = globalThreadId;
	int bytes_read;
	BYTE ask[65535];
	BYTE answer[65535];
	int myClient = globalInSock;

	BYTE cfgFileBuffer[65000];
	WORD cfgFileBufferSize = 0;

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] NetServer: Start server network thread";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	int recvError = 0;
	while (settings->threadFlag.NetServer)
	{
		bytes_read = recv(myClient, ask, 60000, 0);
		if (bytes_read < 0)
		{
			if (externalCtrl-- < 1) externalCtrl = 0;
			if (recvError++ > 10) break;
			delay_ms(400);
			continue;
		}
		if (bytes_read == 0) { printf("[DEBUG]: NetServer Connection timeout ... : %d\n", bytes_read); break;}
		recvError = 0;
  		if (ask[0] 	!= 0x02) continue;
  		if (ask[1] 	!= 0xFE) continue;
		WORD DataLen = *((WORD*)&ask[2])-5;
		long timeout_t = (long)time(NULL);
		while ((DataLen > bytes_read) && (((long)time(NULL) - timeout_t) < 3))
		{
			bytes_read += recv(myClient, &ask[bytes_read], 60000-bytes_read, 0);
			delay_ms(10);
		}
		if (DataLen > bytes_read)
		{
			if (settings->debugFlag.NetServer)
				printf("[NETCTRL] WARNING Network packet lost DataLen: %d RecvBytes: %d\n", DataLen, bytes_read);
			continue;
		}
		int timeout = 150;
		BYTE CMD = ask[4];
		BYTE* packetData = &ask[5];
		unsigned long serial = 0xfFFFFFFF;

		DeviceInfo* inDeviceInfo = (DeviceInfo*)packetData;

		memset(&answer[0], 0, sizeof(answer));
		if (externalCtrl++ > 10) externalCtrl = 10;
		switch(CMD)
		{
			default:
			break;
			case CMD_SRV_GET_SERIAL_NUMBER:
				serial = getDeviceSerialNumber();
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD;
				memcpy(&answer[5], &serial, sizeof(serial));
				DataLen = 5 + sizeof(serial);
				send(myClient, answer, DataLen, 0);
				break;
			case CMD_GET_DEVICE_INFO:
				if (!inDeviceInfo->intDeviceInfo.isReset)
					memcpy(&status.intDeviceInfo, &inDeviceInfo->intDeviceInfo, sizeof(status.intDeviceInfo));
				DataLen = sizeof(status) + 5;
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD;
				memcpy(&answer[5], &status, sizeof(status));
				send(myClient, answer, DataLen, 0);
	   			status.extDeviceInfo.isReset = 0;
				break;
			case CMD_SET_EXT_PRG:
				memcpy(&status.intDeviceInfo, &inDeviceInfo->intDeviceInfo, sizeof(status.intDeviceInfo));
				status.extDeviceInfo.remote_currentProgram = status.intDeviceInfo.program_currentProgram;
				if (settings->debugFlag.NetServer)
					printf("[NETCTRL] Setting new prg: %2d\n", status.intDeviceInfo.program_currentProgram);
				timeout = 150;
				while ((settings->busyFlag.ButtonWatch) && (timeout-->0)) delay_ms(1);
				status.extDeviceInfo.button_newEvent = status.intDeviceInfo.program_currentProgram;
				DataLen = sizeof(status) + 5;
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD;
				memcpy(&answer[5], &status, sizeof(status));
				send(myClient, answer, DataLen, 0);
				break;
			case CMD_SRV_GET_DEVICE_INFO:
				DataLen = 5 + sizeof(status) + sizeof(settings->intErrorCode) + sizeof(settings->extErrorCode);
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD;
				memcpy(&answer[5], 													&status, sizeof(status));
				memcpy(&answer[5+sizeof(status)], 									&settings->intErrorCode, sizeof(settings->intErrorCode));
				memcpy(&answer[5+sizeof(status)+sizeof(settings->intErrorCode)], 	&settings->extErrorCode, sizeof(settings->extErrorCode));
				send(myClient, answer, DataLen, 0);
				break;
			case CMD_SRV_SET_EXT_PRG:
				DataLen = 7;
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD_SRV_SET_EXT_PRG;
				if (settings->progEnabled[*(packetData) & 0x0F])
				{
					if (settings->debugFlag.NetServer)
						printf("[NETCTRL] External command: Setting new prg: %2d\n", (*(packetData)& 0x0F));
					if (db->Log(DB_EVENT_TYPE_EXT_SRV_NEW_PRG, (*(packetData)& 0x0F), 0, "External command: Set new prg"))
						printf("[NETCTRL {CMD_SRV_SET_EXT_PRG} ] IB ERROR: %s\n", db->lastErrorMessage);
					answer[5] = (unsigned char)(*(packetData) & 0x0F);
					answer[6] = (unsigned char)(status.intDeviceInfo.program_currentProgram);
					status.intDeviceInfo.program_currentProgram = (*(packetData) & 0x0F);
					status.intDeviceInfo.extPrgNeedUpdate = 1;
				}
				else
				{
					answer[5] = (BYTE)(status.intDeviceInfo.program_currentProgram);
					answer[6] = 0xFF;
				}
				send(myClient, answer, DataLen, 0);
				break;
			case CMD_SRV_GET_APP_VER:
				DataLen = 7;
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD_SRV_GET_APP_VER;
				answer[5] = (BYTE)prgVer;
				answer[6] = (BYTE)(((int)(prgVer*100))%100);
				if (settings->debugFlag.NetServer)
					printf("[NETCTRL] External command: Get app ver: %2d.%2d\n", answer[5], answer[6]);
				if (send(myClient, answer, DataLen, 0) < 0)
					printf("[NETCTRL] >> External command: Get app ver: ERROR\n");
				break;
			case CMD_SRV_ADD_MONEY:
				status.intDeviceInfo.money_currentBalance += *((short*)packetData);
				status.extDeviceInfo.remote_currentBalance += *((short*)packetData);
				DataLen = 6;
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD_SRV_ADD_MONEY;
				answer[5] = 0xAA;
				if (send(myClient, answer, DataLen, 0) < 0)
					printf("[NETCTRL] >> External command: Add money: ERROR\n");

				if (settings->debugFlag.NetServer)
					printf("[NETCTRL] External command: Add money: %2d\n", *((short*)packetData));
				if (db->Log(DB_EVENT_TYPE_EXT_SRV_ADD_MONEY, *((short*)packetData), 0, "External command: Add money"))
					printf("[NETCTRL {CMD_SRV_ADD_MONEY} ] IB ERROR: %s\n", db->lastErrorMessage);
				break;
			case CMD_SRV_RELOAD_APP:
					settings->threadFlag.MainWatch = 0;
					if (settings->debugFlag.NetServer)
						printf("[NETCTRL] External command: Reload application ...\n");
				break;

			case CMD_SRV_DONWLOAD_CONFIG:
				DataLen = 5;
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD_SRV_DONWLOAD_CONFIG;
				cfgFileBufferSize = loadCfgFile(cfgFileBuffer);
				memcpy(&answer[5], cfgFileBuffer, cfgFileBufferSize);
				DataLen += cfgFileBufferSize;
				answer[DataLen] = getCrc(cfgFileBuffer, cfgFileBufferSize);
				DataLen++;
				if (settings->debugFlag.NetServer)
					printf("[NETCTRL] External command: download config file: data: %5d - packet: %5d - CRC: %02X\n", cfgFileBufferSize, DataLen, answer[DataLen-1]);
				send(myClient, answer, DataLen, 0);
				break;
			case CMD_SRV_UPLOAD_CONFIG:
				if (settings->debugFlag.NetServer)
					printf("[NETCTRL] External command: Upload application config ...\n");
				if (getCrc(packetData, DataLen-1) == packetData[DataLen-1])
					answer[5] = saveCfgFile(packetData, DataLen-1);
				else
				{
					answer[5] = 0xFF;
					if (settings->debugFlag.NetServer)
						printf("[NETCTRL] External command: Upload application config ERROR [%02X - %02X]\n", getCrc(packetData, DataLen), packetData[DataLen-1]);
				}
				DataLen = 6;
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD_SRV_UPLOAD_CONFIG;
				send(myClient, answer, DataLen, 0);
				break;
			case CMD_SRV_GET_DEV_TYPE:
				DataLen = 7;
				answer[0] = 0x02;
				answer[1] = 0xFF;
				*((WORD*)&answer[2]) = DataLen;
				answer[4] = CMD_SRV_GET_DEV_TYPE;
				if (settings->threadFlag.IntCommonThread && settings->threadFlag.ExtCommonThread)
					answer[5] = 0x03;
				else if (settings->threadFlag.IntCommonThread)
					answer[5] = 0x02;
				else if (settings->threadFlag.ExtCommonThread)
					answer[5] = 0x01;
				else
					answer[5] = 0xFF;
				answer[6] = 0x01;
				if (settings->debugFlag.NetServer)
					printf("[NETCTRL] External command: Get device type: %2d.%2d\n", answer[5], answer[5]);
				int size = send(myClient, answer, DataLen, 0);
				if (settings->debugFlag.NetServer)
				{
					if ( size < 0)
						printf("[NETCTRL] >> External command: Get device type: ERROR\n");
					else
						printf("[NETCTRL] >> External command: Get device type: Send %d bytes\n", size);
				}
				//break;
		}
	}
	externalCtrl = 0;
	printf("Connection closed [%d] ...\n", --globalThreadId);
	close(myClient);
	return (void*)0;
}

PI_THREAD(NetServerThread)
{
	Settings* 	settings 	= Settings::getInstance();		// Параметры приложения
	int mySocket;
	struct sockaddr_in addr;
	settings->threadFlag.NetServer = 1;

	mySocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mySocket < 0)
	{
		printf("Error: Socket error\n");
		settings->threadFlag.NetServer = 0;
		return (void*)0;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(globalNetServerPort);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int reuse = 1;
	setsockopt(mySocket, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
	if (bind(mySocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		printf("Error: Bind error\n");
		settings->threadFlag.NetServer = 0;
		return (void*)0;
	}

	listen(mySocket, 10);
	while (settings->threadFlag.NetServer)
	{
		settings->workFlag.NetServer = 0;
		globalInSock = accept(mySocket, NULL, NULL);
		printf("Connection accepted [%d] ...\n", ++globalThreadId);
		int nTimeout = 5000;
		setsockopt(globalInSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeout,sizeof(int));
		try
		{
			piThreadCreate(NetServerClientThread);
		}
		catch (...)
		{printf("[NETSERVER] Catch exception ....\n");}
		delay_ms(100);
	}
	return (void*)0;
}

NetServer::NetServer()
{
}

void NetServer::Init(Settings* settings)
{
	port = settings->netServerConfig.PortNumber;
}

bool NetServer::StartServer()
{
	printf("Start network server on port [%d] ...", port);
	globalNetServerPort = port;
	piThreadCreate(NetServerThread);
	delay_ms(1000);
	printf("done\n");
	return (settings->threadFlag.NetServer == 1);
}

void NetServer::StopServer()
{
	Settings* settings 	= Settings::getInstance();		// Параметры приложения
	settings->threadFlag.NetServer = 0;
	delay_ms(1000);
}
