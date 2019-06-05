#include "../main.h"

Database* gDbCard = new Database();

//#pragma region Подситема "RFID ОБМЕН ДАННЫМИ"
PI_THREAD(RFIDExchangeThread)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.IntCommonThread) return (void*)0;
	int lastCardPresent = 0;
	BYTE lastCardNumber[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	Database* db = new Database();
	db->Init(settings);
	gDbCard->Init(&settings->gdatabaseSettings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] RFID: RFID exchange thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	int rt = 0;
	while (settings->threadFlag.IntCommonThread)
	{
			int timeout = 200;
			while ((settings->busyFlag.RFIDWatch) && (timeout--)) {delay_ms(1);}
			//if ((lastCardPresent == 0) && (status.extDeviceInfo.rfid_cardPresent == 0)) {delay_ms(200); continue;}
			settings->busyFlag.RFIDWatch++;
			//
			// Изменилась состояние карты RFID
			if (lastCardPresent != status.extDeviceInfo.rfid_cardPresent)
			{
				if (status.extDeviceInfo.rfid_cardPresent)
				{
					lastCardPresent = status.extDeviceInfo.rfid_cardPresent;
					// TODO: Если новая карты вставлена
					if (!emptyCardNumber(status.extDeviceInfo.rfid_incomeCardNumber))
					{
						memcpy(&status.extDeviceInfo.rfid_prevCardNumber[0], &lastCardNumber[0], 6);
						memcpy(&lastCardNumber[0], &status.extDeviceInfo.rfid_incomeCardNumber[0], 6);
						printf("[RFID] New card inserted: %02X%02X%02X%02X%02X%02X\n",
							status.extDeviceInfo.rfid_incomeCardNumber[0],
							status.extDeviceInfo.rfid_incomeCardNumber[1],
							status.extDeviceInfo.rfid_incomeCardNumber[2],
							status.extDeviceInfo.rfid_incomeCardNumber[3],
							status.extDeviceInfo.rfid_incomeCardNumber[4],
							status.extDeviceInfo.rfid_incomeCardNumber[5]
							);
						// TODO: Запросить данные карты, владельца, баланс
						DB_RFIDCardInfo cardInfo;
						memset(&cardInfo, 0, sizeof(cardInfo));
						getCardInfo(status.extDeviceInfo.rfid_incomeCardNumber, &cardInfo);
						status.intDeviceInfo.money_currentBalance += cardInfo.cardMoney;
						///
						/// Loging in database ...
						/// New card inserted
						///
						char noteBuffer[1024];
						sprintf(noteBuffer, "New card [BAL: %2d, CRD: %08X]", status.intDeviceInfo.money_currentBalance, getCardIDFromBytes(status.extDeviceInfo.rfid_incomeCardNumber));
						db->Log(DB_EVENT_TYPE_INT_CARD_INSERTED, status.intDeviceInfo.program_currentProgram, status.intDeviceInfo.money_currentBalance, noteBuffer);
						///
						///
					}
				}
				else
				{
					// TODO: КАРТА УБРАНА
					DB_RFIDCardInfo cardInfo;
					getCardInfo(lastCardNumber, &cardInfo);
					if ((status.intDeviceInfo.money_currentBalance - cardInfo.cardMoney) != 0)
						setCardMoney(lastCardNumber, status.intDeviceInfo.money_currentBalance - cardInfo.cardMoney);

					///
					/// Loging in database ...
					/// Card is gone
					///
					char noteBuffer[1024];
					sprintf(noteBuffer, "Card is gone [BAL: %2d, CRD: %08X]", status.intDeviceInfo.money_currentBalance, getCardIDFromBytes(lastCardNumber));
					db->Log(DB_EVENT_TYPE_INT_CARD_GONE, status.intDeviceInfo.program_currentProgram, status.intDeviceInfo.money_currentBalance, noteBuffer);
					///
					///

					status.intDeviceInfo.money_currentBalance = 0;
					memcpy(&status.extDeviceInfo.rfid_prevCardNumber[0], &lastCardNumber[0], 6);
					printf("[RFID] Card is gone [%08X]: %02X%02X%02X%02X%02X%02X\n", getCardIDFromBytes(lastCardNumber),
						status.extDeviceInfo.rfid_incomeCardNumber[0],
						status.extDeviceInfo.rfid_incomeCardNumber[1],
						status.extDeviceInfo.rfid_incomeCardNumber[2],
						status.extDeviceInfo.rfid_incomeCardNumber[3],
						status.extDeviceInfo.rfid_incomeCardNumber[4],
						status.extDeviceInfo.rfid_incomeCardNumber[4]
						);
					memset(&lastCardNumber[0], 0, 6);
					// TODO: Сохранить данные карты, баланс
				}
				lastCardPresent = status.extDeviceInfo.rfid_cardPresent;
			}
			else
			{
					// ФЛАГ ВСТАВКИ КАРТЫ НЕ ИЗМЕНЕН
					// А НОМЕР ПОМЕНЯЛСЯ
					if ((compareCardNumber(lastCardNumber, status.extDeviceInfo.rfid_incomeCardNumber)) && (!emptyCardNumber(lastCardNumber)) && (!emptyCardNumber(status.extDeviceInfo.rfid_incomeCardNumber)))
					{
						// TODO:Карта заменена. Считать баланс. Обновить на карте. Обнулить. Прочитать новую карту
						DB_RFIDCardInfo cardInfo;
						getCardInfo(lastCardNumber, &cardInfo);
						if ((status.intDeviceInfo.money_currentBalance - cardInfo.cardMoney) != 0)
							setCardMoney(lastCardNumber, status.intDeviceInfo.money_currentBalance - cardInfo.cardMoney);

						char noteBuffer[1024];
						sprintf(noteBuffer, "Card is gone (change) [BAL: %2d, CRD: %08X]", status.intDeviceInfo.money_currentBalance, getCardIDFromBytes(lastCardNumber));
						db->Log(DB_EVENT_TYPE_INT_CARD_GONE, status.intDeviceInfo.program_currentProgram, status.intDeviceInfo.money_currentBalance, noteBuffer);

						status.intDeviceInfo.money_currentBalance = 0;

						getCardInfo(status.extDeviceInfo.rfid_incomeCardNumber, &cardInfo);
						status.intDeviceInfo.money_currentBalance = cardInfo.cardMoney;

						sprintf(noteBuffer, "New card (change) [BAL: %2d, CRD: %08X]", status.intDeviceInfo.money_currentBalance, getCardIDFromBytes(status.extDeviceInfo.rfid_incomeCardNumber));
						db->Log(DB_EVENT_TYPE_INT_CARD_INSERTED, status.intDeviceInfo.program_currentProgram, status.intDeviceInfo.money_currentBalance, noteBuffer);

						memcpy(status.extDeviceInfo.rfid_prevCardNumber, lastCardNumber, 6);
						memcpy(lastCardNumber, status.extDeviceInfo.rfid_incomeCardNumber, 6);
						lastCardPresent = status.extDeviceInfo.rfid_cardPresent;
					}
			}
			settings->busyFlag.RFIDWatch = 0;
		delay_ms(100);
	}
	db->Close();
	printf("[RFID_EXCHANGE]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
