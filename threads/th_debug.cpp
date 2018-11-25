#include "../main.h"

//#pragma Отладочный модуль
PI_THREAD(DebugThread)
{
	int tmp_scr = 0;
	char scrl[] = {0x2D, 0x5C, 0x7C, 0x2F};
	time_t workTimer;
	workTimer = time(NULL);

	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.DebugThread) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] DEBUG: Debug thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	int scr_x = 0, scr_y = 0;
	int oscr_x = 0, oscr_y = 0;
	char buff[100];
	int bypassPinNum = settings->getPinConfig(DVC_SENSOR_BYPASS, 1);
	term_setattr(0);
	term_clear();
	while (settings->threadFlag.DebugThread)
	{
		settings->busyFlag.DebugThread++;
		settings->workFlag.DebugThread = 0;
		cur_saveattr();
		term_getwindowsize(&scr_x, &scr_y);
		if ((oscr_x != scr_x) || (oscr_y != scr_y))
		{
			term_setattr(0);
			term_clear();
			oscr_x = scr_x;
			oscr_y = scr_y;
		}
		//
		// print app name
		cur_gotoxy(0,0);
		term_setattr(1);
		term_setattr(37);
		printf("beWash: System v%2.2f | http://www.bewash.ru | +7 (861) 372-52-76", prgVer);
		term_eraseendline();

		//
		// print current time
		workTimer = time(NULL);
		cur_gotoxy(scr_x-32, 1);
		printf("%s", buff);
		term_setattr(32);
		printf("Program [%d] uptime: %3d:%02d:%02d", settings->commonParams.deviceId, ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
		term_setattr(31);
		if (tmp_scr++ > sizeof(scrl)-2) tmp_scr = 0;
		printf("%c", scrl[tmp_scr]);
		term_eraseendline();
		term_setattr(0);

		//
		// Thread state
		int dbg_pos = 2;
		term_setattr(37);
		cur_gotoxy(scr_x-23, dbg_pos++);
		printf("----------------------");
		term_setattr(30);
		term_setattr(47);
		cur_gotoxy(scr_x-23, dbg_pos++);
		printf("     Debug state      ");
		term_setattr(37);
		term_setattr(40);
		cur_gotoxy(scr_x-23, dbg_pos++);
		printf("----------------------");
		cur_gotoxy(scr_x-23, dbg_pos++);
		bool dbOpen = commonDb->IsOpened();
		if (dbOpen)	term_setattr(32);
		else term_setattr(31);
		printf("SQL Database      : %1d|", dbOpen);
		cur_gotoxy(scr_x-23, dbg_pos++);
		if (!settings->winterMode.winterMode)	term_setattr(32);
		else term_setattr(31);
		printf("Winter mode  : %1d(%5d)|", settings->winterMode.winterMode, winterModeActive);
		cur_gotoxy(scr_x-23, dbg_pos++);
		printf("----------------------");
		cur_gotoxy(scr_x-23, dbg_pos++);
		term_setattr(35);
		printf("External temp : %4d  ", status.intDeviceInfo.externalTemp);
		cur_gotoxy(scr_x-23, dbg_pos++);
		term_setattr(35);
		printf("Ext panel temp: %4d  ", status.extDeviceInfo.extCurrentTemp);
		term_setattr(32);
		cur_gotoxy(scr_x-23, dbg_pos++);
		printf("----------------------");
		cur_gotoxy(scr_x-23, dbg_pos++);
		printf("%s", iddqd);

		//
		// Current status
		int attr1 = 43, attr2 = 40,attr3 = 43, attr4 = 40;
		dbg_pos = 2;
		term_setattr(37);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("----------------------+");
		term_setattr(30);
		term_setattr(47);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("  Ext device status   ");
		term_setattr(40);
		term_setattr(37);
		printf("|");
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("----------------------+");  // 23 char
		term_setattr(0);
		cur_gotoxy(scr_x-46, dbg_pos++);
		if (status.extDeviceInfo.button_lastEvent+1 > 1) attr1 = 41;
		else attr1 = 40;
		if (status.extDeviceInfo.button_currentLight+1 > 1) attr3 = 41;
		else attr3 = 40;
		printf("BTN: \x1b[%dm%03X\x1b[%dm    LGTH: \x1b[%dm%03X\x1b[%dm |", attr1, status.extDeviceInfo.button_lastEvent+1, attr2, attr3, status.extDeviceInfo.button_currentLight+1, attr4);

		cur_gotoxy(scr_x-46, dbg_pos++);
		if (status.extDeviceInfo.rfid_cardPresent > 0) {attr1 = 41; attr3 = 41; }
		else { attr1 = 40; attr3 = 40; }
		printf("CRD: \x1b[%dm%1d\x1b[%dm  CRDN: \x1b[%dm%08X\x1b[%dm|", attr1, status.extDeviceInfo.rfid_cardPresent, attr2, attr3, getCardIDFromBytes(status.extDeviceInfo.rfid_incomeCardNumber), attr4);

		term_setattr(37);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("CRD: \x1b[%dm%1d\x1b[%dm LCRDN: \x1b[%dm%08X\x1b[%dm|", attr1, status.extDeviceInfo.rfid_cardPresent, attr2, attr3, getCardIDFromBytes(status.extDeviceInfo.rfid_prevCardNumber), attr4);

		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("TEXT ON EXT MON: %c%c%c%c |", status.extDeviceInfo.monitor_currentText[0], status.extDeviceInfo.monitor_currentText[1], status.extDeviceInfo.monitor_currentText[2], status.extDeviceInfo.monitor_currentText[3]);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("TEXT ON INT MON: %c%c%c%c |", status.intDeviceInfo.monitor_currentText[0], status.intDeviceInfo.monitor_currentText[1], status.intDeviceInfo.monitor_currentText[2], status.intDeviceInfo.monitor_currentText[3]);

		cur_gotoxy(scr_x-46, dbg_pos++);
		if (status.extDeviceInfo.remote_currentBalance > 0) attr1 = 41;
		else attr1 = 40;
		if (status.extDeviceInfo.remote_currentProgram > 1) attr3 = 41;
		else attr4 = 40;
		printf("RBAL: \x1b[%dm%5d\x1b[%dm  RPRG: \x1b[%dm%02X\x1b[%dm |", attr1, status.extDeviceInfo.remote_currentBalance, attr2, attr3, status.extDeviceInfo.remote_currentProgram, attr4);
		term_setattr(37);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("----------------------+");
		term_setattr(30);
		term_setattr(47);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("  Int device status   ");
		term_setattr(40);
		term_setattr(37);
		printf("|");
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("----------------------+");  // 23 char
		term_setattr(37);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("PRG: %2d - %c%c%c%c%c%c%c $(%2d)|", status.intDeviceInfo.program_currentProgram,
				settings->progName[status.intDeviceInfo.program_currentProgram][0],
				settings->progName[status.intDeviceInfo.program_currentProgram][1],
				settings->progName[status.intDeviceInfo.program_currentProgram][2],
				settings->progName[status.intDeviceInfo.program_currentProgram][3],
				settings->progName[status.intDeviceInfo.program_currentProgram][4],
				settings->progName[status.intDeviceInfo.program_currentProgram][5],
				settings->progName[status.intDeviceInfo.program_currentProgram][6],
				settings->progPrice[status.intDeviceInfo.program_currentProgram]);
		cur_gotoxy(scr_x-46, dbg_pos++);
		term_setattr(37);
		if (settings->progPrice[status.intDeviceInfo.program_currentProgram] > 4)
			printf("DISC: %3d%%   PRG:$(%2d)|", settings->discountSize,(long)(
				settings->progPrice[status.intDeviceInfo.program_currentProgram]
					- (settings->progPrice[status.intDeviceInfo.program_currentProgram] * (((double)settings->discountSize)/100))));
		else
		{
			printf("DISC: ");
			term_setattr(30);
		    term_setattr(43);
			printf("PRG PRICE IS LOW");
			term_setattr(40);
			printf(" |");
		}
		term_setattr(37);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("$:%6d rur RPM: %04d|", status.intDeviceInfo.allMoney, status.intDeviceInfo.engine_currentRpm);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("BAL: %5d I: %s |", status.intDeviceInfo.money_currentBalance, &idkfa[0]);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("----------------------+");  // 23 char
		// Relay 0 - 9
		int rval = 0;
		cur_gotoxy(scr_x-46, dbg_pos++);
		term_setattr(35);
		printf("I");
		term_setattr(32);
		printf(" 1 2 3 4 5 6 7 8 9 10|");
		int relayIndex = 0;
		term_setattr(34);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf(" ");
		for (relayIndex=0; relayIndex < 10; relayIndex++)
		{
			rval = status.intDeviceInfo.relay_currentVal[relayIndex];
			if (rval) term_setattr(43);	else term_setattr(40);
			printf(" %1d", rval);
		}
		printf(" |");
		//
		// Relay 10 - 19
		cur_gotoxy(scr_x-46, dbg_pos++);
		term_setattr(35);
		printf("E");
		term_setattr(32);
		printf(" 1 2 3 4 5 6 7 8 9 10|");
		relayIndex = 0;
		term_setattr(34);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf(" ");
		for (relayIndex=0; relayIndex < 10; relayIndex++)
		{
			rval = status.extDeviceInfo.relay_currentVal[relayIndex];
			if (rval) term_setattr(43);	else term_setattr(40);
			printf(" %1d", rval);
		}
		printf(" |");
		//
		term_setattr(37);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("----------------------+");
		term_setattr(30);
		term_setattr(47);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf(" Workspace DATE TIME  ");
		term_setattr(40);
		term_setattr(37);
		printf("|");
		term_setattr(0);
		cur_gotoxy(scr_x-46, dbg_pos++);
		printf("%s   |", status.intDeviceInfo.wrkOpened);  // 23 char

		//
		// Engine status
		dbg_pos = 2;
		term_setattr(37);
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("----------------+");
		term_setattr(30);
		if (engine->errorCode != 0)
			term_setattr(41);
		else
			term_setattr(47);
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("     ENGINE     ");
		term_setattr(40);
		term_setattr(37);
		printf("|");
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("----------------+");	// 17 chars
		if (engine->errorCode != 0)
			term_setattr(31);
		else
			term_setattr(0);

		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("errCode:    %4d", engine->errorCode);	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("needFrq:    %4d|", engine->needFreq);	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		if (engine->currFreq > 1) attr1 = 42;
		else attr1 = 40;
		printf("currFrq:    \x1b[%dm%4d\x1b[%dm|", attr1, engine->currFreq, attr2);	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		if (engine->engineRotates > 0) attr1 = 42;
		else attr1 = 40;
		printf("engineRotates: \x1b[%dm%1d\x1b[%dm|", attr1, engine->engineRotates, attr2);	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("resState:   %04X|", engine->regState);	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
			printf("wrkTime:%8d|", (unsigned long)((gEngineFullWorkTime+engine->workTimeSec)));	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		if (engine->bypassMode > 0) attr1 = 42;
		else attr1 = 40;
		printf("bypassMode: \x1b[%dm%04d\x1b[%dm|", attr1, engine->bypassMode, attr2);	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("bypass_V:[%2d] %2d|", bypassPinNum, (getGPIOState(bypassPinNum) == 0));	// 17 chars
		//
		// Device status
		CCBillDevice* 	billDevice 	= CCBillDevice::getInstance();
		CoinDevice* 	coinDevice1 	= CoinDevice::getInstance();
		RFIDDevice* 	rfidDevice = RFIDDevice::getInstance();
  		//CCTalkBillDevice* ccTalkBillDevice = CCTalkBillDevice::getInstance();

		term_setattr(37);
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("----------------+");
		term_setattr(30);
		term_setattr(47);
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf(" PERIPH STATUS  ");
		term_setattr(40);
		term_setattr(37);
		printf("|");
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("----------------+");	// 17 chars
//		if (!ccTalkBillDevice->IsOpened() && settings->threadFlag.MoneyCCTalkWatch)
//			term_setattr(31);
//		else
//			term_setattr(0);
//		cur_gotoxy(scr_x-63, dbg_pos++);
//		printf("CCTalk Bill Vl %1d", (!ccTalkBillDevice->IsOpened() && settings->threadFlag.MoneyCCTalkWatch));	// 17 chars

		if (!billDevice->IsOpened() && settings->threadFlag.MoneyWatch)
			term_setattr(31);
		else
			term_setattr(0);
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("Bill Validator %1d", (!billDevice->IsOpened() && settings->threadFlag.MoneyWatch));	// 17 chars

		if (!settings->threadFlag.CoinPulseWatch)
			term_setattr(31);
		else
			term_setattr(0);
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("Coin Pulse Act %1d", (!settings->threadFlag.CoinPulseWatch));	// 17 chars

		if (!rfidDevice->IsOpened() && settings->threadFlag.RFIDWatch)
			term_setattr(31);
		else
			term_setattr(0);
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("RFID Reader    %1d", (!rfidDevice->IsOpened() && settings->threadFlag.RFIDWatch));	// 17 chars

		if (status.extDeviceInfo.collectionButton)
			term_setattr(31);
		else
			term_setattr(0);
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("Collection BTN %1d", status.extDeviceInfo.collectionButton);	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("----------------+");	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("Serial: %08X", settings->digitalVector);	// 17 chars
		printf("|");
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("Link ERR: %6d", keyErrorCount);	// 17 chars
		printf("|");
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("----------------+");	// 17 chars
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("Queue log: %5d", queueLog->QueueCount);	// 17 chars
		printf("|");
		cur_gotoxy(scr_x-63, dbg_pos++);
		printf("Queue KKM: %5d", queueKkm->QueueCount);	// 17 chars
		printf("|");

		cur_loadattr();
		fflush(stdout);

		settings->busyFlag.DebugThread = 0;
		delay_ms(500);
	}
	db->Close();
	printf("[DEBUG]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
