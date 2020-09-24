#include "../main.h"

//!!! ВНИМАНИЕ, РЕАЛЬНО РАБОТАЕТ ТОЛЬКО ПОДОГРЕВ ВНЕШНЕГО БОКСА

//#pragma region Подсистема "Термальная защита"
PI_THREAD(ThermalThread)
{
	if (!settings->threadFlag.ThermalWatch) return (void*)0;

	char deviceCount;				// Количество найденых устройств
	char deviceList[10][25];		// Список устройств
	DIR *localDir;
	struct dirent *dir;

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] Thermal: Thermal monitor thread init";
	if (db->Log( 0, DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

    FILE* in;
    char buff[100];
    int errorCount = 0;

	if (settings->getEnabledDevice(DVC_RELAY_WARM_LINE))
	{
		int warmPinNum = settings->getPinConfig(DVC_RELAY_WARM_LINE, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, 0);
		}
	}
	while (settings->threadFlag.ThermalWatch)
	{
		if (errorCount > 10) { errorCount = 58; delay(120);}
		if (strlen(settings->thermalParam.portName) < 1)
		{
			if (settings->debugFlag.ThermalWatch)
				printf("[DEBUG] THERMAL: Error in thermal sensor path. Try detect ...\n");
			localDir = opendir("/sys/bus/w1/devices/w1_bus_master1/");
			if (localDir)
			{
				deviceCount = 0;
				memset(deviceList, 0, 250);
				while ((dir = readdir(localDir)) != NULL)
				{
					if ((dir->d_name[0] == '2') && (dir->d_name[1] == '8'))
					{
						strcpy(deviceList[deviceCount], dir->d_name);
						if (settings->debugFlag.ThermalWatch)
							printf("[DEBUG] THERMAL: Device %s detected.\n", deviceList[deviceCount]);
						deviceCount++;
					}
				}

				closedir(localDir);
			}
			if (deviceCount)
			{
				sprintf(settings->thermalParam.portName, "/sys/bus/w1/devices/w1_bus_master1/%s/w1_slave", deviceList[0]);
				if (settings->debugFlag.ThermalWatch)
					printf("[DEBUG] THERMAL: Using first device: %s\n", settings->thermalParam.portName);
			}
		}

		if ((in = fopen(settings->thermalParam.portName, "r"))==NULL)
        	{ delay(1); errorCount++; continue; }
    	while (fgets(buff, 100, in) != NULL)
    	{
			settings->workFlag.ThermalWatch = 0;
			char *pos = strstr(buff, "t=");
			if (pos != NULL)
			{
				pos += 2;
				float temp = atof(pos)/1000;
				status.extDeviceInfo.extCurrentTemp = (int)(temp-0.5);
				if (status.extDeviceInfo.extCurrentTemp >= settings->thermalParam.tempOff)
				{
					if (settings->getEnabledDevice(DVC_RELAY_WARM_LINE))
					{
						int warmPinNum = settings->getPinConfig(DVC_RELAY_WARM_LINE, 1);
						if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
							setGPIOState(warmPinNum, 0);
					}
					/// Убрана обработка реле внешнего бокса, для включения подогрева необходимо использовать
					/// устройство DVC_RELAY_WARM_LINE 
					/// [07.03.2018]
					//status.extDeviceInfo.relay_currentVal[settings->thermalParam.thermalRelay] = 0;
					//if (settings->threadFlag.IntCommonThread && settings->threadFlag.ExtCommonThread)
					//	status.intDeviceInfo.relay_currentVal[settings->thermalParam.thermalRelay] = 0;
				}
				
				if (status.extDeviceInfo.extCurrentTemp <= settings->thermalParam.tempOn)
				{
					if (settings->getEnabledDevice(DVC_RELAY_WARM_LINE))
					{
						int warmPinNum = settings->getPinConfig(DVC_RELAY_WARM_LINE, 1);
						if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
							setGPIOState(warmPinNum, 1);
					}
					/// Убрана обработка реле внешнего бокса, для включения подогрева необходимо использовать
					/// устройство DVC_RELAY_WARM_LINE 
					/// [07.03.2018]
					//status.extDeviceInfo.relay_currentVal[settings->thermalParam.thermalRelay] = 1;
					//if (settings->threadFlag.IntCommonThread && settings->threadFlag.IntCommonThread)
					//	status.intDeviceInfo.relay_currentVal[settings->thermalParam.thermalRelay] = 1;
				}
			}
	    }
	    fclose(in);
	    delay_ms(1000);
	}
	printf("[THERMAL]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
