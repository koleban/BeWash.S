#include "../main.h"

/// ERROR CODE DICTIONARY
///
///		100	-	ETH0 - Link is down
///		101	-	Connection error
///		102	-	GetDeviceInfo error
///		103	-	Reconnect ...
///		104	-	SetNewExtPrg error
///		105	-	Unknown error

//#pragma region Ïîäñèñòåìà "ÂÍÓÒÐÅÍÍÈÉ ÁÎÊÑ: ÑÅÒÅÂÎÉ ÎÁÌÅÍ Ñ ÂÍÅØÍÈÌ ÁÎÊÑÎÌ"

PI_THREAD(NetClientThread)
{
	int iter = 0;
	int result = 0;
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.NetClient) return (void*)0;
	NetClient* 		netClient	= new NetClient();
	netClient->Init(settings);
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] NetClient: NetClient thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	while (settings->threadFlag.NetClient)
	{
		if (!CheckLink(settings->ethName)) { netClient->CloseConnection(); settings->intErrorCode.NetClient = 100; delay_ms(1000); db->Log(DB_EVENT_TYPE_NET_LINKDOWN, 0, status.intDeviceInfo.money_currentBalance, "[Net Client]: ERROR Interface eth0: link down ..."); break;}
		result = (int)netClient->OpenConnection();
		if (!netClient->isConnected)
			settings->intErrorCode.NetClient = 101;
		if (!netClient->isConnected)
			db->Log(DB_EVENT_TYPE_NET_RECONNECT, 0, status.intDeviceInfo.money_currentBalance, "[Net Client]: ERROR - Reconnect ...");
		else
			db->Log(DB_EVENT_TYPE_NET_CONNECTED, 0, status.intDeviceInfo.money_currentBalance, "[Net Client]: Connected ... OK!");
		if (netClient->isConnected)
			printf("NetClient: Remote server - connected [OK]\n");
		while ((settings->threadFlag.NetClient) && (netClient->isConnected))
		{
			if (iter++ > 20)
				{ iter = 0; if (!CheckLink(settings->ethName)) { netClient->CloseConnection(); settings->intErrorCode.NetClient = 100; delay_ms(1000); continue;}}
			try{
			int timeout = 2000;
			int delayTime = 200;
			settings->workFlag.NetClient = 0;
			if (settings->intErrorCode.NetClient == 101)
				settings->intErrorCode.NetClient = 0x00;

			while ((settings->busyFlag.NetClient > 0) && (timeout-- > 0)) { delayTime--; settings->workFlag.NetClient = 0; delay_ms(1); }
			if (timeout < 50) printf("[DEBUG]: NetClient: Timeout detected!\n");

			settings->busyFlag.NetClient = 0;

			if (!status.intDeviceInfo.extPrgNeedUpdate)
			{
				settings->busyFlag.NetClient++;
				timeout = 2;
				while ((!netClient->cmdGetExtDeviceInfo()) && (timeout-- > 0)) { delayTime -= 100; delay_ms(100); }
				if (timeout == 0) { printf("[netClient]: Error cmdGetExtDeviceInfo()\n"); settings->intErrorCode.NetClient = 102; continue; }
				else settings->intErrorCode.NetClient = 0x00;
				settings->busyFlag.NetClient--;
				if (delayTime > 0) delay_ms(delayTime);
			}


			delayTime = 200;
			if (status.intDeviceInfo.extPrgNeedUpdate)
			{
				printf("[netClient]: SetExtPrg ...\n");
				settings->busyFlag.NetClient++;
				timeout = 2;
				while ((!netClient->cmdSetExtPrg()) && (timeout-- > 0)) { delayTime -= 100; delay_ms(100); printf("[NET] Resend command >>> \n");}
				if (timeout == 0) { printf("[netClient]: Error cmdSetExtPrg()\n"); settings->intErrorCode.NetClient = 104; continue; }
				else { settings->intErrorCode.NetClient = 0x00; status.intDeviceInfo.extPrgNeedUpdate = 0;}
				settings->busyFlag.NetClient--;
				if (delayTime > 0) delay_ms(delayTime);
			}

			delay_ms(500);
			} catch(...)
			{printf("NetClient: Catch exception ....\n"); settings->intErrorCode.NetClient = 105; delay_ms(1000);}
		}
		netClient->CloseConnection();
		if (settings->threadFlag.NetClient)
		{
			settings->intErrorCode.NetClient = 103;
			printf("NetClient: Reconnect to external panel server ....\n");
		}
		int tmp = 0;
		while (tmp++ < 2) { settings->workFlag.NetClient = 0; delay(1);}
	}
	printf("[CLIENT]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
/**/
/*******************************************************************************************/
/**/
