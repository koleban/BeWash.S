#include "../main.h"

using namespace DriverFR;

int errorCode = 0;
static DrvFR* drv;

char tmpUTFStr[1024] = {0};
char tmpCPStr[1024] = {0};
void cp866_cp1251( unsigned char* s , int length);
//---------------------------------------------------------------------
int OpenPaymentDocument(DrvFR* drvFr)
{
	drvFr->CutType = settings->kkmParam.CutType;
	drvFr->CutCheck();
	return 0;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
int AddWareString(DrvFR* drvFr, TCheckType checkType, double quantity, double price, char* stringForPrinting, int taxType, int paymentItemSign, int paymentTypeSign)
{
	drvFr->Department = 1;
	drvFr->Price = price;
	drvFr->Quantity = quantity;
	drvFr->Summ1 = drvFr->Price * drvFr->Quantity;
	drvFr->Summ1Enabled = 0x00;
	drvFr->CheckType = (int)checkType;
	drvFr->Tax1 = taxType;
	drvFr->TaxType = taxType;
	drvFr->TaxValue = 0;
	drvFr->TaxValueEnabled = 0x00;
	drvFr->DiscountOnCheck = 0x00;
	drvFr->PaymentItemSign = paymentItemSign;
	drvFr->PaymentTypeSign = paymentTypeSign;
	strncpy(drvFr->StringForPrinting, stringForPrinting, 40);

	int res = drvFr->FNOperation();
	if ((drvFr->ResultCode != 0) && (drvFr->ResultCode != 69) && (drvFr->ResultCode != 70))
		printf("KKM: Error [1] %d %s\n", drvFr->ResultCode, drvFr->ResultCodeDescription);
	fflush(stdout);
	return res;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
int ClosePaymentDocument(DrvFR* drvFr, double Summ1 = 0, double Summ2 = 0, double Summ3 = 0, double Summ4 = 0, double Summ5 = 0, double Summ6 = 0, double Summ7 = 0, double Summ8 = 0, double Summ9 = 0, double Summ10 = 0, double Summ11 = 0, double Summ12 = 0, double Summ13 = 0, double Summ14 = 0, double Summ15 = 0, double Summ16 = 0)
{
	Settings* settings = Settings::getInstance();

	drvFr->Summ1 = Summ1;
	drvFr->Summ2 = Summ2;
	drvFr->Summ3 = Summ3;
	drvFr->Summ4 = Summ4;
	drvFr->Summ5 = Summ5;
	drvFr->Summ6 = Summ6;
	drvFr->Summ7 = Summ7;
	drvFr->Summ8 = Summ8;
	drvFr->Summ9 = Summ9;
	drvFr->Summ10 = Summ10;
	drvFr->Summ11 = Summ11;
	drvFr->Summ12 = Summ12;
	drvFr->Summ13 = Summ13;
	drvFr->Summ14 = Summ14;
	drvFr->Summ15 = Summ15;
	drvFr->Summ16 = Summ16;

	drvFr->TaxValue1 = 0;
	drvFr->TaxValue2 = 0;
	drvFr->TaxValue3 = 0;
	drvFr->TaxValue4 = 0;
	drvFr->TaxValue5 = 0;
	drvFr->TaxValue6 = 0;

	drvFr->RoundingSumm = 0;
	drvFr->TaxType = settings->kkmParam.TaxType;

    memset(drvFr->StringForPrinting, 0, sizeof(drvFr->StringForPrinting));

	int res = drvFr->FNCloseCheckEx();
	if ((drvFr->ResultCode != 0) && (drvFr->ResultCode != 69) && (drvFr->ResultCode != 70))
		printf("KKM: Error [2] %d %s\n", drvFr->ResultCode, drvFr->ResultCodeDescription);
	fflush(stdout);
	return res;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------

void CheckDevice(DrvFR* drv)
{
	drv->GetECRStatus();
	int dataReceiveCount = 0;
	while (!(((drv->ECRMode == (int)TECRMode::OpenWorkspace) || (drv->ECRMode == (int)TECRMode::WorkMode))))
	{
		if (drv->ECRMode == (int)TECRMode::DataInProgress)
		{
			if (errorCode != 0x10)
			{
				cp2utf("Устройство выдает данные. Ожидаем ...", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x10;
			if (dataReceiveCount++ > 5)
			{
				cp2utf("Прерываем передачу данных от устройства!", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
				drv->Password = 0;
				drv->InterruptDataStream();
				drv->PrintErrorDescription();
				dataReceiveCount = 0;
				drv->Password = 30;
			}
		}
		else if (drv->ECRMode == (int)TECRMode::OpenWorkspace24h)
		{
			if (errorCode != 0x11)
			{
				cp2utf("Открытая смена, 24 часа кончились. Закроем смену.", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x11;
			drv->FNCloseSession();
			delay_ms(10000);
			drv->CutType = settings->kkmParam.CutType;
			drv->CutCheck();
			delay_ms(500);
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::Blocked_WrongPasswordTaxInspector)
		{
			if (errorCode != 0x12)
			{
				cp2utf("Пароль налогового инспектора не корректен. Установим пароль администратора.", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x12;
			drv->Password = 30;
		}
		else if ((drv->ECRMode == (int)TECRMode::WorkWithFNDocument) || (drv->ECRMode == (int)TECRMode::WorkFNDocumentDone))
		{
			if (errorCode != 0x13)
			{
				cp2utf("Работа с подкладным документом ....", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x13;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::PrintingEKLZReport)
		{
			if (errorCode != 0x14)
			{
				cp2utf("Печать отчета ЕКЛЗ ....", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x14;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::PrintingFNReport)
		{
			if (errorCode != 0x15)
			{
				cp2utf("Печать фискального отчета ....", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x15;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::DigitPointChangeRight)
		{
			if (errorCode != 0x16)
			{
				cp2utf("Разрешение изменения положения десятичной точки.", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x16;
			drv->PointPosition = true;
			drv->SetPointPosition();
		}
		else if (drv->ECRMode == (int)TECRMode::WaitingDateConfirm)
		{
			if (errorCode != 0x17)
			{
				cp2utf("Ожидание подтверждения ввода даты.", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x17;
			int day = 24;
			int month = 10;
			int year = 2018;
			drv->Date.tm_mday = day;
			drv->Date.tm_mon = month - 1;
			drv->Date.tm_year = year - 1900;
			mktime(&drv->Date);
			time_t rawtime;
			time(&rawtime);
			struct tm * loctime = localtime(&rawtime);
			memcpy(&drv->Date, loctime, sizeof(drv->Date));
			drv->ConfirmDate();
		}
		else if (drv->ECRMode == (int)TECRMode::CloseWorkspace)
		{
			if (errorCode != 0x18)
			{
				cp2utf("Смена закрыта. Откроем смену.", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x18;
			drv->FNOpenSession();
			delay_ms(3000);
			drv->CutType = settings->kkmParam.CutType;
			drv->CutCheck();
			delay_ms(500);
		}
		else if (drv->ECRMode == (int)TECRMode::PrintFNDocument)
		{
			if (errorCode != 0x19)
			{
				cp2utf("Печать чека ....", tmpUTFStr);
				printf("%s\n", tmpUTFStr);
			}
			errorCode = 0x19;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::OpenedDocument)
		{
			break;
		}
		else
		{
			sprintf(tmpCPStr, "Не корректный режим устройства. Ожидается \"Принтер готов\" или \"Смена открыта\" Debug: (%d) %s\n", drv->ECRMode, drv->ECRModeDescription);
			cp2utf(tmpCPStr, tmpUTFStr);
			printf("%s\n", tmpUTFStr);
		}
		usleep(1000000);
		drv->GetECRStatus();
	}

	while (drv->ECRAdvancedMode != (int)TECRAdvancedMode::DeviceOK)
	{
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::NoPapper)
		{
			if (errorCode != 0x1A)
			{
				sprintf(tmpCPStr, "Отсутствует бумага. Ожидаем ...\n");
				cp2utf(tmpCPStr, tmpUTFStr);
				printf("%s", tmpUTFStr);
			}
			errorCode = 0x1A;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::PrintingFNDocument)
		{
			if (errorCode != 0x1B)
			{
				sprintf(tmpCPStr, "Печатается фискальный документ. Ожидаем завершения ...\n");
				cp2utf(tmpCPStr, tmpUTFStr);
				printf("%s", tmpUTFStr);
			}
			errorCode = 0x1B;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::PrintingFNDocument)
		{
			if (errorCode != 0x1C)
			{
				sprintf(tmpCPStr, "Печатается документ. Ожидаем завершения ...\n");
				cp2utf(tmpCPStr, tmpUTFStr);
				printf("%s", tmpUTFStr);
			}
			errorCode = 0x1C;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::NoPapperError)
		{
			if (errorCode != 0x1D)
			{
				sprintf(tmpCPStr, "Внимание! Закончилась бумага!\n");
				cp2utf(tmpCPStr, tmpUTFStr);
				printf("%s", tmpUTFStr);
			}
			errorCode = 0x1D;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::WaitPrintContinue)
		{
			if (errorCode != 0x1E)
			{
				sprintf(tmpCPStr, "Продолжаем печать ...\n");
				cp2utf(tmpCPStr, tmpUTFStr);
				printf("%s", tmpUTFStr);
			}
			errorCode = 0x1E;
			drv->ContinuePrinting();
		}
		usleep(1000000);
		drv->GetECRStatus();
	}
}
//---------------------------------------------------------------------
void showTLVStruct(DrvFR* drv)
{
	if (!settings->debugFlag.KKMWatch) return;

	printf("FNRequestFiscalDocumentTLV: Show TLV structure for fiscal document\n");
	struct tm* now;
	time_t rawtime;
	tlv_box_t* parsedBoxes = tlv_box_parse(drv->TLVData, drv->DataLength);
	char value[128];
	int length = 128;
	int l_index = 0;
	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_FN, value, &length);
	printf("ФН: %s\n", value);
	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_RNKKT, value, &length);
	printf(" РН ККТ: %s\n", value);
	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_INN, value, &length);
	printf(" ИНН: %s\n", value);

	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_FD, value, &length);
	printf(" ФД: %d\n", *((DWORD*)&value[0]));

	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_DATE_TIME, value, &length);
	if (length == 4)
	{
		rawtime = (time_t)*((DWORD*)&value[0]);
		if (settings->kkmParam.LocalTime)
			now = localtime(&rawtime);
		else
			now = gmtime(&rawtime);
		printf(" %02d.%02d.%04d %02d:%02d:%02d\n", now->tm_mday, now->tm_mon, now->tm_year, now->tm_hour, now->tm_min, now->tm_sec);
	}
	else
	{
		l_index = 0;
		while (l_index < length)
	    	printf(" %02X", value[l_index++]);
		printf("\n");
	}

	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_FP, value, &length);
	printf(" ФП:");
	printf(" %lu", (value[2] << 24)+(value[3] << 16)+(value[4] << 8)+(value[5]));
	printf("\n");

	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_SESSION, value, &length);
	printf(" СМЕНА: %d", *((DWORD*)&value[0]));
	printf("\n");
	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_DOCNUM, value, &length);
	printf(" НОМ. ЧЕКА: %d", *((DWORD*)&value[0]));
	printf("\n");
	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_SUMM, value, &length);
	printf(" СУММА: %g", ((float)*((DWORD*)&value[0]))/100);
	printf("\n");
	length = 128;
//	tlv_box_get_string(parsedBoxes, TLV_DATA_DEVNUMBER, value, &length);
//	printf("TLV_DATA_DEVNUMBER:			 %s \n", value);
	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_SUMM_CASH, value, &length);
	printf(" НАЛ.: %g", ((float)*((DWORD*)&value[0]))/100);
	printf("\n");
	length = 128;
	tlv_box_get_string(parsedBoxes, TLV_DATA_SUMM_CARD, value, &length);
	printf(" VISA: %g", ((float)*((DWORD*)&value[0]))/100);
	printf("\n");
	length = 128;
//	tlv_box_get_string(parsedBoxes, TLV_DATA_FNS_LINK, value, &length);
//	printf("TLV_DATA_FNS_LINK:			 %s \n", value);
//	printf("\n");

	length = 128;
//	tlv_box_get_string(parsedBoxes, TLV_DATA_ADDR_DOC, value, &length);
//	cp866_cp1251((unsigned char*)value, length);
//	printf("TLV_DATA_ADDR_DOC:			 %s \n", value);
//	printf("\n");

	length = 128;
//	tlv_box_get_string(parsedBoxes, TLV_DATA_USERNAME, value, &length);
//	cp866_cp1251((unsigned char*)value, length);
//	printf("TLV_DATA_USERNAME:			 %s \n", value);
//	printf("\n");

	length = 128;
//	tlv_box_get_string(parsedBoxes, TLV_DATA_USERADDR, value, &length);
//	cp866_cp1251((unsigned char*)value, length);
//	printf("TLV_DATA_USERADDR:			 %s \n", value);
	printf("\n");
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------

PI_THREAD(KKMWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.KKMWatch) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[2000];
	sprintf(myNote, "[THREAD] KKM: Online KMM thread init");
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	printf("Thread KKM is now working TaxType: %d\n", settings->kkmParam.TaxType);
	printf("    TaxType  1 : NDS\n");
	printf("    TaxType  2 : 6%\n");
	printf("    TaxType  4 : 15%\n");
	printf("    TaxType  8 : ENVD\n");
	printf("    TaxType 16 : ESHN\n");
	printf("    TaxType 32 : PATENT\n");

	QueueType valueKkm;

	struct tm* now;
	time_t rawtime;
	bool error = true;
	drv = new DrvFR(settings->kkmParam.kkmPass, 0, 0, 1000, 0, TConnectionType::ctSocket, settings->kkmParam.kkmPort, settings->kkmParam.kkmAddr, true);
	while (settings->threadFlag.KKMWatch)
	{
		sprintf(myNote, "[THREAD] KKM: Creating KKM driver instance and trying to connect to the device");
		db->Log(DB_EVENT_TYPE_KKM_THREAD, 0, 0, myNote);
		if (settings->debugFlag.KKMWatch)
			printf("%s\n", myNote);
		if (drv->Connect() > 0)
		{
			sprintf(myNote, "[THREAD] KKM: Connecting to KKM device successfully");
			db->Log(DB_EVENT_TYPE_KKM_THREAD, 0, 0, myNote);
			if (settings->debugFlag.KKMWatch)
				printf("%s\n", myNote);
			settings->workFlag.KKMWatch = 0;
			if (settings->kkmParam.SharedMode == 0)
				drv->Beep();
			delay_ms(200);
			drv->FNGetInfoExchangeStatus();
			if (drv->MessageCount > 0)
			{
				term_setattr(32);
				sprintf(tmpCPStr, "[THREAD] ККМ: ВНИМАНИЕ : ЕСТЬ НЕ ОТПРАВЛЕНИЕ ЧЕКИ В ОФД. ПРОВЕРЬТЕ БАЛАНС, ПОДПИСКУ, ПАРАМЕТРЫ ОФД И СВЯЗЬ С ИНТЕРНЕТОМ!!!\n");
				cp2utf(tmpCPStr, tmpUTFStr);
				printf("%s", tmpUTFStr);

				term_setattr(37);
			}

			if (settings->kkmParam.SharedMode)
			{
				drv->Disconnect();
				sprintf(tmpCPStr, "[THREAD] ККМ: ИСПОЛЬЗУЕТСЯ SHARED РЕЖИМ !!!\n");
				cp2utf(tmpCPStr, tmpUTFStr);
				printf("%s", tmpUTFStr);
			}

			while (settings->threadFlag.KKMWatch)
			{
				time(&rawtime);
				now = localtime(&rawtime);
				fflush(stdout);
				if (settings->kkmParam.SharedMode)
				{
					if (queueKkm->QueueCount <= 0)
					{
						drv->Disconnect();
						delay_ms(1000);
						continue;
					}
					else
					{
						if (drv->Connect() <= 0) drv->Disconnect();
						delay_ms(200);
					}
				}
				if (drv->CheckConnection() != 1)
				{
							if (!error)
							{
								sprintf(tmpCPStr, "[  CHK  ] Проверка связи с ККМ %s", asctime(now));
								cp2utf(tmpCPStr, tmpUTFStr);
								printf("%s", tmpUTFStr);
							}
							error = true;
							fflush(stdout);
							drv->Disconnect();
							break;
				}
				if (error)
				{
					sprintf(myNote, "[THREAD] KKM: Connection worked. Device: %s", drv->UDescription);
					db->Log(DB_EVENT_TYPE_KKM_THREAD, 0, 0, myNote);
					if (settings->debugFlag.KKMWatch)
						printf("%s\n", myNote);
				}
				error = false;

				CheckDevice(drv);
				int result = 0;
				if (!( (drv->ECRMode == (int)TECRMode::OpenWorkspace) || (drv->ECRMode == (int)TECRMode::WorkMode) ) )
				{
					result = 1;
					if (drv->ECRMode == (int)TECRMode::OpenedDocument)
					{
						sprintf(tmpCPStr, "[THREAD] KKM: Обнаружен открытый документ. Отменяем его\n");
						cp2utf(tmpCPStr, tmpUTFStr);
						printf("%s", tmpUTFStr);
						drv->CancelCheck();
						if (drv->ResultCode == 0)
							result = 0;
					}

					if (result != 0)
						printf ("[THREAD] KKM: mode is incorrect ECRMode: %d\n", drv->ECRMode);
				}
				else
				{
					result = 0;
				}

				drv->FNGetStatus();
				delay(4);
				if (result == 0)
				{
					while(queueKkm->QueueGet(&valueKkm) >= 0)
					{
						if (valueKkm.eventId >= settings->kkmParam.MaxAmount)
						{
							sprintf(myNote, "[THREAD] KKM: Error on amount size [MAX: %d, Curr: %d]", settings->kkmParam.MaxAmount, valueKkm.eventId);
							db->Log(DB_EVENT_TYPE_KKM_AMOUNT_ERROR, (double)valueKkm.eventId, valueKkm.data1, myNote);
							sprintf(tmpCPStr, "[THREAD] KKM: Обнаружена ОШИБКА. Сумма оплаты больше допустимой (%d руб) : %d руб\n", settings->kkmParam.MaxAmount, valueKkm.eventId);
							cp2utf(tmpCPStr, tmpUTFStr);
							printf("%s", tmpUTFStr);
							continue;
						}
						CheckDevice(drv);
						delay_ms(1000);
						result = OpenPaymentDocument(drv);
						sprintf(myNote, "[THREAD] KKM: Opening the payment document");
						db->Log(DB_EVENT_TYPE_KKM_FN_OPEN_DOC, 0, 0, myNote);
						if (settings->debugFlag.KKMWatch)
							printf("%s\n", myNote);
						result = AddWareString(drv, TCheckType::Sale, 1, valueKkm.eventId+valueKkm.data1, valueKkm.note, TTaxType::NoNds, TPaymentItemSign::Service, TPaymentTypeSign::Prepayment100);
						if (result != 0)
						{
							sprintf(myNote, "[THREAD] KKM: Cancel payment document");
							db->Log(DB_EVENT_TYPE_KKM_FN_CANCEL_DOC, 0, 0, myNote);
							term_setattr(31);
							sprintf(tmpCPStr, "Печать чека ОТМЕНА\n");
							cp2utf(tmpCPStr, tmpUTFStr);
							printf("%s", tmpUTFStr);
							term_setattr(37);
							break;
						}
						sprintf(tmpCPStr, "[THREAD] KKM: Продажа : %s - 1 шт\n", valueKkm.note);
						cp2utf(tmpCPStr, tmpUTFStr);
						printf("%s", tmpUTFStr);
						fflush(stdout);
//
//						!!! LOG for payment !!!
//
						delay_ms(50);
						drv->CheckSubTotal();
						delay_ms(50);
						term_setattr(32);
						sprintf(tmpCPStr, "[THREAD] KKM: Закрываем чек : Сумма: Нал. %d  Картой: %d\n", valueKkm.eventId, valueKkm.data1);
						cp2utf(tmpCPStr, tmpUTFStr);
						printf("%s", tmpUTFStr);
						term_setattr(37);
						sprintf(myNote, "[THREAD] KKM: Close the payment document [$: %d, VISA: %d]", valueKkm.eventId, (int)valueKkm.data1);
						db->Log(DB_EVENT_TYPE_KKM_FN_CLOSE_DOC, (double)valueKkm.eventId, valueKkm.data1, myNote);

						ClosePaymentDocument(drv, valueKkm.eventId, 0, 0, valueKkm.data1);
						if ((drv->ResultCode != 0) && (drv->ResultCode != 69) && (drv->ResultCode != 70))
						{
							sprintf(myNote, "[THREAD] KKM: Close the payment document ERROR %d %s", drv->ResultCode, drv->ResultCodeDescription);
							db->Log(DB_EVENT_TYPE_KKM_ERROR, (double)valueKkm.eventId, valueKkm.data1, myNote);
							printf("KKM: Close check ERROR %d %s\n", drv->ResultCode, drv->ResultCodeDescription);
						}
						fflush(stdout);
						delay_ms(1000);
						CheckDevice(drv);
						delay_ms(500);
						drv->FNGetStatus();
						CheckDevice(drv);
						drv->Password = 30;
						if (drv->FNReadFiscalDocumentTLV() == 1)
							showTLVStruct(drv);
						delay_ms(settings->kkmParam.QueryTime);
					}
				}
				if (settings->kkmParam.SharedMode)
					drv->Disconnect();
				settings->workFlag.KKMWatch = 0;
				delay_ms(settings->kkmParam.QueryTime);
			}
		}
		else
		{
			drv->Disconnect();
			printf("KKM: Connection ERROR\n");
		}
		delay_ms(settings->kkmParam.QueryTime);
	}
	if (db->Close())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	printf("[KKM]: Thread ended.\n");
	return (void*)0;
}

//***************************************************************

void cp866_cp1251( unsigned char* s , int length)
{
    for( int index = 0; index < length; index++ )
    {
        if( s[index] >= 0x80 && s[index] <= 0xAF )
            s[index] += 64;
        else if( s[index] >= 0xE0 && s[index] <= 0xEF )
            s[index] += 16;
    }
}