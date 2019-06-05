#include "main.h"

char prgNote[] = "beWash: Self carwash system. Hardware and software solution. Software developer: Serg V Kolebanov, hardware developer: Dmitry N Kondratev. http://www.bewash.ru mail:sale@bewash.ru support:dev@bewash.ru, koleban@gmail.com";
char prgProtect[] = "This application copy and clone protected. Hardware code key. Part of application business logic translate in hardware key. Guardant Net.Code";

extern char comports[30][60];
int licenseError = 0;
void signal_handler(int s);
float prgVer;
int prgBuild;
int exitStatus;
unsigned long gEngineFullWorkTime = 0;
int	wrkOpenedFlag = 1;
time_t wrkOpenedDateTime = (time_t)0;
QueueArray* queueLog = new QueueArray(10000);
QueueArray* queueKkm = new QueueArray(1000);
int stopWork = 0;
bool thButton = 0;
bool thDisplay = 0;

volatile bool detectInProgress = false;

int main(int argc, char *argv[])
{

	/////////////////////////
	///
	///
    prgVer = 2.07;
    prgBuild = 1040;
    ///
    ///
    /////////////////////////

    exitStatus = 0;
	char			fileCount;
	char			fileList[10][25];
	long			fileModTime[10];

	/*
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = signal_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGKILL, &sigIntHandler, NULL);
	sigaction(SIGTERM, &sigIntHandler, NULL);
	*/

	struct sigaction sigIntHandler;
	struct sigaction old_action;
	sigIntHandler.sa_handler = signal_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	
	sigaction (SIGINT, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGINT, &sigIntHandler, NULL);
	
	sigaction (SIGKILL, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGKILL, &sigIntHandler, NULL);

	sigaction (SIGTERM, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN)
		sigaction(SIGTERM, &sigIntHandler, NULL);


	unsigned long md5Hash[4];
	MD5One((unsigned char*)prgNote, sizeof(prgNote), (unsigned char*)md5Hash);

	// Init GPIO subsystem
	wiringPiSetup();

	char procFileName[] = "sudo /home/pi/bewash/bewashs\x0";

	pid_t pid = proc_find(procFileName);
	if (pid != -1)
	{
		printf("\n\nWARNING: BeWash applicarion already runing .... quit!\n");
		delay(1);
		exit(1);
	}

	printf ("Initialisating relay state");
	memset(status.intDeviceInfo.relay_currentVal, 0, sizeof(status.intDeviceInfo.relay_currentVal));
	memset(status.extDeviceInfo.relay_currentVal, 0, sizeof(status.extDeviceInfo.relay_currentVal));
	printf (" ... done.\n");

	memset(&Cport[0], 0, sizeof(Cport));

	winterModeActive = 0;
	winterModeEngineActive = 0;
	externalCmd_collectionButton = 0;
	status.extDeviceInfo.collectionButton = 0;

    prgStartTimer = time(NULL);
    winterCurrTime = prgStartTimer;

	settings 			= Settings::getInstance();

	DIR				*cfgDir;
	struct dirent 	*dir;
	cfgDir = opendir("/home/pi/bewash/");
	if (cfgDir)
	{
		fileCount = 0;
		memset(fileList, 0, 250);

		while ((dir = readdir(cfgDir)) != NULL)
		{
			int len = strlen(dir->d_name);
			if ( (dir->d_name[len-3] == 'c') && (dir->d_name[len-2] == 'f') && (dir->d_name[len-1] == 'g'))
			{
				strcpy(fileList[fileCount], dir->d_name);
				fileCount++;
			}
		}
		closedir(cfgDir);
	}
	for (int index=0; index < fileCount; index++ )
	{
		struct stat st;
		if (!stat(fileList[index], &st))
    		fileModTime[index] = (long)st.st_mtim.tv_sec;
	}

	// ����������
	if (fileCount == 0)
	{
		strcpy(fileList[0], "bewash.cfg");
		fileCount++;
	}
    if (argc > 1)
    {
    	settings->threadFlag.ConsoleWatch |= (strcmp(argv[1], "console") == 0);
    	settings->threadFlag.DebugThread |= (strcmp(argv[1], "debug") == 0);
    }

    if (argc > 2)
    {
    	settings->threadFlag.ConsoleWatch |= (strcmp(argv[2], "console") == 0);
    	settings->threadFlag.DebugThread |= (strcmp(argv[2], "debug") == 0);
    }

    if (settings->threadFlag.ConsoleWatch)
		printf ("WARNING!!!: Console activated ...\n");

	char settingFileName[100];
	printf ("Load configuration file:\n");
	for (int index=0; index < fileCount; index++ )
	{
		sprintf(settingFileName, "/home/pi/bewash/%s", fileList[index]);
		printf("   >>> %s\n", settingFileName);
		settings->loadConfig(settingFileName);
	}

	status.intDeviceInfo.objectId = settings->commonParams.objectId;
	status.intDeviceInfo.deviceId = settings->commonParams.deviceId;
	status.intDeviceInfo.userId = settings->commonParams.userId;
	status.extDeviceInfo.objectId = settings->commonParams.objectId;
	status.extDeviceInfo.deviceId = settings->commonParams.deviceId;
	status.extDeviceInfo.userId = settings->commonParams.userId;
	//sleep(1);

	// Init GPIO Extender
	printf ("Initialisating GPIO extender interface.\n");
  	try
  	{
		printf (" ... using MCP23017 .... loading i2c interface ");
  		if (settings->useMCP)
  		{
  			mcp23017Setup (100, 0x20);
  			for (int tmp1=0; tmp1<16; tmp1++)
  			{
  				setPinModeMy(100+tmp1, 0);
  				setGPIOState(100+tmp1, 0);
  			}
			printf (" OK \n");
  		}
	} catch(...)
	{
		settings->intErrorCode.MainWatch = 221;
		printf ("ERROR\n");
	}

	printf ("done.\n");

	// Init i2c EEPROM
	EEPROM* eeprom = EEPROM::getInstance();
	if (settings->useEeprom)
	{
		printf ("Initialisating EEPROM.\n");
	  	try
	  	{
			printf (" ... using EEPROM .... loading i2c interface ");
	  			if (eeprom->Init())
					printf (" OK \n");
				else
				{
					settings->useEeprom = 0;
					settings->useEepromDateTime = 0;
					settings->intErrorCode.MainWatch = 222;
					printf ("ERROR\n");
				}
		} catch(...)
		{
			settings->useEeprom = 0;
			settings->useEepromDateTime = 0;
			settings->intErrorCode.MainWatch = 222;
			printf ("ERROR\n");
		}
		printf ("done.\n");
	}
	piThreadCreate(DigitalKey);

	printf ("Initialisating database ... waiting database\n");

	piThreadCreate(ClearQueueLog);

	term_setattr(34);
	printf("Start thread for loading params from DB ... ");
	settings->threadFlag.MainWatch = 1;
	piThreadCreate(load_params_from_db);
	term_setattr(32);
	printf("done.\n");
	term_setattr(37);

	monitor				= Monitor::getInstance();
	relay				= RelaySwitch::getInstance();
	if (settings->engine_deviceType == 0)
		engine				= (Engine*)Engine9300::getInstance();
	else if (settings->engine_deviceType == 1)
		engine				= (Engine*)Engine8100::getInstance();
	else if (settings->engine_deviceType == 3)
		engine				= (Engine*)Engine8400::getInstance();
	else
		engine				= (Engine*)EngineEmu::getInstance();
#ifndef _RFID_DEVICE_CRT288K_
	rfid = RFIDDevice::getInstance();
#endif
#ifdef _RFID_DEVICE_CRT288K_
	rfid = Crt288KDevice::getInstance();
#endif
	coinDevice 			= CoinDevice::getInstance();
	coinPulseDevice 	= CoinPulseDevice::getInstance();
	billDevice 			= CCBillDevice::getInstance();

	netServer 			= new NetServer();

	status.extDeviceInfo.isReset = 1;
	status.intDeviceInfo.isReset = 1;

	char usbPath[25];
	for (int usbBus=1; usbBus < 3; usbBus++)
		for (int usbDev=1; usbDev < 15; usbDev++)
		{
			sprintf(usbPath, "/dev/bus/usb/%03d/%03d", usbBus, usbDev);
			usbreset(usbPath);
		}

	engine->Init(settings);
	monitor->Init(settings);
	relay->Init(settings);
	rfid->Init(settings);
	coinDevice->Init(settings);
	coinPulseDevice->Init(settings);
	billDevice->Init(settings);

	if (settings->threadFlag.RFIDWatch)
		rfid->Detect();

	if (settings->threadFlag.CoinWatch)
		coinDevice->Detect();

	if (settings->threadFlag.MoneyWatch)
		billDevice->Detect();

	// Blink LED
	piThreadCreate(start_blink);

	netServer->Init(settings);
	netServer->StartServer();


	piThreadCreate(ConsoleThread);
	piThreadCreate(DebugThread);

	piThreadCreate(MonitorWatch);
	piThreadCreate(ButtonWatch);

	piThreadCreate(RFIDWatch);
	piThreadCreate(CoinWatch);
	piThreadCreate(CoinPulseWatch);
	piThreadCreate(MoneyWatch);
	piThreadCreate(BV_MoneyWatch);
	piThreadCreate(EngineWatch);

	piThreadCreate(ExtCommonThread);
	piThreadCreate(IntCommonThread);
	piThreadCreate(TimeTickThread);

	piThreadCreate(DataExchangeThread);
	piThreadCreate(RFIDExchangeThread);

	piThreadCreate(NetClientThread);

	piThreadCreate(ThermalThread);

	piThreadCreate(DiscountWatch);
	piThreadCreate(gpioext_wd);

	piThreadCreate(EepromThread);

	piThreadCreate(Lcd20x4);
	piThreadCreate(RemoteCtrlWatch);
	piThreadCreate(ButtonMasterWatch);
	piThreadCreate(KKMWatch);

	piThreadCreate(VoiceWatch);

	piThreadCreate(RemoteCounterCtrlWatch);
	piThreadCreate(AlienDeviceWatch);

	piThreadCreate(OsmosWatch);

	piThreadCreate(ButtonTerminalWatch);

	settings->threadFlag.MainWatch = 1;
	int counter_for_live_massage = 0;
	sprintf(iddqd, "%s%d", "", 0);
	// Replace 5 sec on 2 sec for app starting faster
	delay_ms(2000);

	int detect_timeout = 0;
	while (settings->threadFlag.MainWatch)
	{
		sigIntHandler.sa_handler = signal_handler;
		sigemptyset(&sigIntHandler.sa_mask);
		sigIntHandler.sa_flags = 0;
		
		sigaction (SIGINT, NULL, &old_action);
		if (old_action.sa_handler != SIG_IGN)
			sigaction(SIGINT, &sigIntHandler, NULL);
		
		sigaction (SIGKILL, NULL, &old_action);
		if (old_action.sa_handler != SIG_IGN)
			sigaction(SIGKILL, &sigIntHandler, NULL);

		sigaction (SIGTERM, NULL, &old_action);
		if (old_action.sa_handler != SIG_IGN)
			sigaction(SIGTERM, &sigIntHandler, NULL);

		detect_timeout = 3000;
		while ((detect_timeout--) && (settings->busyFlag.DebugThread)) delay_ms(1);
		settings->workFlag.RFIDWatch++;
		settings->workFlag.CoinWatch++;
		settings->workFlag.CoinPulseWatch++;
		settings->workFlag.MoneyWatch++;
		settings->workFlag.MoneyCCTalkWatch++;
		settings->workFlag.EngineWatch++;
		settings->workFlag.ButtonWatch++;
		settings->workFlag.DataExchangeThread++;
		settings->workFlag.NetServer++;
		settings->workFlag.NetClient++;
		settings->workFlag.MonitorWatch++;
		settings->workFlag.ExtCommonThread++;
		settings->workFlag.IntCommonThread++;
		settings->workFlag.TimeTickThread++;
		settings->workFlag.DebugThread++;
		settings->workFlag.ThermalWatch++;
		delay_ms(1000);

		if (settings->workTimeDevice.UseWorkTime)
		{
			time_t currTime = time(NULL);
  			struct tm* timeInfo;
  			timeInfo = localtime(&currTime);
  			if 	(  ((timeInfo->tm_hour*60+timeInfo->tm_min)	>= (settings->workTimeDevice.StartTimeHour*60+settings->workTimeDevice.StartTimeMinute)) 
  				&& ((timeInfo->tm_hour*60+timeInfo->tm_min) <= (settings->workTimeDevice.StopTimeHour*60+settings->workTimeDevice.StopTimeMinute)))
  			{
  				if (stopWork != 0)
  				{
	  				printf ("Use WORK TIME!!!\n");
	  				printf ("Starting thread ...\n");
					stopWork = 0;
					settings->threadFlag.ButtonWatch = thButton;
					settings->threadFlag.MonitorWatch = thDisplay;
					piThreadCreate(MonitorWatch);
					piThreadCreate(ButtonWatch);
				}
			}
			else if (stopWork == 0)
			{
				stopWork = 1;
  				printf ("Use WORK TIME!!!\n");
  				printf ("Stoping thread ...\n");
				thButton = settings->threadFlag.ButtonWatch;
				thDisplay = settings->threadFlag.MonitorWatch;
				settings->threadFlag.ButtonWatch = 0;
				settings->threadFlag.MonitorWatch = 0;
			}
		}

		///
		/// Auto reload configuration after changing ...
		/// >>>>>>>>>>>>>>>
		if (settings->useAutoReloadConfig)
		for (int index=0; index < fileCount; index++ )
		{
			struct stat st;
			if (!stat(fileList[index], &st))
			{
	    		if (fileModTime[index] != (long)st.st_mtim.tv_sec)
	    		{
					//delay_ms(1000);
					sprintf(settingFileName, "/home/pi/bewash/%s", fileList[index]);
					printf("[WARNING] Configuration file is modifed! Reload configuration file!!! [%s]\n", fileList[index]);
					settings->loadConfig(settingFileName);
					if (!stat(fileList[index], &st))
		    			fileModTime[index] = (long)st.st_mtim.tv_sec;
		    	}
			}
		}
		/// <<<<<<<<<<<<<<<<

		// Chech device IP address. If IP address not configured
		// run system command. And add IP address @192.168.254.DevID@
		unsigned int addr[10];
		int count = getIpList(addr, 10);
		int ip_is_ok = 0;
		while (count-- > 0)
			if (((addr[count] >> 24) & 0xFF) == (settings->commonParams.deviceId & 0xFF)) ip_is_ok++;
		if (ip_is_ok == 0)
		{
			char devaddr[250];
			sprintf(devaddr, "sudo ifconfig eth0 add 192.168.254.%d netmask 255.255.255.0", settings->commonParams.deviceId);
			printf("Add new IP: %s\n", devaddr);
			system(devaddr);
		}
	}
	printf("Turn off realy ...");
	memset(status.intDeviceInfo.relay_currentVal, 0, sizeof(status.intDeviceInfo.relay_currentVal));
	memset(status.extDeviceInfo.relay_currentVal, 0, sizeof(status.extDeviceInfo.relay_currentVal));
	status.intDeviceInfo.program_currentProgram = 0;
	printf("OK\n");
	delay(2);

	printf("Turn off display ...");
	status.intDeviceInfo.program_currentProgram = 0;
	memset(status.intDeviceInfo.relay_currentVal, 0, sizeof(status.intDeviceInfo.relay_currentVal));
	memset(status.extDeviceInfo.relay_currentVal, 0, sizeof(status.extDeviceInfo.relay_currentVal));
	printf("OK \nKill all threads ...\n");
	memset(&settings->threadFlag, 0, sizeof(settings->threadFlag));
	settings->threadFlag.DebugThread++;
	delay(5);
	printf("[OK]\n");
	printf("=============================\n\nGoodbay. See you later!\n\n");

	return exitStatus;
}

void signal_handler(int s)
{
	printf("Signal: %d\n", s);

    printf("\n Pressed Ctrl+C. System stoping ... \n");
	exitStatus = 10;

	printf("Turn off realy ...");
	memset(status.intDeviceInfo.relay_currentVal, 0, sizeof(status.intDeviceInfo.relay_currentVal));
	memset(status.extDeviceInfo.relay_currentVal, 0, sizeof(status.extDeviceInfo.relay_currentVal));
	status.intDeviceInfo.program_currentProgram = 0;
	printf("OK\n");
	delay(2);

	printf("Turn off display ...");
	status.intDeviceInfo.program_currentProgram = 0;
	memset(status.intDeviceInfo.relay_currentVal, 0, sizeof(status.intDeviceInfo.relay_currentVal));
	memset(status.extDeviceInfo.relay_currentVal, 0, sizeof(status.extDeviceInfo.relay_currentVal));
	printf("OK \nKill all threads ...\n");
	memset(&settings->threadFlag, 0, sizeof(settings->threadFlag));
	settings->threadFlag.DebugThread++;

	delay(8);
	printf("[OK]\n");
	printf("=============================\n\nGoodbay. See you later!\n\n");

	settings->threadFlag.MainWatch = 0;
	exit(1);
}

int usbreset(char *argv)
{
	if (!settings->resetUsbDevice) return 0;
    const char *filename;
    int fd;
    int rc;

    filename = argv;

    fd = open(filename, O_WRONLY);
    if (fd < 0) {
        return 1;
    }

    rc = ioctl(fd, USBDEVFS_RESET, 0);
    if (rc < 0)
        return 1;
    else
    {
    	printf("Resetting USB device %s successful.\n", filename);
	}
    close(fd);
    return 0;
}