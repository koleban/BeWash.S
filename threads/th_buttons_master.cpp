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

	if (settings->getEnabledDevice(DVC_BUTTON_COLLECTION))
	{
		setPinModeMy(settings->getPinConfig(DVC_BUTTON_COLLECTION, 1), PIN_INPUT);
		pullUpDnControl(settings->getPinConfig(DVC_BUTTON_COLLECTION, 1), PUD_DOWN);
	}

	if (settings->debugFlag.ButtonMasterThread)
		printf("[DEBUG] ButtonMasterThread: Button subsystem init  [%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);

	int last_collectionButton = status.extDeviceInfo.collectionButton;
	while (settings->threadFlag.ButtonMasterThread)
	{
		int delayTime = 100;
		settings->workFlag.ButtonMasterThread = 0;

		if (settings->getEnabledDevice(DVC_BUTTON_COLLECTION))
		{
			currentPin = settings->getPinConfig(DVC_BUTTON_COLLECTION, 1);
			int timeout = 50;
			while((timeout-- > 0) && getGPIOState(currentPin)) { delayTime--; delay_ms(1); }

			status.extDeviceInfo.collectionButton = (timeout < 1);
			if ((!last_collectionButton) && (status.extDeviceInfo.collectionButton))
				db->Log(DB_EVENT_TYPE_EXT_COLL_BUTTON, 0, 0, "[External]: Collection button pressed!");
			if ((settings->debugFlag.ButtonMasterThread) && (!last_collectionButton) && (status.extDeviceInfo.collectionButton))
				printf("[DEBUG] ButtonMasterThread: Collection button is TURN ON\n");
			if ((settings->debugFlag.ButtonMasterThread) && (last_collectionButton) && (!status.extDeviceInfo.collectionButton))
				printf("[DEBUG] ButtonMasterThread: Collection button is TURN OFF\n");
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
			settings->busyFlag.ButtonMasterThread--;
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

					int slaveId = index;
					
					for (index = 0; index < 12; index++)
					{
						currentDeviceID = DVC_BUTTON01 + index;
						if (!settings->getEnabledDevice(currentDeviceID)) continue;
						currentPin = settings->getPinConfig(currentDeviceID, 1);
						if ((currentPin == 0xFF) || (currentPin == 0x00)) continue;
						setPinModeMy(currentPin, PIN_OUTPUT);
						setGPIOState(currentPin, 1);
					}

					///
					// !!!!!!!!!!!!!!!!!!!!
					// Add to structure remoteCtrl information for send to slave ctrl other modbus protocole
					//
					printf ("Add to queue MODBUS command. index: %d devId: %d\n", slaveId, remoteCtrl[slaveId].devId);
					//
					// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					printf ("              READ COMMAND COUNTER\n");
					remoteCtrl[slaveId].cmdRead = 1;
					remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
					remoteCtrl[slaveId].doCmd = 1;
					int timeout = 1000;
					while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
					
					if (remoteCtrl[slaveId].cmdResult > 0xFFFF) break;
					unsigned int cmdCounter = ((unsigned int)remoteCtrl[slaveId].cmdResult);
					printf ("              COMMAND COUNTER VAL: %d\n", cmdCounter);
					//
					// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
					printf ("              WRITE COMMAND COUNTER\n");
					int sendedBal = status.intDeviceInfo.money_currentBalance;
					remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
					remoteCtrl[slaveId].cmdWrite = 1;
					remoteCtrl[slaveId].devImpVal[0] = (sendedBal / 10);	// 10 rur/imp
					remoteCtrl[slaveId].devImpVal[1] = (sendedBal % 10);	// 1  rur/imp
					remoteCtrl[slaveId].doCmd = 1;
					timeout = 1000;
					while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
					if ((timeout == 0) || (remoteCtrl[slaveId].cmdResult > 0xFFFF))
						printf ("              WRITE COMMAND ERROR\n");
					else
						printf ("              WRITE COMMAND OK\n");

					//
					// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					int retryCounter = 3;
					while ((remoteCtrl[slaveId].cmdResult > 0xFFFF) && (retryCounter-- > 0))
					{
						printf ("              READ COMMAND COUNTER LOOP [RETRY: %d]\n", 4 - retryCounter);
						remoteCtrl[slaveId].cmdRead = 1;
						remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
						remoteCtrl[slaveId].doCmd = 1;
						timeout = 1000;
						while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
					}
					
					if (remoteCtrl[slaveId].cmdResult > 0xFFFF) break;
					if ((cmdCounter == ((unsigned int)remoteCtrl[slaveId].cmdResult)) && (cmdCounter > 0)) break;
					printf ("              COMMAND COUNTER VAL: %d\n", cmdCounter);

					// Add information for print KKM documents
					// queueKkm->QueuePut( CashSumm, DON'T USED, DON'T USED, ServiceName); 
					//
					queueKkm->QueuePut(sendedBal, 0, 0, settings->kkmParam.ServiceName);
					// !!!!!!!!!!!!!!!!!!!!
					///

					status.intDeviceInfo.money_currentBalance -= sendedBal;
					status.extDeviceInfo.remote_currentBalance -= sendedBal;
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
