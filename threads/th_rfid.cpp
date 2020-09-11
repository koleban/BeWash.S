#include "../main.h"
/**/
/*******************************************************************************************/
/**/
//#pragma region  арты "RFID  арты"
PI_THREAD(RFIDWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.RFIDWatch) return (void*)0;
#ifdef _RFID_DEVICE_SHS_
	RFIDDevice* rfidDevice = RFIDDevice::getInstance();
#endif
#ifdef _RFID_DEVICE_CRT288K_
	Crt288KDevice* rfidDevice = Crt288KDevice::getInstance();
#endif
#ifdef _RFID_DEVICE_CP_Z_
	RFID_CPZDevice* rfidDevice = RFID_CPZDevice::getInstance();
#endif

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] RFID: Card reader (RFID) thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	rfidDevice->Init(settings);
	rfidDevice->Lock(1);
	delay(2);
	DWORD prevCard = 0;
	int prevCardPresent = 0;
	int connErrorCouter = 0;
	while (settings->threadFlag.RFIDWatch)
	{
		rfidDevice->OpenDevice();
		if (rfidDevice->IsOpened())
		{
			rfidDevice->Lock(0);
			while (settings->threadFlag.RFIDWatch)
			{
				connErrorCouter = 0;
				settings->workFlag.RFIDWatch = 0;
				int timeout_id = 0;
				//while ((timeout_id++ < 1000) && (settings->busyFlag.RFIDWatch)) {settings->workFlag.RFIDWatch = 0; delay_ms(1); continue;}
				while ((timeout_id++ < 100) && (settings->busyFlag.RFIDWatch)) {settings->workFlag.RFIDWatch = 0; delay_ms(1); continue;}
				if (timeout_id >= 100) settings->workFlag.RFIDWatch = 0;
				if (rfidDevice->errorCount > 10)
					{ if (settings->debugFlag.RFIDWatch) { printf("[DEBUG] RFID: Redetect ....\n");} break; }
				settings->busyFlag.RFIDWatch++;
				int tryCount = 3;
				#ifdef _RFID_DEVICE_CRT288K_
				tryCount = 1;
				#endif
				while (tryCount-- > 0)
				{
					DWORD resultCardID = rfidDevice->cmdPoll();
					if (resultCardID == 0xFFFFFFFF) {status.extDeviceInfo.rfid_errorNum++; settings->workFlag.RFIDWatch = 0; delay_ms(250);}
					if ((resultCardID < 0xFFFFFFFF) && (resultCardID > 0xFF)) break;
				}
				
				///
				/// ≈сли используетс€ прикладной считыватель или двухфакторна€ работа с картами
				if (settings->useRFID2Mobile == 1)
				{
					BYTE zeroCrdNum[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
					if ((status.intDeviceInfo.money_currentBalance <= 1) && (status.intDeviceInfo.program_currentProgram != 0))
					{
						status.intDeviceInfo.money_currentBalance = 0;
						// активна€ карта вставленна
						memcpy(status.extDeviceInfo.rfid_incomeCardNumber, zeroCrdNum, 6);
						status.extDeviceInfo.rfid_cardPresent = 0;
					}
					if ((rfidDevice->cardPresent) && (!emptyCardNumber(rfidDevice->cardNumber)))
					{
						if (memcmp(status.extDeviceInfo.rfid_incomeCardNumber, zeroCrdNum, 6) == 0)
						{
							// текуща€ карта 0 нова€ карта вставленна
							memcpy(status.extDeviceInfo.rfid_incomeCardNumber, rfidDevice->cardNumber, 6);
							status.extDeviceInfo.rfid_cardPresent = 1;
							prevCard = 1;
							if (settings->useRFID2OnlyDiscount == 1)
							{
								double dsCurrent = (double)maxval(settings->discountCardDeposit,settings->cardBonus);
								
								if ((status.extDeviceInfo.rfid_cardPresent) && (dsCurrent > 0))
								{
									// ƒоначислим скидку дл€ карты
									status.intDeviceInfo.money_currentBalance += (int)(status.intDeviceInfo.money_currentBalance * (dsCurrent / 100));
									printf("[DEBUG] RFIDThread: Discount for card deposite: add %d rur\n", (int)(status.intDeviceInfo.money_currentBalance * (dsCurrent / 100)));
								}
							}
						}
						if (settings->useRFID2OnlyDiscount == 0)
						{
							if ((memcmp(status.extDeviceInfo.rfid_incomeCardNumber, rfidDevice->cardNumber, 6) == 0) && (status.extDeviceInfo.rfid_cardPresent == 1) && (prevCard == 0))
							{
								// активна€ карта вставленна
								memcpy(status.extDeviceInfo.rfid_incomeCardNumber, zeroCrdNum, 6);
								status.extDeviceInfo.rfid_cardPresent = 0;
							}
							if ((memcmp(status.extDeviceInfo.rfid_incomeCardNumber, rfidDevice->cardNumber, 6) != 0) && (status.extDeviceInfo.rfid_cardPresent == 1) && (prevCard == 0))
							{
								// активна€ карта изменена
								memcpy(status.extDeviceInfo.rfid_incomeCardNumber, rfidDevice->cardNumber, 6);
								prevCard = 1;
							}
						}
					}
					else
					{
						if (status.extDeviceInfo.rfid_cardPresent == 0)
						{
							memcpy(status.extDeviceInfo.rfid_incomeCardNumber, zeroCrdNum, 6);
							int tmp22 = 0;
// TEST //					while ((rfidDevice->cardPresent) && (tmp22++ < 10)) { rfidDevice->cmdPoll(); delay_ms(250);}
							while ((rfidDevice->cardPresent) && (tmp22++ < 3)) { rfidDevice->cmdPoll(); delay_ms(150);}
						}
						prevCard = 0;
					}
				}
				///
				/// —тандартна€ работа с картами
				else
				{
					///
					/// ѕроверим чтоб присутствие карты "установилось посто€нным"
					if (prevCardPresent != rfidDevice->cardPresent)
					{
						prevCardPresent = rfidDevice->cardPresent;
						settings->busyFlag.RFIDWatch--;
					}
					///
					/// ѕроверим чтоб номер карты "установилс€ посто€нный"
					else
					{
						if (prevCard != rfidDevice->digCardNumber)
						{
							if (settings->debugFlag.RFIDWatch)
								printf("[RFID Thread]: Detect new card: curr: %08X prev: (%08X)\n", rfidDevice->digCardNumber, prevCard);
							prevCard = rfidDevice->digCardNumber;
							settings->busyFlag.RFIDWatch--;
						}
						else
						{
							///
							/// ”становилс€ посто€нный номер карты
							memcpy(status.extDeviceInfo.rfid_incomeCardNumber, rfidDevice->cardNumber, 6);
							status.extDeviceInfo.rfid_cardPresent = rfidDevice->cardPresent;
							#ifndef _RFID_DEVICE_CRT288K_
							if (status.extDeviceInfo.rfid_cardPresent)
								rfidDevice->Lock(0);
							else
								rfidDevice->OKBlink();
							settings->busyFlag.RFIDWatch--;
							delay_ms(250);
							#endif
	
							#ifdef _RFID_DEVICE_CRT288K_
							delay_ms(100);
							#endif
						}
					}
				}
			}	
		}
		else
			connErrorCouter++;
		rfidDevice->LockError();
		delay(1);
		rfidDevice->CloseDevice();
		rfidDevice->Detect();
		if (connErrorCouter > 5)
			{ delay(60); connErrorCouter = 0; }
	}
	rfidDevice->LockError();
	printf("[RFID]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
