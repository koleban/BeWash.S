#include "../main.h"

extern int waitSumm;
extern int paymentSumm;

//#pragma region Подсистема управления "КОНСОЛЬ УПРАВЛЕНИЯ"
PI_THREAD(ConsoleThread)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.ConsoleWatch) return (void*)0;
	char command[1024];
	char param[10][25];
	NetClient* 		netClient	= new NetClient();
	while (settings->threadFlag.ConsoleWatch)
	{
		bool cmdYes = 0;
		int chIndex = 0;
		memset(command, 0x00, sizeof(command));
		memset(param, 0x00, sizeof(param));
		if (fgets(command, 1024, stdin) == NULL) continue;
		if (strlen(command) < 4) continue;

		char * pch = strtok (command," ");
		pch = strtok (NULL, " ");
		while ((pch != NULL) && (chIndex++ < 10))
		{
			strcpy(param[chIndex-1], pch);
      		pch = strtok (NULL, " ");
		}

		for (int i = 0; i < chIndex-1; i++)
			printf ("param[%d]: %s\n", i, param[i]);

		int timeout = 0;
		int bal = 0;
		int myNewPrg = 0;
		int chPointer = 0;
		int cmdType = 0;
		int tmpIndex = 0;
		WORD valTemp = 0x0000;
		WORD valTemp1 = 0x0000;
		WORD valTemp2 = 0x0000;
		char strTemp[20];
		char strTmp256[1024];
		if (strstr(command, "help") == command)
		{
				printf("Console command list\n");
				printf("  clear eeprom price                         - Clear prg price in EEPROM\n");
				printf("  pay [summ]                                 - Start VISA payment on [summ]\n");
				printf("  pay kill                                   - Kill VISA payment thread\n");
				printf("  coll                                       - Turn on collection button on 2 second\n");
				printf("  chk [devNum]                               - New KKM doc on 1 rur for [devNum] device\n");
				printf("  add [sum]                                  - Add [sum] with device balance\n");
				printf("  prg [val]                                  - Turn on program number [val]\n");
				printf("  engw [val]                                 - Add [val] to full engine work time\n");
				printf("  engz                                       - Clear engne full work time. Set time to 0\n");
				printf("  reload config                              - Reload config file\n");
				printf("  block/unblock                              - Lock or unlock device\n");
		}

		if (strstr(command, "pay") == command)
		{
			waitSumm = 0;
			paymentSumm = atoi(param[0]);
			/*
			tmpIndex = proc_find("./bwpay");
			if (tmpIndex != -1)
			{
				sprintf(strTmp256, "sudo kill -s SIGUSR1 %d", tmpIndex);
				printf("%s\n", strTmp256);
				system(strTmp256);
			}
			if (strstr(command, "kill") != NULL) continue;
			payInfo.summ = atoi(param[0]);
			payInfo.deviceNum = atoi(param[1]);
			if ((payInfo.summ == 0) || (payInfo.deviceNum == 0)) continue;
			cp2utf("Оплата картой за услуги автомойки", payInfo.note);
			sprintf(payInfo.r_phone, "");
			sprintf(payInfo.r_email, "test@test.email");
			payInfo.inUse = 1;
			waitSumm = atoi(param[0]);
			*/
		}
		if (strstr(command, "reload config") == command)
		{
			char cfgFile[256];
			sprintf(cfgFile, "/home/pi/bewash/bewash.cfg");
			printf("   >>> load %s\n", cfgFile);
			settings->loadConfig(cfgFile);
		}
		if (strstr(command, "clear eeprom price") == command)
		{
			for(tmpIndex = 0; tmpIndex < 9; tmpIndex++)
				eepromPrgPrice[tmpIndex] = 0;
		}
			// chk1
		if (strstr(command, "chk") == command)
		{
			sprintf(strTmp256, "%s (П:%d)", settings->kkmParam.ServiceName, atoi(param[0]));
			queueKkm->QueuePut(1, 0, 0, strTmp256);
		}
		// Block
		if (strstr(command, "block") == command)
		{
			blockWork = 1;
		}
		if (strstr(command, "unblock") == command)
		{
			blockWork = 0;
		}
		if (strstr(command, "coll") == command)
		{
			printf ("Collect mode: [External command]\n");
			externalCmd_collectionButton = 1;
			status.extDeviceInfo.collectionButton = 1;
			deviceWorkMode = TDeviceWorkMode::CollectionMode;
			delay(5);
			externalCmd_collectionButton = 0;
			status.extDeviceInfo.collectionButton = 0;
			deviceWorkMode = TDeviceWorkMode::WorkMode;
		}
			// prg
		if (strstr(command, "prg") == command)
		{
				printf ("prg %d\n", atoi(param[0]));
				myNewPrg = atoi(param[0]);
				if (myNewPrg > 31) myNewPrg = 0;
				if (settings->progEnabled[myNewPrg])
				{
					int mytimeout = 2000;
					while((settings->busyFlag.NetClient) && (mytimeout--)) {delay_ms(1);}
					if (!mytimeout)
						printf("[DEBUG] ConsoleThread: NetClient busy wait timeout\n");
					int delayTime = 200;
		   			while ((delayTime-- > 0) && (settings->busyFlag.ButtonWatch)) {delayTime--; delay_ms(1); continue;}
   					settings->busyFlag.ButtonWatch++;
					status.extDeviceInfo.button_newEvent = myNewPrg;
					settings->busyFlag.ButtonWatch--;
					// <<<
					status.extDeviceInfo.button_lastEvent = myNewPrg;
					status.extDeviceInfo.button_currentLight = myNewPrg+1;
					status.intDeviceInfo.program_currentProgram = myNewPrg;
					status.intDeviceInfo.extPrgNeedUpdate = 1;
				}
		}
		// engw
		if (strstr(command, "engw") == command)
		{
			printf ("engw %d\n", atoi(param[0]));
			myNewPrg = atoi(param[0]);
			gEngineFullWorkTime += myNewPrg;
		}
		// engz
		if (strstr(command, "engz") == command)
		{
			printf ("Engine time work - 0\n");
			gEngineFullWorkTime = 0;
		}
		if (strstr(command, "add") == command)
		{
			bal = atoi(param[0]);
			printf("Add money: %4d\n", bal);
			status.intDeviceInfo.money_currentBalance += bal;
			status.extDeviceInfo.remote_currentBalance += bal;
		}
	}
	printf("[CONSOLE]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
