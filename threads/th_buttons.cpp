#include "../main.h"

PI_THREAD(ButtonWatch)
{
	/// ����� ���������
	Settings* settings = Settings::getInstance();
	/// ���� ����� ��������, �� �����������
	if (!(settings->threadFlag.ButtonWatch)) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	db->Log(DB_EVENT_TYPE_THREAD_INIT, 		0, 0, "[THREAD] Button: Button thread init");
	db->Log(DB_EVENT_TYPE_DVC_BUTTON_INIT, 	0, 0, "Button panel device opened");

	if (settings->debugFlag.ButtonWatch)
		printf("[DEBUG] ButtonWatch: Debug information is showed\n");

	int index;
	BYTE currentPin = 0;
	BYTE lightDeviceID = 0;
	BYTE currentDeviceID = 0;
	status.extDeviceInfo.button_currentLight = 255;
	status.extDeviceInfo.button_lastEvent = 255;
	status.extDeviceInfo.button_newEvent = 100;

	if (settings->debugFlag.ButtonWatch)
		printf("[DEBUG] ButtonWatch: Check pin communication\n");
	BYTE wtMcp = 10;
	while (wtMcp > 3)
	{
		wtMcp = 0;
		for (index = 0; index < 13; index++)
		{
			currentDeviceID = DVC_BUTTON01 + index;
			if (!settings->getEnabledDevice(currentDeviceID))
				continue;
			currentPin = settings->getPinConfig(currentDeviceID, 1);
			if (currentPin == 0xFF) continue;
			setGPIOState(currentPin, 0);
			setPinModeMy(currentPin, PIN_INPUT);

  			if (getGPIOState(currentPin)) wtMcp++;
		}
		delay_ms(1000);
	}
	if (settings->debugFlag.ButtonWatch)
		printf("[DEBUG] ButtonWatch: Check pin communication ... OK\n");
	///
	/// ��������� �������������� ��������� ������
	/// (0 - 12 ������ + ������ ��������� 13)
	///
	for (index = 0; index < 13; index++)
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
	if (settings->debugFlag.ButtonWatch)
		printf("[DEBUG] ButtonWatch: Button subsystem init  [%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);

	///
	/// STUB FOR CORRECT AND BEAUTIFUL LIGHT
	/// ���� ����� ������� �������� ��������� ���������� ����
	///
	index = 0;
	currentDeviceID = DVC_BUTTON01 + index;
	if (settings->commonParams.showDemo)
	{
		bool eDev = settings->getEnabledDevice(currentDeviceID);
		currentPin = settings->getPinConfig(currentDeviceID, 1);
		if ((eDev) && (currentPin != 0xFF))
		{
			setPinModeMy(currentPin, PIN_OUTPUT);
			delay_ms(20);
			setGPIOState(currentPin, 1);
			delay_ms(150);
			setGPIOState(currentPin, 0);
			delay_ms(20);
			setPinModeMy(currentPin, PIN_INPUT);
		}
		delay_ms(1000);
	}

	///
	/// Configure COLLECT button
	///
	if (settings->getEnabledDevice(DVC_BUTTON_COLLECTION))
	{
		setPinModeMy(settings->getPinConfig(DVC_BUTTON_COLLECTION, 1), PIN_INPUT);
		pullUpDnControl(settings->getPinConfig(DVC_BUTTON_COLLECTION, 1), PUD_DOWN);
	}

	int last_collectionButton = status.extDeviceInfo.collectionButton;
	while (settings->threadFlag.ButtonWatch)
	{
		int delayTime = 100;
		settings->workFlag.ButtonWatch = 0;

		int timeout = 100;
		while ((timeout-- > 0) && (settings->busyFlag.ButtonWatch)) {delay_ms(1); continue;}
		if ((settings->debugFlag.ButtonWatch) && (timeout == 0)) { printf("[DEBUG] ButtonWatch: Timeout waiting thread busyFlag\n"); settings->busyFlag.ButtonWatch = 0;}

		settings->busyFlag.ButtonWatch++;

		if ((settings->getEnabledDevice(DVC_BUTTON_COLLECTION)) && (externalCmd_collectionButton == 0))
		{
			currentPin = settings->getPinConfig(DVC_BUTTON_COLLECTION, 1);
			int timeout = 50;
			while((timeout-- > 0) && getGPIOState(currentPin)) { delayTime--; delay_ms(1); }

			status.extDeviceInfo.collectionButton = (timeout < 1);
			if ((!last_collectionButton) && (status.extDeviceInfo.collectionButton))
				db->Log(DB_EVENT_TYPE_EXT_COLL_BUTTON, 0, 0, "[External]: Collection button pressed!");
			if ((settings->debugFlag.ButtonWatch) && (!last_collectionButton) && (status.extDeviceInfo.collectionButton))
				printf("[DEBUG] ButtonWatch: Collection button is TURN ON\n");
			if ((settings->debugFlag.ButtonWatch) && (last_collectionButton) && (!status.extDeviceInfo.collectionButton))
				printf("[DEBUG] ButtonWatch: Collection button is TURN OFF\n");
		}

		if (status.extDeviceInfo.collectionButton)
		{
			for (index = 0; index < 12; index++)
			{
				// Turn on light on all buttons for show user "Collection mode"
				if (!settings->getEnabledDevice(DVC_BUTTON01+index)) continue;
				setPinModeMy(settings->getPinConfig(DVC_BUTTON01+index, 1), PIN_OUTPUT);
				setGPIOState(settings->getPinConfig(DVC_BUTTON01+index, 1), 1);
			}
			last_collectionButton = status.extDeviceInfo.collectionButton;
			delay_ms(delayTime);
			settings->busyFlag.ButtonWatch--;
			continue;
		}

		if (last_collectionButton)
		{
			for (index = 0; index < 12; index++)
			{
				if (!settings->getEnabledDevice(DVC_BUTTON01+index)) continue;
				setPinModeMy(settings->getPinConfig(DVC_BUTTON01+index, 1), PIN_OUTPUT);
				setGPIOState(settings->getPinConfig(DVC_BUTTON01+index, 1), 0);
				setPinModeMy(settings->getPinConfig(DVC_BUTTON01+index, 1), PIN_INPUT);
			}
			last_collectionButton = status.extDeviceInfo.collectionButton;
		}

		///
		/// ��������� ��� ������ �� ������� � ������������� "externalInfo.lastButtonEvent"
		///
		lightDeviceID = DVC_BUTTON01 + (status.extDeviceInfo.button_currentLight);
		if (status.extDeviceInfo.button_newEvent == 255)
		{
			setGPIOState(settings->getPinConfig(lightDeviceID, 1), 0);
			setPinModeMy(settings->getPinConfig(lightDeviceID, 1), PIN_INPUT);
			status.extDeviceInfo.button_currentLight = 255;
			status.extDeviceInfo.button_lastEvent = 255;
			status.extDeviceInfo.button_newEvent = 100;
		}

		if (status.extDeviceInfo.button_newEvent < 99)
		{
			for (index = 0; index < 12; index++)
			{
				currentDeviceID = DVC_BUTTON01 + index;
				if (!settings->getEnabledDevice(currentDeviceID)) continue;
				currentPin = settings->getPinConfig(currentDeviceID, 1);
				if ((currentPin == 0xFF) || (index == status.extDeviceInfo.button_newEvent)) continue;
				setGPIOState(settings->getPinConfig(DVC_BUTTON01 + index, 1), 1);
				setPinModeMy(settings->getPinConfig(DVC_BUTTON01 + index, 1), PIN_INPUT);
			}
			setPinModeMy(settings->getPinConfig(DVC_BUTTON01 + status.extDeviceInfo.button_newEvent, 1), PIN_OUTPUT);
			setGPIOState(settings->getPinConfig(DVC_BUTTON01 + status.extDeviceInfo.button_newEvent, 1), 1);
			status.extDeviceInfo.button_currentLight = status.extDeviceInfo.button_newEvent;
			status.extDeviceInfo.button_lastEvent = status.extDeviceInfo.button_newEvent;
			status.extDeviceInfo.button_newEvent = 100;
		}

		int lastMyEvent = status.extDeviceInfo.button_currentLight;
		for (index = 0; index < 12; index++)
		{
			currentDeviceID = DVC_BUTTON01 + index;
			if (!settings->getEnabledDevice(currentDeviceID)) continue;
			currentPin = settings->getPinConfig(currentDeviceID, 1);
			if (currentPin == 0xFF) continue;

			if (lightDeviceID != currentDeviceID)
			{
				if (getGPIOState(currentPin))
				{
					int timeout = 50;
					if (settings->debugFlag.ButtonWatch)
						printf("[DEBUG] ButtonWatch: Pressed state on %d button [PIN: %03d]\n", index, currentPin);
					while((timeout-- > 0) && getGPIOState(currentPin)) { delayTime--; delay_ms(1); }
					if (timeout <= 0)
					{
						setGPIOState(settings->getPinConfig(lightDeviceID, 1), 0);
						setPinModeMy(settings->getPinConfig(lightDeviceID, 1), PIN_INPUT);

						lightDeviceID = DVC_BUTTON01 + index;
						if (settings->debugFlag.ButtonWatch)
							printf("[DEBUG] ButtonWatch: Setting new programm %d --> %d\n", status.extDeviceInfo.button_lastEvent, index);
						status.extDeviceInfo.button_lastEvent = index;
						status.extDeviceInfo.button_currentLight = status.extDeviceInfo.button_lastEvent;
						if ((settings->debugFlag.ButtonWatch) && (status.extDeviceInfo.button_currentLight != lastMyEvent))
						{
							printf("[DEBUG] ButtonWatch: New button pressed - %d [%3d:%02d:%02d]\n", status.extDeviceInfo.button_currentLight, ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
							db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, status.extDeviceInfo.button_currentLight, status.extDeviceInfo.button_currentLight, "[External]: New button pressed");
							lastMyEvent = status.extDeviceInfo.button_currentLight;
						}
						setPinModeMy(currentPin, PIN_OUTPUT);
						setGPIOState(currentPin, 1);
					}
				}
			}
			// ������ �� ����������� ������. ��������!!!
			// >>>>
			else
			{
				if (!status.extDeviceInfo.collectionButton && (status.extDeviceInfo.remote_currentBalance > 0))
				{
					setPinModeMy(currentPin, PIN_OUTPUT);
					setGPIOState(currentPin, 1);
				}
			}
			// <<<<<<
		}
		settings->busyFlag.ButtonWatch--;

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
	printf("[DEBUG]: ButtonWatch: Thread is terminate.\n");
	return (void*)0;
}
