#include "../main.h"

#define EP_ADDR_DATE_TIME		0x0003
#define EP_ADDR_DATE_TIME_1		0x0004
#define EP_ADDR_DATE_TIME_2		0x0005
#define EP_ADDR_DATE_TIME_3		0x0006
#define EP_ADDR_MONEY_1			0x0007
#define EP_ADDR_MONEY_2			0x0008
#define EP_ADDR_MONEY_3			0x0009
#define EP_ADDR_MONEY_4			0x000A
#define EP_ADDR_ENGWRKTIME_1		0x000B
#define EP_ADDR_ENGWRKTIME_2		0x000C
#define EP_ADDR_ENGWRKTIME_3		0x000D
#define EP_ADDR_ENGWRKTIME_4		0x000E
#define EP_ADDR_WORKSPACE_1		0x000F
#define EP_ADDR_WORKSPACE_2		0x0010
#define EP_ADDR_WORKSPACE_3		0x0011
#define EP_ADDR_WORKSPACE_4		0x0012

PI_THREAD(EepromThread)
{
	EEPROM* eeprom = EEPROM::getInstance();
	Settings* 		settings 	= Settings::getInstance();
	int firstRun = 1;
	int counter = 0;
	while (settings->useEeprom)
	{
		if (!settings->useEeprom) {delay_ms(1000); continue;}
		counter++;
		//////////////
		/// EEPROM DATE TIME
		//////////////
		DWORD date_time_eeprom = 0;	// 1111 1111 - year 1111 - month 1111 1 - date 111 11 - hour 11 1111 - minute
		int val = -1;
		int timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_DATE_TIME);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if (val >= 0)
			date_time_eeprom |= val << 24;

 		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_DATE_TIME_1);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if (val >= 0)
			date_time_eeprom |= val << 16;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_DATE_TIME_2);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if (val >= 0)
			date_time_eeprom |= val << 8;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_DATE_TIME_3);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if (val >= 0)
			date_time_eeprom |= val;
		/////////////

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
				unsigned int ddd = (118 << 24) + (3 << 20) + (10 << 15) + (1 << 10) + (20 << 4);

  				date_time_eeprom = (localTime.tm_year << 24) + ((localTime.tm_mon+1) << 20) +
  					(localTime.tm_mday << 15) + (localTime.tm_hour << 10) + (localTime.tm_min << 4);
				timeout = 20;
				while ((timeout--) && (eeprom->WriteByte(EP_ADDR_DATE_TIME, (date_time_eeprom >> 24) & 0xFF) < 0))
					delay_ms(EEPROM_DELAY);
				timeout = 20;
				while ((timeout--) && (eeprom->WriteByte(EP_ADDR_DATE_TIME_1, (date_time_eeprom >> 16) & 0xFF) < 0))
					delay_ms(EEPROM_DELAY);
				timeout = 20;
				while ((timeout--) && (eeprom->WriteByte(EP_ADDR_DATE_TIME_2, (date_time_eeprom >> 8) & 0xFF) < 0))
					delay_ms(EEPROM_DELAY);
				timeout = 20;
				while ((timeout--) && (eeprom->WriteByte(EP_ADDR_DATE_TIME_3, date_time_eeprom & 0xFF) < 0))
					delay_ms(EEPROM_DELAY);
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
		DWORD allMoney = 0;
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_MONEY_1);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if (val >= 0) allMoney |= val << 24;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_MONEY_2);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if (val >= 0) allMoney |= val << 16;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_MONEY_3);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if (val >= 0)	allMoney |= val << 8;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_MONEY_4);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if (val >= 0) allMoney |= val;
		/////////////

		if (firstRun)
		{
			if (allMoney == (DWORD)0xFFFFFFFF)
				allMoney = 0;
			status.intDeviceInfo.allMoney = allMoney;
		}

		if (allMoney != status.intDeviceInfo.allMoney)
		{
			allMoney = status.intDeviceInfo.allMoney;
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_MONEY_1, (allMoney >> 24) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_MONEY_2, (allMoney >> 16) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_MONEY_3, (allMoney >> 8) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_MONEY_4, allMoney & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
		}

		//////////////
		/// EEPROM ENGINE WORK TIME
		//////////////
		DWORD engWrkTime = 0;
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_ENGWRKTIME_1);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if ((val >= 0) && (timeout > 0)) engWrkTime |= val << 24;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_ENGWRKTIME_2);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if ((val >= 0) && (timeout > 0)) engWrkTime |= val << 16;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_ENGWRKTIME_3);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if ((val >= 0) && (timeout > 0)) engWrkTime |= val << 8;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_ENGWRKTIME_4);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if ((val >= 0) && (timeout > 0)) engWrkTime |= val;
		/////////////

		if (firstRun)
		{
			if (engWrkTime == (DWORD)0xFFFFFFFF)
				engWrkTime = 0;
			gEngineFullWorkTime = engWrkTime;
			printf ("[DEBUG] Read engine work time: %d sec\n", gEngineFullWorkTime);
		}

		//DWORD GetTimeToEeprom(time_t currentTime)
		//time_t GetTimeFromEeprom(DWORD date_time_eeprom)

		if (gEngineFullWorkTime != engWrkTime)
		{
			engWrkTime = gEngineFullWorkTime;
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_ENGWRKTIME_1, (engWrkTime >> 24) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_ENGWRKTIME_2, (engWrkTime >> 16) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_ENGWRKTIME_3, (engWrkTime >> 8) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_ENGWRKTIME_4, engWrkTime & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
		}

		//////////////
		/// EEPROM WORKSPACE DATE TIME
		//////////////
		int wrkDateTime = 0;
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_WORKSPACE_1);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if ((val >= 0) && (timeout > 0)) wrkDateTime |= val << 24;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_WORKSPACE_2);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if ((val >= 0) && (timeout > 0)) wrkDateTime |= val << 16;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_WORKSPACE_3);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if ((val >= 0) && (timeout > 0)) wrkDateTime |= val << 8;
		/////////////
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = eeprom->ReadByte(EP_ADDR_WORKSPACE_4);
			if (val < 0) delay_ms(EEPROM_DELAY);
		}
		if ((val >= 0) && (timeout > 0)) wrkDateTime |= val;
		/////////////

		if (firstRun)
		{

			if ((wrkDateTime == (int)0xFFFFFFFF) || (wrkDateTime == 0))
			{
				time(&wrkOpenedDateTime);
				wrkDateTime = (int)wrkOpenedDateTime;
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
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_WORKSPACE_1, (wrkDateTime >> 24) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_WORKSPACE_2, (wrkDateTime >> 16) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_WORKSPACE_3, (wrkDateTime >> 8) & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
			timeout = 20;
			while ((timeout--) && (eeprom->WriteByte(EP_ADDR_WORKSPACE_4, wrkDateTime & 0xFF) < 0))
				delay_ms(EEPROM_DELAY);
		}

		firstRun = 0;
  		delay(5);
	}
	return (void*)0;
}
