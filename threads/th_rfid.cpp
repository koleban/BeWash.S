#include "../main.h"
/**/
/*******************************************************************************************/
/**/
//#pragma region Карты "RFID Карты"
PI_THREAD(RFIDWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.RFIDWatch) return (void*)0;
#ifndef _RFID_DEVICE_CRT288K_
	RFIDDevice* rfidDevice = RFIDDevice::getInstance();
#endif
#ifdef _RFID_DEVICE_CRT288K_
	Crt288KDevice* rfidDevice = Crt288KDevice::getInstance();
#endif

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] RFID: Card reader (RFID) thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

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
				while ((timeout_id++ < 1000) && (settings->busyFlag.RFIDWatch)) {settings->workFlag.RFIDWatch = 0; delay_ms(1); continue;}
				if (rfidDevice->errorCount > 10)
					{ if (settings->debugFlag.RFIDWatch) { printf("[DEBUG] RFID: Redetect ....\n");} break; }
				settings->busyFlag.RFIDWatch++;
				int tryCount = 3;
				while (tryCount-- > 0)
				{
					DWORD resultCardID = rfidDevice->cmdPoll();
					if (resultCardID == 0xFFFFFFFF) {status.extDeviceInfo.rfid_errorNum++; settings->workFlag.RFIDWatch = 0; delay_ms(250);}
					if ((resultCardID < 0xFFFFFFFF) && (resultCardID > 0xFF)) break;
				}
				if (tryCount == 0) { printf("RFID Error: %d\n", status.extDeviceInfo.rfid_errorNum); settings->busyFlag.RFIDWatch--; delay_ms(250); continue;}
				///
				/// Проверим чтоб присутствие карты "установилось постоянным"
				if (prevCardPresent != rfidDevice->cardPresent)
				{
					prevCardPresent = rfidDevice->cardPresent;
					settings->busyFlag.RFIDWatch--;
				}
				///
				/// Проверим чтоб номер карты "установился постоянный"
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
						/// Установился постоянный номер карты
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
				// TODO!!! Delay ???
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
