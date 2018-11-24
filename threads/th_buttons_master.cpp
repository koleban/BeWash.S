#include "../main.h"

PI_THREAD(ButtonMasterWatch)
{
	/// Общие параметры
	Settings* settings = Settings::getInstance();
	/// Если ПОТОК запрещен, то завершаемся
	if (!(settings->threadFlag.ButtonMasterThread)) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	db->Log(DB_EVENT_TYPE_THREAD_INIT, 		0, 0, "[THREAD] Button (master mode): Button thread init");
	db->Log(DB_EVENT_TYPE_DVC_BUTTON_INIT, 	0, 0, "Button (master mode) panel device opened");

	if (settings->debugFlag.ButtonWatch)
		printf("[DEBUG] ButtonMasterWatch: Debug information is showed\n");

	int index;
	BYTE currentPin = 0;
	BYTE currentDeviceID = 0;

	///
	/// Установим первоначальные параметры кнопок
	/// (0 - 12 кнопку)
	///
	for (index = 0; index < 12; index++)
	{
		currentDeviceID = DVC_BUTTON01 + index;
		if (!settings->getEnabledDevice(currentDeviceID))
			continue;
		currentPin = settings->getPinConfig(currentDeviceID, 1);
		if (currentPin == 0xFF) continue;
		if (settings->commonParams.showDemo)
		{
			setPinModeMy(currentPin, PIN_OUTPUT);
			setGPIOState(currentPin, 1);
			delay_ms(150);
		}
		setGPIOState(currentPin, 0);
		setPinModeMy(currentPin, PIN_INPUT);
  		pullUpDnControl (currentPin, PUD_DOWN) ;
	}

	if (settings->debugFlag.ButtonMasterThread)
		printf("[DEBUG] ButtonMasterThread: Button subsystem init  [%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);

	while (settings->threadFlag.ButtonMasterThread)
	{
		int delayTime = 100;
		settings->workFlag.ButtonMasterThread = 0;

		int timeout = 100;

		if (status.extDeviceInfo.remote_currentBalance <= 0)
		{
			for (index = 0; index < 12; index++)
			{
				currentDeviceID = DVC_BUTTON01 + index;
				if (!settings->getEnabledDevice(currentDeviceID))
					continue;
				currentPin = settings->getPinConfig(currentDeviceID, 1);
				if (currentPin == 0xFF) continue;
				if (settings->commonParams.showDemo)
				{
					setPinModeMy(currentPin, PIN_OUTPUT);
					setGPIOState(currentPin, 1);
					delay_ms(150);
				}
				setGPIOState(currentPin, 0);
				setPinModeMy(currentPin, PIN_INPUT);
		  		pullUpDnControl (currentPin, PUD_DOWN) ;
			}
			continue;
		}

		///
		/// Проверяем все кнопки на нажатие
		///

		for (index = 0; index < 12; index++)
		{
			currentDeviceID = DVC_BUTTON01 + index;
			if (!settings->getEnabledDevice(currentDeviceID)) continue;
			currentPin = settings->getPinConfig(currentDeviceID, 1);
			if (currentPin == 0xFF) continue;

			setPinModeMy(currentPin, PIN_INPUT);
			if (getGPIOState(currentPin))
			{
				int timeout = 50;
				if (settings->debugFlag.ButtonMasterThread)
					printf("[DEBUG] ButtonMasterThread: Pressed state on %d button [PIN: %03d]\n", index, currentPin);
				db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, index, currentPin, "[ButtonMasterThread]: New button pressed");
				while((timeout-- > 0) && getGPIOState(currentPin)) { delayTime--; delay_ms(1); }
				if (timeout <= 0)
				{
					if (settings->debugFlag.ButtonMasterThread)
						printf("[DEBUG] ButtonMasterThread: Send ballance to device %d\n", index);
					db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, index, status.intDeviceInfo.money_currentBalance, "[ButtonMasterThread]: Send money to the remote controller");

					for (index = 0; index < 12; index++)
					{
						currentDeviceID = DVC_BUTTON01 + index;
						if (!settings->getEnabledDevice(currentDeviceID)) continue;
						currentPin = settings->getPinConfig(currentDeviceID, 1);
						if ((currentPin == 0xFF) || (currentPin == 0x00)) continue;
						setPinModeMy(currentPin, PIN_OUTPUT);
						setGPIOState(currentPin, 1);
					}

					delay_ms(1000);
					///
					// !!!!!!!!!!!!!!!!!!!!
					remoteCtrl[index].cmdWrite = 1;

					remoteCtrl[index].cmdWritePrm = 0x06;
					remoteCtrl[index].cmdWritePrm = remoteCtrl[index].cmdWritePrm << 16;
					remoteCtrl[index].cmdWritePrm = remoteCtrl[index].cmdWritePrm + status.intDeviceInfo.money_currentBalance;
					remoteCtrl[index].doCmd = 1;
					// !!!!!!!!!!!!!!!!!!!!
					///

					status.intDeviceInfo.money_currentBalance = 0;
					status.extDeviceInfo.remote_currentBalance = 0;
					break;
				}
				else
				{
					if (settings->debugFlag.ButtonMasterThread)
						printf("[DEBUG] ButtonMasterThread: Pressed state on %d button [PIN: %03d] - failed [%d ms]\n", index, currentPin, 50 - timeout);
					db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, index, 50 - timeout, "[ButtonMasterThread]: Button don't detected. Failed");
				}
			}
		}

		if (delayTime > 0) delay_ms(delayTime);

	}

	///
	/// Button first setup
	for (index = 0; index < 12; index++)
	{
		currentDeviceID = DVC_BUTTON01 + index;
		if (!settings->getEnabledDevice(currentDeviceID))
			continue;
		currentPin = settings->getPinConfig(currentDeviceID, 1);
		if (currentPin == 0xFF) continue;
		setPinModeMy(currentPin, PIN_OUTPUT);
		setGPIOState(currentPin, 1);
		delay_ms(50);
		setGPIOState(currentPin, 0);
		setPinModeMy(currentPin, PIN_INPUT);
  		pullUpDnControl (currentPin, PUD_DOWN) ;
	}

	db->Log(DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "Button panel device is closed");
	db->Close();
	printf("[DEBUG]: ButtonMasterThread: Thread is terminate.\n");
	return (void*)0;
}
