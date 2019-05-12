#include "../main.h"

int btnMasterProgress = 0;

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

	if (settings->debugFlag.ButtonMasterThread)
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

		if ((settings->getEnabledDevice(DVC_BUTTON_COLLECTION)) && (externalCmd_collectionButton == 0))
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
			btnMasterProgress = (int)TBtnMasterProgress::CollectionMode;
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
			btnMasterProgress = (int)TBtnMasterProgress::Idle;
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
			btnMasterProgress = (int)TBtnMasterProgress::WaitSelectDevice;
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
					int retryCounter = 3;
					remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
					int timeout = 1000;
					while ((remoteCtrl[slaveId].cmdResult > 0xFFFF) && (retryCounter-- > 0))
					{
						printf ("              READ COMMAND COUNTER\n");
						remoteCtrl[slaveId].cmdRead = 1;
						remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
						remoteCtrl[slaveId].doCmd = 1;
						while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
					}

					printf ("              COMMAND RESULT: %08X (timeout: %d)\n", remoteCtrl[slaveId].cmdResult, timeout);
					if ((remoteCtrl[slaveId].cmdResult > 0xFFFFUL) || (timeout < 0)) break;
					unsigned int cmdCounter = ((unsigned int)remoteCtrl[slaveId].cmdResult);
					printf ("              COMMAND COUNTER VAL: %d\n", cmdCounter);
					db->Log(DB_EVENT_TYPE_MODBUS_SLAVE_CTRL, slaveId, cmdCounter, "[ButtonMasterThread]: Read counter [id] [counter]");
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
					printf ("              COMMAND RESULT: %08X (timeout: %d)\n", remoteCtrl[slaveId].cmdResult, timeout);
					if ((remoteCtrl[slaveId].doCmd != 0) || (remoteCtrl[slaveId].cmdResult > 0xFFFF) || (timeout < 0))
					{
						printf ("              WRITE COMMAND ERROR\n");
						db->Log(DB_EVENT_TYPE_MODBUS_SLAVE_CTRL, slaveId, timeout, "[ButtonMasterThread]: Write counter ERROR [id] [timeout]");
					}
					else
					{
						btnMasterProgress = (int)TBtnMasterProgress::SendingCommand;
						printf ("              WRITE COMMAND OK\n");
						db->Log(DB_EVENT_TYPE_MODBUS_SLAVE_CTRL, slaveId, sendedBal, "[ButtonMasterThread]: Write counter with balance [id] [balance]");
						///
						/// Waiting while salve device processed recivied data.
						/// While be slave device processed recived data it is not respond
						/// Delay size: (ImpulseCount + 3) * SignalWidth
						//
//						printf ("              WAITING WHILE PROCESSED DATA [%d ms]\n", (((sendedBal / 10) + (sendedBal % 10) + 3) * 200));
//						delay_ms(
//									(( (sendedBal / 10) + (sendedBal % 10) + 5) * 200)
//									);
						int m1 = (int)(sendedBal / 10);
						int m2 = (int)(sendedBal % 10);
						while (m1-- > 0)
						{
							status.intDeviceInfo.money_currentBalance -= 10;
							status.extDeviceInfo.remote_currentBalance -= 10;
							delay_ms(100);
						}

						while (m2-- > 0)
						{
							status.intDeviceInfo.money_currentBalance -= 1;
							status.extDeviceInfo.remote_currentBalance -= 1;
							delay_ms(100);
						}
					}
					//
					// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
/*					retryCounter = 5;
					remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
					while ((remoteCtrl[slaveId].cmdResult > 0xFFFF) && (retryCounter-- > 0))
					{
						printf ("              READ COMMAND COUNTER LOOP [RETRY: %d]\n", 4 - retryCounter);
						remoteCtrl[slaveId].cmdRead = 1;
						remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
						remoteCtrl[slaveId].doCmd = 1;
						timeout = 2000;
						while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
						db->Log(DB_EVENT_TYPE_MODBUS_SLAVE_CTRL, slaveId, ((unsigned int)remoteCtrl[slaveId].cmdResult), "[ButtonMasterThread]: Read counter [id] [counter]");
					}

					if ((cmdCounter == ((unsigned int)remoteCtrl[slaveId].cmdResult)) && (cmdCounter > 0)) break;
					cmdCounter = ((unsigned int)remoteCtrl[slaveId].cmdResult);
					printf ("              COMMAND COUNTER VAL: %d\n", cmdCounter);

					int errorRate = ((unsigned int)remoteCtrl[slaveId].cmdResult) - cmdCounter;
					if (errorRate > 1)
					{
							printf ("!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!! WRITE COMMAND DOUBLE\n");
					}
*/
					// Add information for print KKM documents
					// queueKkm->QueuePut( CashSumm, DON'T USED, DON'T USED, ServiceName);
					//
					btnMasterProgress = (int)TBtnMasterProgress::PrintCheck;
					char serviceNote[256];
					sprintf(serviceNote, "%s (Пост N %d)", settings->kkmParam.ServiceName, slaveId+1);
					queueKkm->QueuePut(sendedBal, 0, 0, serviceNote);
					// !!!!!!!!!!!!!!!!!!!!
					///

					delay_ms(1000);
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
