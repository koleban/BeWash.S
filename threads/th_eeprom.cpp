#include "../main.h"

#define EP_ADDR_DATE_TIME		0x0003	// 4
#define EP_ADDR_MONEY			0x0007	// 4
#define EP_ADDR_ENGWRKTIME		0x000B	// 4
#define EP_ADDR_WORKSPACE		0x000F	// 4
#define EP_ADDR_PRG_PRICE		0x0013	// 12x4 byte (48) 0x30 byte
#define EP_ADDR_PRG_BP_PRICE	0x0043	// 12x4 byte (48) 0x30 byte
#define EP_ADDR_LAST_PRG_OSMOS	0x0073	// 2

DWORD readFromEEPROM_DWORD(EEPROM* eeprom, WORD addr, BYTE* errorCode = NULL)
{
	BYTE errCode = 0;
	DWORD res = 0;
	int timeout = 20;
	int val = -1;
	while ((timeout--) && (val < 0))
	{
		val = eeprom->ReadByte(addr);
		if (val < 0) delay_ms(EEPROM_DELAY);
	}
	if (val >= 0)
		res |= val << 24;
	if (timeout <= 0) errCode = 1;
	//************************************
	val = -1;
	timeout = 20;
	while ((timeout--) && (val < 0))
	{
		val = eeprom->ReadByte(addr+1);
		if (val < 0) delay_ms(EEPROM_DELAY);
	}
	if (val >= 0)
		res |= val << 16;
	if (timeout <= 0) errCode = 2;
	//************************************
	val = -1;
	timeout = 20;
	while ((timeout--) && (val < 0))
	{
		val = eeprom->ReadByte(addr+2);
		if (val < 0) delay_ms(EEPROM_DELAY);
	}
	if (val >= 0)
		res |= val << 8;
	if (timeout <= 0) errCode = 3;
	//************************************
	val = -1;
	timeout = 20;
	while ((timeout--) && (val < 0))
	{
		val = eeprom->ReadByte(addr+3);
		if (val < 0) delay_ms(EEPROM_DELAY);
	}
	if (val >= 0)
		res |= val;
	if (timeout <= 0) errCode = 4;

	if (errorCode != NULL) *errorCode = errCode;

	//printf ("EEPROM READ: err: %d addr: %02X val: %08X\n", errCode, addr, res);

	return res;
}

BYTE writeToEEPROM_DWORD(EEPROM* eeprom, WORD addr, DWORD val)
{
	int timeout = 20;
	while ((timeout--) && (eeprom->WriteByte(addr, (val >> 24) & 0xFF) < 0))
		delay_ms(EEPROM_DELAY);
	if (timeout <= 0) return 1;
	timeout = 20;
	while ((timeout--) && (eeprom->WriteByte(addr+1, (val >> 16) & 0xFF) < 0))
		delay_ms(EEPROM_DELAY);
	if (timeout <= 0) return 2;
	timeout = 20;
	while ((timeout--) && (eeprom->WriteByte(addr+2, (val >> 8) & 0xFF) < 0))
		delay_ms(EEPROM_DELAY);
	if (timeout <= 0) return 3;
	timeout = 20;
	while ((timeout--) && (eeprom->WriteByte(addr+3, val & 0xFF) < 0))
		delay_ms(EEPROM_DELAY);
	if (timeout <= 0) return 4;
	return 0;
}

PI_THREAD(EepromThread)
{
	EEPROM* eeprom = EEPROM::getInstance();
	Settings* 		settings 	= Settings::getInstance();
	int firstRun = 1;
	int counter = 0;
	int osmosLastPrg = 0;

	memset (&eepromPrgPrice[0], 0, sizeof(eepromPrgPrice));

	while (settings->useEeprom)
	{
		if (!settings->useEeprom) {delay_ms(1000); continue;}
		counter++;
		//////////////
		/// EEPROM DATE TIME
		//////////////
		DWORD date_time_eeprom = readFromEEPROM_DWORD(eeprom, EP_ADDR_DATE_TIME);	// 1111 1111 - year 1111 - month 1111 1 - date 111 11 - hour 11 1111 - minute

		if ((counter > 15) && (date_time_eeprom > 0) && (date_time_eeprom < 0xFFFFFFFF))
		{
			counter = 0;
			int day = (date_time_eeprom >> 15) & 0x1F;
			int month = (date_time_eeprom >> 20) & 0x0F;
			int year = (date_time_eeprom >> 24);
			int hour = (date_time_eeprom >> 10) & 0x1F;
			int minute = (date_time_eeprom >> 4) & 0x3F;
			time_t currentTime;
			time(&currentTime);
			struct tm localTime;
  			localTime.tm_hour = hour; localTime.tm_min = minute; localTime.tm_sec = 0;
			localTime.tm_mon = month-1;  localTime.tm_mday = day; localTime.tm_year = year;
  			double df_second = difftime(currentTime,mktime(&localTime));
  			if (df_second > 600)
  			{
				time(&currentTime);
  				localTime = *localtime(&currentTime);
//				unsigned int ddd = (118 << 24) + (3 << 20) + (10 << 15) + (1 << 10) + (20 << 4);

  				date_time_eeprom = (localTime.tm_year << 24) + ((localTime.tm_mon+1) << 20) +
  					(localTime.tm_mday << 15) + (localTime.tm_hour << 10) + (localTime.tm_min << 4);
				writeToEEPROM_DWORD(eeprom, EP_ADDR_DATE_TIME, date_time_eeprom);
  			}
  			else if (df_second < -300)
  			{
  				char sysCmd[255];
  				sprintf(sysCmd, "sudo date -s \"%02d/%02d/%d %02d:%02d:00\"", month, day, 1900+year, hour, minute);
				if ((settings->useEepromDateTime) && (!settings->useHWClock)) {printf("EEPROM service: [DEBUG] Setting date and time from EEPROM"); system(sysCmd);}
  			}
		}

		//////////////
		/// EEPROM MONEY
		//////////////
		DWORD allMoney = readFromEEPROM_DWORD(eeprom, EP_ADDR_MONEY);

		if (firstRun)
		{
			if (allMoney == (DWORD)0xFFFFFFFF)
				allMoney = 0;
			status.intDeviceInfo.allMoney = allMoney;
		}

		if (allMoney != status.intDeviceInfo.allMoney)
		{
			allMoney = status.intDeviceInfo.allMoney;
			writeToEEPROM_DWORD(eeprom, EP_ADDR_MONEY, allMoney);
		}

		//////////////
		/// EEPROM ENGINE WORK TIME
		//////////////
		DWORD engWrkTime = readFromEEPROM_DWORD(eeprom, EP_ADDR_ENGWRKTIME);

		if (firstRun)
		{
			if (engWrkTime == (DWORD)0xFFFFFFFF)
				engWrkTime = 0;
			gEngineFullWorkTime = engWrkTime;
			printf ("[DEBUG] Read engine work time: %d sec\n", gEngineFullWorkTime);
		}

		if (gEngineFullWorkTime != engWrkTime)
		{
			engWrkTime = gEngineFullWorkTime;
			writeToEEPROM_DWORD(eeprom, EP_ADDR_ENGWRKTIME, engWrkTime);
		}

		//////////////
		/// EEPROM WORKSPACE DATE TIME
		//////////////
		DWORD wrkDateTime = readFromEEPROM_DWORD(eeprom, EP_ADDR_WORKSPACE);
		if (firstRun)
		{

			if ((wrkDateTime == (int)0xFFFFFFFF) || (wrkDateTime == 0))
			{
				time(&wrkOpenedDateTime);
				wrkDateTime = (DWORD)wrkOpenedDateTime;
			}
			wrkOpenedDateTime = (time_t)wrkDateTime;
			printf ("[DEBUG] Read workspace date & time: %08X\n", wrkOpenedDateTime);
			struct tm localTime = *localtime(&wrkOpenedDateTime);
			sprintf(status.intDeviceInfo.wrkOpened, "%02d.%02d.%02d %02d:%02d    ",
				localTime.tm_mday, localTime.tm_mon+1, localTime.tm_year - 100,
				localTime.tm_hour, localTime.tm_min);
		}

		if ((int)wrkOpenedDateTime != wrkDateTime)
		{
			wrkDateTime = (int)wrkOpenedDateTime;
			writeToEEPROM_DWORD(eeprom, EP_ADDR_WORKSPACE, wrkDateTime);
		}

		//////////////
		/// EEPROM LAST PRG OSMOS
		//////////////

		if (settings->threadFlag.OsmosThread)
		{
			BYTE errorCodeRead = 0;
			if (firstRun)
			{
				osmosLastPrg = readFromEEPROM_DWORD(eeprom, EP_ADDR_LAST_PRG_OSMOS, &errorCodeRead);
				printf("[EEPROM]: Read OSMOS last prg %d (err: %02X)\n", osmosLastPrg, errorCodeRead);
				if (osmosLastPrg > 6) osmosLastPrg = 2;
				status.extDeviceInfo.button_newEvent = osmosLastPrg;
				status.extDeviceInfo.button_lastEvent = osmosLastPrg;
				status.extDeviceInfo.button_currentLight = osmosLastPrg+1;
				status.intDeviceInfo.program_currentProgram = osmosLastPrg;
				status.intDeviceInfo.extPrgNeedUpdate = 1;
			}

			if ((readFromEEPROM_DWORD(eeprom, EP_ADDR_LAST_PRG_OSMOS, &errorCodeRead) != status.intDeviceInfo.program_currentProgram)
				&& (status.intDeviceInfo.program_currentProgram < 5))
			{
				osmosLastPrg = status.intDeviceInfo.program_currentProgram;
				printf ("EEPROM: Write osmosLastPrg %d !!!\n", osmosLastPrg);
				if (writeToEEPROM_DWORD(eeprom, EP_ADDR_LAST_PRG_OSMOS, osmosLastPrg) > 0)
					printf ("EEPROM: Write ERROR !!!\n");
			}
		}

		if ((int)wrkOpenedDateTime != wrkDateTime)
		{
			wrkDateTime = (int)wrkOpenedDateTime;
			writeToEEPROM_DWORD(eeprom, EP_ADDR_WORKSPACE, wrkDateTime);
		}

		if(settings->useEepromParams == 1)
		{
			DWORD summ;
			BYTE errorCodeRead = 0;
			DWORD prgPrice[16];
			memset(&prgPrice, 0, sizeof(prgPrice));

			if (firstRun)
			{
				printf("[EEPROM]: Load EEPROM program price\n");
				for (int i=0; i<9; i++)
				{
					prgPrice[i] = readFromEEPROM_DWORD(eeprom, (EP_ADDR_PRG_PRICE+i*4), &errorCodeRead);
					if (errorCodeRead == 0)
					{
						eepromPrgPrice[i] = prgPrice[i];
						printf("[EEPROM]: prg[%d] = %d\n", i, eepromPrgPrice[i]);
					}
				}
				summ = 0;
				for (int i=0; i<9; i++)
					summ += eepromPrgPrice[i];
				if (summ > 0)
					for (int i=0; i<9; i++)
						settings->progPrice[i+1] = eepromPrgPrice[i];
			}

			for (int i=0; i<9; i++)
				prgPrice[i] = readFromEEPROM_DWORD(eeprom, (EP_ADDR_PRG_PRICE+i*4), &errorCodeRead);
			for (int i=0; i<9; i++)
			if (eepromPrgPrice[i] != prgPrice[i])
			{
				writeToEEPROM_DWORD(eeprom, (EP_ADDR_PRG_PRICE + i*4), eepromPrgPrice[i]);
				delay_ms(200);
				DWORD tmpValE = readFromEEPROM_DWORD(eeprom, (EP_ADDR_PRG_PRICE+i*4), &errorCodeRead);
				if (errorCodeRead == 0)
				{
					prgPrice[i] = tmpValE;
					printf("[EEPROM]: Write new price PRG[%d] = %d (%d)\n", i+1, eepromPrgPrice[i], prgPrice[i]);
					settings->progPrice[i+1] = eepromPrgPrice[i];
				}
			}
		}

		firstRun = 0;
  		delay(10);
	}
	return (void*)0;
}
