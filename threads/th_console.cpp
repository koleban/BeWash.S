#include "../main.h"

//#pragma region Подсистема управления "КОНСОЛЬ УПРАВЛЕНИЯ"
PI_THREAD(ConsoleThread)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.ConsoleWatch) return (void*)0;
	char command[1024];
	NetClient* 		netClient	= new NetClient();
	while (settings->threadFlag.ConsoleWatch)
	{
		bool cmdYes = 0;
		int chIndex = 0;
		memset(command, 0x00, sizeof(command));
		BYTE t = (BYTE)mygetch();
		while (t > 0)
		{
			if (chIndex > 99) { chIndex = 0; command[1] = 0x00;}
			command[chIndex++] = t;
			t = (BYTE)mygetch();
			if (t == 0x0A) { t = 0; command[chIndex] = 0x00; }
			if (chIndex > 15) break;
			cmdYes = 1;
		}
		if (cmdYes)
		{
		if (strlen(command) < 4) continue;

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
		switch(*((DWORD*)(command)))
		{
			// pay1
			case 0x31796170:
				tmpIndex = proc_find("./bwpay");
				if (tmpIndex != -1)
				{
					sprintf(strTmp256, "sudo kill -s SIGUSR1 %d", tmpIndex);
					printf("%s\n", strTmp256);
					system(strTmp256);
				}
				break;
					payInfo.summ = 100;
					payInfo.deviceNum = 1;
					cp2utf("Оплата картой за услуги автомойки", payInfo.note);
					sprintf(payInfo.r_phone, "");
					sprintf(payInfo.r_email, "test@test.email");
					payInfo.inUse = 1;
				break;
			// payk
			case 0x6B796170:
				tmpIndex = proc_find("./bwpay");
				if (tmpIndex != -1)
				{
					sprintf(strTmp256, "sudo kill -s SIGUSR1 %d", tmpIndex);
					printf("%s\n", strTmp256);
					system(strTmp256);
				}
				break;
			// eprg
			case 0x67727065:
				for(tmpIndex = 0; tmpIndex < 9; tmpIndex++)
					eepromPrgPrice[tmpIndex] = 0;
				break;
			// chk1
			case 0x316B6863:
				sprintf(strTmp256, "%s (П:%d)", settings->kkmParam.ServiceName, 4);
				queueKkm->QueuePut(1, 0, 0, strTmp256);
				break;
			// chk5
			case 0x356B6863:
				sprintf(strTmp256, "%s (П:%d)", settings->kkmParam.ServiceName, 1);
				queueKkm->QueuePut(1, 0, 0, strTmp256);
				delay_ms(500);
				sprintf(strTmp256, "%s (П:%d)", settings->kkmParam.ServiceName, 2);
				queueKkm->QueuePut(0, 2, 0, strTmp256);
				delay_ms(500);
				sprintf(strTmp256, "%s (П:%d)", settings->kkmParam.ServiceName, 3);
				queueKkm->QueuePut(0, 2, 0, strTmp256);
				delay_ms(500);
				sprintf(strTmp256, "%s (П:%d)", settings->kkmParam.ServiceName, 4);
				queueKkm->QueuePut(1, 0, 0, strTmp256);
				delay_ms(500);
				sprintf(strTmp256, "%s (П:%d)", settings->kkmParam.ServiceName, 5);
				queueKkm->QueuePut(1, 0, 0, strTmp256);
				break;
			case 0x706C6568:
				printf("Console command list\n");
				printf("  mbus [read/write] [dev] [reg] [val]        - Read/write modbus device register\n");
				printf("  coll                                       - Turn on collection button on 2 second\n");
				printf("  add [sum]                                  - Add [sum] with device balance\n");
				printf("  prg [val]                                  - Turn on program number [val]\n");
				printf("  engw [val]                                 - Add [val] to full engine work time\n");
				printf("  engz                                       - Clear engne full work time. Set time to 0\n");
				printf("  chk1                                       - Add to queue KKM fiscal document on 1 rur\n");
				break;
			// mbus &
			case 0x7375626D:
				while ((command[chPointer] != 0x20) & (chPointer < strlen(command))) chPointer++;
				if (chPointer >= strlen(command)) break;
				while ((command[chPointer] == 0x20) & (chPointer < strlen(command))) chPointer++;
				if (chPointer >= strlen(command)) break;
				if (strlen(command) >= chPointer + 1)
					valTemp =  (WORD)(command[chPointer] << 8) + (WORD)(command[chPointer + 1] & 0x00FF);
				// CMD mbus. Check comman "re" - read or "wr" - write
				cmdType = 0;
				if (valTemp == 0x7265)
					cmdType = 1;
				if (valTemp == 0x7772)
					cmdType = 2;
				if (cmdType == 0)
					break;

				while ((command[chPointer] != 0x20) & (chPointer < strlen(command))) chPointer++;
				if (chPointer >= strlen(command)) break;
				while ((command[chPointer] == 0x20) & (chPointer < strlen(command))) chPointer++;
				if (chPointer >= strlen(command)) break;

				// check device number
				valTemp = (WORD) atoi(&command[chPointer]);
				if ((valTemp > 0) && (valTemp <= 30))
				{
					if (cmdType == 1)
						sprintf(strTemp, "%s", "READ");
					if (cmdType == 2)
						sprintf(strTemp, "%s", "WRITE");
					printf ("[Modbus command] %s Device: %d\n", strTemp, valTemp);
				}
				else
					break;

				if (cmdType == 1)
				{
					remoteCtrl[valTemp].cmdRead = 1;
					remoteCtrl[valTemp].cmdReadPrm = 1;
					remoteCtrl[valTemp].doCmd = 1;
					valTemp2 = 1000;
					while ((remoteCtrl[valTemp].doCmd == 1) && (valTemp2-- > 0))
						delay_ms(10);
					if (valTemp2 > 0)
						printf ("Modbus command result: %08X\n", remoteCtrl[valTemp].cmdResult);
					break;
				}

				if (cmdType != 2) break;
				while ((command[chPointer] != 0x20) & (chPointer < strlen(command))) chPointer++;
				if (chPointer >= strlen(command)) break;
				while ((command[chPointer] == 0x20) & (chPointer < strlen(command))) chPointer++;
				if (chPointer >= strlen(command)) break;

				valTemp1 = (WORD) atoi(&command[chPointer]);

				while ((command[chPointer] != 0x20) & (chPointer < strlen(command))) chPointer++;
				if (chPointer >= strlen(command)) break;
				while ((command[chPointer] == 0x20) & (chPointer < strlen(command))) chPointer++;
				if (chPointer >= strlen(command)) break;

				valTemp2 = (WORD) atoi(&command[chPointer]);
				printf ("            Reg: %04X Val: %04X\n", valTemp1, valTemp2);
				remoteCtrl[valTemp].cmdWrite = 1;
				remoteCtrl[valTemp].cmdWritePrm = ((DWORD)valTemp1) << 16 + valTemp2;
				remoteCtrl[valTemp].doCmd = 1;
				valTemp2 = 1000;
				while ((remoteCtrl[valTemp].doCmd == 1) && (valTemp2-- > 0))
					delay_ms(10);
				if (valTemp2 > 0)
					printf ("Modbus command result: %08X\n", remoteCtrl[valTemp].cmdResult);


			break;
			// coll &
			case 0x6C6C6F63:
				printf ("Collect mode: [External command]\n");
				externalCmd_collectionButton = 1;
				status.extDeviceInfo.collectionButton = 1;
				deviceWorkMode = TDeviceWorkMode::CollectionMode;
				delay(5);
				externalCmd_collectionButton = 0;
				status.extDeviceInfo.collectionButton = 0;
				deviceWorkMode = TDeviceWorkMode::WorkMode;
			break;
			// prg
			case 0x20677270:
				printf ("prg %d\n", atoi((char*)(command+4)));
				myNewPrg = atoi((char*)(command+4));
				if (settings->progEnabled[myNewPrg])
				{
					int mytimeout = 2000;
					while((settings->busyFlag.NetClient) && (mytimeout--)) {delay_ms(1);}
					if (!mytimeout)
						printf("[DEBUG] ConsoleThread: NetClient busy wait timeout\n");
					// Kolebanov
					// 27.07.2018
					// >>>
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
			break;
			// engw
			case 0x77676E65:
				printf ("engw %d\n", atoi((char*)(command+5)));
				myNewPrg = atoi((char*)(command+5));
				gEngineFullWorkTime += myNewPrg;
			break;
			// engz
			case 0x7A676E65:
				printf ("Engine time work - 0\n");
				gEngineFullWorkTime = 0;
			break;
			// add &
			case 0x20646461:
				bal = atoi((char*)(command+4));
				printf("Add money: %4d\n", bal);
				status.intDeviceInfo.money_currentBalance += bal;
				// TODO !!!!
				status.extDeviceInfo.remote_currentBalance += bal;
			break;
			default:
				printf ("%08X   %08X\n", *((DWORD*)(command)), *((DWORD*)(command+4)));
				printf("\n");
				memset(command, 0x00, sizeof(command));
			break;
		}
		}
	}
	printf("[CONSOLE]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
