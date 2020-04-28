#include "main.h"

NetClient::NetClient()
{
}

void NetClient::Init(Settings* settings)
{
	isConnected = 0;
	memset(hostAddr, 0, sizeof(hostAddr));
	port = settings->extPanelNetConfig.PortNumber;
	memcpy(hostAddr, settings->extPanelNetConfig.netServerAddr, sizeof(settings->extPanelNetConfig.netServerAddr));
}

bool NetClient::OpenConnection()
{
	if (isConnected) return isConnected;
	isConnected = 0;
	Settings* settings 	= Settings::getInstance();		// Параметры приложения
	sock = socket(AF_INET, SOCK_STREAM, 0);
	signal(SIGPIPE, SIG_IGN);
	if(sock < 0) return 0;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); // или любой другой порт...
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_addr.s_addr = inet_addr(hostAddr);
	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) return 0;
	isConnected = 1;
	return 1;
}

void NetClient::CloseConnection()
{
	Settings* settings 	= Settings::getInstance();		// Параметры приложения
	if (isConnected) 	isConnected = 0;
	close(sock);
}

bool NetClient::cmdGetExtDeviceInfo()
{
	Settings* settings 	= Settings::getInstance();		// Параметры приложения
	if (!isConnected) OpenConnection();
	status.extDeviceInfo.isReset = 0;

	ssize_t result = netSendData(sock, CMD_GET_DEVICE_INFO, (BYTE*)&status, sizeof(status));
	if (result < 0) { CloseConnection(); return 0; }

	BYTE in_buff[0xFFFF];
	WORD index = 0;
	index = netReadData(sock, &in_buff[0], 0xFFF0);

	if (index <= 0) { CloseConnection(); return 0; }

	if (index > 5)
	{
		BYTE* SYNC 	= (BYTE*)&in_buff[0];
		BYTE* ADR 	= (BYTE*)&in_buff[1];
		WORD* LEN 	= (WORD*)&in_buff[2];
		BYTE* CMD 	= (BYTE*)&in_buff[4];
		BYTE* DATA 	= (BYTE*)&in_buff[5];
		DeviceInfo* deviceInfo = (DeviceInfo*)&in_buff[5];

		if ((*SYNC != 0x02) || (*ADR != 0xFF)) return 0;

		if (status.intDeviceInfo.isReset)
		{
    		DWORD	objectId = status.intDeviceInfo.objectId;                   // ИД Объекта - Мойка
    		DWORD	deviceId = status.intDeviceInfo.deviceId;                   // ИД Устройства на объекте
    		DWORD	userId = status.intDeviceInfo.userId;                     	// ИД Пользователя
    		DWORD	serialNum = status.intDeviceInfo.serialNum;                 // Серийный номер устройства
			DWORD	allMoney = status.intDeviceInfo.allMoney;                   // Всего денег
			char	wrkOpened[30];												// Текст: Когда открыта текущая смена
			memcpy(&wrkOpened, &status.intDeviceInfo.wrkOpened, 30);

			memcpy(&status.intDeviceInfo, &deviceInfo->intDeviceInfo, sizeof(status.intDeviceInfo));
			status.intDeviceInfo.isReset = 0;
			status.intDeviceInfo.money_moneyCoinUpdate = 1;
    		status.intDeviceInfo.objectId	= objectId;                   // ИД Объекта - Мойка
    		status.intDeviceInfo.deviceId	= deviceId;                   // ИД Устройства на объекте
    		status.intDeviceInfo.userId	= userId;                     	// ИД Пользователя
    		status.intDeviceInfo.serialNum	= serialNum;                 // Серийный номер устройства
			status.intDeviceInfo.allMoney	= allMoney;                   // Всего денег
			memcpy(&status.intDeviceInfo.wrkOpened, &wrkOpened, 30);

			printf("[DEBUG] NetClient Load ExtBox status state!\n");
		}

		if (!deviceInfo->extDeviceInfo.isReset)
		{
			int countWait = 0;
			while (settings->busyFlag.IntCommonThread) {delay_ms(1); if (countWait++ > 20000) {settings->busyFlag.IntCommonThread = 0; break;} }
			memcpy(&status.extDeviceInfo, &deviceInfo->extDeviceInfo, sizeof(status.extDeviceInfo));
		}
		else
		{
			status.intDeviceInfo.money_moneyCoinUpdate = 2;
			printf("[DEBUG] NetClient ExtBox is RESET status!\n");
			delay_ms(100);
		}
    }

	return 1;
}

bool NetClient::cmdSetExtPrg()
{
	Settings* settings 	= Settings::getInstance();		// Параметры приложения
	if (!isConnected) OpenConnection();

	ssize_t result = netSendData(sock, CMD_SET_EXT_PRG, (BYTE*)&status, sizeof(status));
	if (result < 0) { CloseConnection(); return 0; }

	BYTE in_buff[0xFFFF];
	WORD index = 0;
	index = netReadData(sock, &in_buff[0], 0xFFF0);

	if (index <= 0) { CloseConnection(); return 0; }

	if (index > 5)
	{
		BYTE* SYNC 	= (BYTE*)&in_buff[0];
		BYTE* ADR 	= (BYTE*)&in_buff[1];
		WORD* LEN 	= (WORD*)&in_buff[2];
		BYTE* CMD 	= (BYTE*)&in_buff[4];
		BYTE* DATA 	= (BYTE*)&in_buff[5];
		DeviceInfo* deviceInfo = (DeviceInfo*)&in_buff[5];

		if ((*SYNC != 0x02) || (*ADR != 0xFF)) return 0;
		if (settings->debugFlag.NetClient)
			printf("[NETCTRL] Setting new external prg >>> \n");
    }

	return 1;
}

bool NetClient::cmdSendBalance(int valBalance)
{
	Settings* settings 	= Settings::getInstance();		// Параметры приложения
	if (!isConnected) OpenConnection();

	int recvBalance = 0;
	int zeroBalance = 0;
	int nTimeout = 1000;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeout,sizeof(int));
	ssize_t result = netSendData(sock, CMD_SEND_BALANCE, (BYTE*)&zeroBalance, sizeof(zeroBalance));
	if (result < 0) { CloseConnection(); printf("[NetClient::cmdSendBalance] Error 0x01\n"); return 0; }

	BYTE in_buff[0xFFFF];
	WORD index = 0;
	index = netReadData(sock, &in_buff[0], 0xFFF0);

	if (index <= 5) { delay_ms(20); netReadData(sock, &in_buff[0], 0xFFF0); CloseConnection(); printf("[NetClient::cmdSendBalance] Error 0x02\n"); return 0; }

	if (index > 5)
	{
		BYTE* SYNC 	= (BYTE*)&in_buff[0];
		BYTE* ADR 	= (BYTE*)&in_buff[1];
		WORD* LEN 	= (WORD*)&in_buff[2];
		BYTE* CMD 	= (BYTE*)&in_buff[4];
		BYTE* DATA 	= (BYTE*)&in_buff[5];
		DeviceInfo* deviceInfo = (DeviceInfo*)&in_buff[5];

		if ((*SYNC != 0x02) || (*ADR != 0xFF)) return 0;
		if (settings->debugFlag.NetClient)
			printf("[NetClient::cmdSendBalance] Sending balance >>> \n");
		if (deviceInfo)
			printf("[NetClient::cmdSendBalance] Receive remote balance. Remote balance: %d rur\n", deviceInfo->intDeviceInfo.money_currentBalance);
		recvBalance = deviceInfo->intDeviceInfo.money_currentBalance;
    }

	netSendData(sock, CMD_SEND_BALANCE, (BYTE*)&valBalance, sizeof(valBalance));

	delay_ms(20);
	index = netReadData(sock, &in_buff[0], 0xFFF0);

	if (index <= 5) 
	{ 
		delay_ms(20); 
		netReadData(sock, &in_buff[index], 0xFFF0-index); 
		CloseConnection(); 
		printf("[NetClient::cmdSendBalance] Error in connection. Reconnect ...\n");
		delay_ms(20); 
		OpenConnection();
		result = netSendData(sock, CMD_SEND_BALANCE, (BYTE*)&zeroBalance, sizeof(zeroBalance));
		index = netReadData(sock, &in_buff[0], 0xFFF0);

    	if (index <= 5) { delay_ms(20); netReadData(sock, &in_buff[0], 0xFFF0); CloseConnection(); printf("[NetClient::cmdSendBalance] Error 0x03\n"); return 0; }
	}

	if (index > 5)
	{
		BYTE* SYNC 	= (BYTE*)&in_buff[0];
		BYTE* ADR 	= (BYTE*)&in_buff[1];
		WORD* LEN 	= (WORD*)&in_buff[2];
		BYTE* CMD 	= (BYTE*)&in_buff[4];
		BYTE* DATA 	= (BYTE*)&in_buff[5];
		DeviceInfo* deviceInfo = (DeviceInfo*)&in_buff[5];

		if ((*SYNC != 0x02) || (*ADR != 0xFF)) {printf("[NetClient::cmdSendBalance] Error 0xFF\n"); return 0; }
		if (settings->debugFlag.NetClient)
			printf("[NETCTRL] Sending balance >>> \n");
		if ((deviceInfo->intDeviceInfo.money_currentBalance - valBalance) == recvBalance)
		{
			printf("[NetClient::cmdSendBalance] Sending balance OK. Remote balance: %d rur\n", deviceInfo->intDeviceInfo.money_currentBalance);
			return 1;
		}
		else
		{
			printf("Sending balance ERROR. RecvB: %d SendB: %d CurrB: %d\n", recvBalance, valBalance, deviceInfo->intDeviceInfo.money_currentBalance);
			int corrBal = (valBalance + recvBalance) - deviceInfo->intDeviceInfo.money_currentBalance;
			delay_ms(100);
			netSendData(sock, CMD_SEND_BALANCE, (BYTE*)&corrBal, sizeof(corrBal));
		}
    }

	return 1;
}
