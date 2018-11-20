#include "../main.h"

//#pragma region Подсистема защиты

unsigned long answer;
unsigned long keyErrorCount = 0;
unsigned long keyOKCount = 0;

unsigned long getDeviceSerialNumber()
{
   FILE *f = fopen("/proc/cpuinfo", "r");
   if (!f) {
      return 0xAAABBBCC;
   }

   char line[256];
   long serial;
   while (fgets(line, 256, f)) {
      if (strncmp(line, "Serial", 6) == 0) {
         char serial_string[16 + 1];
         serial = strtoul(strcpy(serial_string, strchr(line, ':') + 10), NULL, 16);
      }
   }

   return (unsigned long)serial;
}

PI_THREAD(DigitalKey)
{
	Settings* settings = Settings::getInstance();
	settings->digitalVector 	= getDeviceSerialNumber();
	#ifdef __SECURITY_DEBUG_
	if (settings->debugFlag.MainWatch)
		printf("[DEBUG] SECURITY: Device serial: %08X\n", settings->digitalVector);
	#endif
	char note[100];
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[100];
	sprintf(myNote, "[THREAD] Hardware: Hardware thread init [%08X]", settings->digitalVector);
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	int i1 = (rand()&0x0F);
	int i2 = rand();
	int timeout = 15 + i1;
	while(1)
	{
		if (timeout < 1)
		{
			timeout = 15 + ((0x10+10*i1+i2) & 0x5F);
			for (int index=0; index < sizeof(settings->threadFlag); index++)
				*(((BYTE*)&settings->threadFlag)+index) = 0;
		}

		int fd = wiringPiI2CSetup(0x09);
		if (fd == -1) {keyErrorCount++; delay_ms(1000); timeout--; continue; }
		#ifdef __SECURITY_DEBUG_
		if (settings->debugFlag.MainWatch)
			printf("[DEBUG] SECURITY: Query key: %08X\n", settings->digitalVector);
		#endif

		for (int index=1; index < 35; index++)
			wiringPiI2CWriteReg8 (fd, index, 0);
		wiringPiI2CWriteReg8 (fd, 4, (settings->digitalVector) & 0xFF);
		wiringPiI2CWriteReg8 (fd, 3, (settings->digitalVector >> 8) & 0xFF);
		wiringPiI2CWriteReg8 (fd, 2, (settings->digitalVector >> 16) & 0xFF);
		wiringPiI2CWriteReg8 (fd, 1, (settings->digitalVector >> 24) & 0xFF);

		wiringPiI2CWriteReg8 (fd, 5, 0x53);
		wiringPiI2CWriteReg8 (fd, 6, 0x65);
		wiringPiI2CWriteReg8 (fd, 7, 0x72);
		wiringPiI2CWriteReg8 (fd, 8, 0x67);
		wiringPiI2CWriteReg8 (fd, 9, 0x65);
		wiringPiI2CWriteReg8 (fd, 10, 0x79);

		wiringPiI2CWriteReg8 (fd, 0, 0x01);
		close(fd);

		unsigned int dSize = 200;
		delay_ms(dSize);

		fd = wiringPiI2CSetup(0x09);
		if (fd == -1) {keyErrorCount++; delay_ms(1000); timeout--; continue; }

		answer = wiringPiI2CReadReg8   (fd, 1);
		answer = (answer << 8) + wiringPiI2CReadReg8 (fd, 2);
		answer = (answer << 8) + wiringPiI2CReadReg8 (fd, 3);
		answer = (answer << 8) + wiringPiI2CReadReg8 (fd, 4);
		#ifdef __SECURITY_DEBUG_
		if (settings->debugFlag.MainWatch)
			printf("[DEBUG] SECURITY: Read security answer: %08X\n", answer);
		#endif
		close(fd);


		if (!settings->getEnabledDevice(DVC_HARDWARE_KEY_PROTECT) &&
			(system("lsusb -d 0a89:0009 > /dev/null") != 0) &&
			((unsigned long)settings->digitalKey != (unsigned long)answer)
			)
		{
			#ifdef __SECURITY_DEBUG_
			if (settings->debugFlag.MainWatch)
				printf("[DEBUG] SECURITY: Q: %08X - A: %08X\n", settings->digitalKey, answer);
			#endif
			sprintf(note, "WARNING: Hardware error [%08X]-[%08X]!", settings->digitalVector, answer);
			db->Log(DB_EVENT_TYPE_HARDWARE_ERROR, (double)settings->digitalVector, (double)answer, note);
			keyErrorCount++;
			/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			int device = DVC_ERROR_LED_1;
			int value = 1;
			if (settings->getEnabledDevice(device))
			{
				int warmPinNum = settings->getPinConfig(device, 1);
				if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
				{
					setPinModeMy(warmPinNum, 0);
					pullUpDnControl (warmPinNum, PUD_DOWN) ;
					setGPIOState(warmPinNum, value);
				}
			}
			device = DVC_ERROR_LED_2;
			value = 1;
			if (settings->getEnabledDevice(device))
			{
				int warmPinNum = settings->getPinConfig(device, 1);
				if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
				{
					setPinModeMy(warmPinNum, 0);
					pullUpDnControl (warmPinNum, PUD_DOWN) ;
					setGPIOState(warmPinNum, value);
				}
			}
			device = DVC_ERROR_LED_3;
			value = 1;
			if (settings->getEnabledDevice(device))
			{
				int warmPinNum = settings->getPinConfig(device, 1);
				if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
				{
					setPinModeMy(warmPinNum, 0);
					pullUpDnControl (warmPinNum, PUD_DOWN) ;
					setGPIOState(warmPinNum, value);
				}
			}
			device = DVC_ERROR_LED_4;
			value = 1;
			if (settings->getEnabledDevice(device))
			{
				int warmPinNum = settings->getPinConfig(device, 1);
				if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
				{
					setPinModeMy(warmPinNum, 0);
					pullUpDnControl (warmPinNum, PUD_DOWN) ;
					setGPIOState(warmPinNum, value);
				}
			}
			delay_ms(300);
			/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			device = DVC_ERROR_LED_1;
			value = 0;
			if (settings->getEnabledDevice(device))
			{
				int warmPinNum = settings->getPinConfig(device, 1);
				if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
				{
					setPinModeMy(warmPinNum, 0);
					pullUpDnControl (warmPinNum, PUD_DOWN) ;
					setGPIOState(warmPinNum, value);
				}
			}
			device = DVC_ERROR_LED_2;
			value = 0;
			if (settings->getEnabledDevice(device))
			{
				int warmPinNum = settings->getPinConfig(device, 1);
				if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
				{
					setPinModeMy(warmPinNum, 0);
					pullUpDnControl (warmPinNum, PUD_DOWN) ;
					setGPIOState(warmPinNum, value);
				}
			}
			device = DVC_ERROR_LED_3;
			value = 0;
			if (settings->getEnabledDevice(device))
			{
				int warmPinNum = settings->getPinConfig(device, 1);
				if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
				{
					setPinModeMy(warmPinNum, 0);
					pullUpDnControl (warmPinNum, PUD_DOWN) ;
					setGPIOState(warmPinNum, value);
				}
			}
			device = DVC_ERROR_LED_4;
			value = 0;
			if (settings->getEnabledDevice(device))
			{
				int warmPinNum = settings->getPinConfig(device, 1);
				if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
				{
					setPinModeMy(warmPinNum, 0);
					pullUpDnControl (warmPinNum, PUD_DOWN) ;
					setGPIOState(warmPinNum, value);
				}
			}
			keyOKCount=0;
		}
		else
		{
			keyErrorCount = 0;
			keyOKCount++;
			dSize = 300*keyOKCount;
			delay_ms(dSize);
		}
		if (keyErrorCount > 5)
		{
			sprintf(note, "ERROR: Hardware error! Process terminate.");
			db->Log(DB_EVENT_TYPE_HARDWARE_ERROR, (double)settings->digitalVector, (double)answer, note);
			settings->threadFlag.MainWatch = 0;
			memset(&settings->threadFlag, 0, sizeof(settings->threadFlag));
			//system("aplay /home/pi/bewash/snd/police_s.wav");
			delay(10);
		}

		dSize = 500;
		delay_ms(dSize);
	}
	return (void*)0;
}
//#pragma endregion
