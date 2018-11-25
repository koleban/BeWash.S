#include "../main.h"

using namespace DriverFR;

int errorCode = 0;
static DrvFR* drv;

//---------------------------------------------------------------------
int OpenPaymentDocument()
{
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
	drvFr->Price = price;
	drvFr->Quantity = quantity;
	drvFr->Summ1 = drvFr->Price * drvFr->Quantity;
	drvFr->Summ1Enabled = 0x00;
	drvFr->CheckType = (int)checkType;
	drvFr->TaxType = taxType;
	drvFr->TaxValue = 0;
	drvFr->TaxValueEnabled = 0x00;
	drvFr->DiscountOnCheck = 0x00;
	drvFr->PaymentItemSign = paymentItemSign;
	drvFr->PaymentTypeSign = paymentTypeSign;
	strncpy(drvFr->StringForPrinting, stringForPrinting, 40);

	int res = drvFr->FNOperation();
	if ((res != 0) && (res != 69) && (res != 70))
		printf("KKM: Error %d %s\n", drvFr->ResultCode, drvFr->ResultCodeDescription);
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
	drvFr->TaxType = (int)TDocumentTaxType::Tax6;

	strncpy(drvFr->StringForPrinting, "", 40);

	int res = drvFr->FNCloseCheckEx();
	if ((res != 0) && (res != 69) && (res != 70))
		printf("KKM: Error %d %s\n", drvFr->ResultCode, drvFr->ResultCodeDescription);
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
				printf("Устройство выдает данные. Ожидаем ...\n");
			errorCode = 0x10;
			if (dataReceiveCount++ > 5)
			{
				printf("Прерываем передачу данных от устройства!\n");
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
				printf("Открытая смена, 24 часа кончились. Закроем смену.\n");
			errorCode = 0x11;
			drv->FNCloseSession();
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::Blocked_WrongPasswordTaxInspector)
		{
			if (errorCode != 0x12)
				printf("Пароль налогового инспектора не корректен. Установим пароль администратора.\n");
			errorCode = 0x12;
			drv->Password = 30;
		}
		else if ((drv->ECRMode == (int)TECRMode::WorkWithFNDocument) || (drv->ECRMode == (int)TECRMode::WorkFNDocumentDone))
		{
			if (errorCode != 0x13)
				printf("Работа с подкладным документом ....\n");
			errorCode = 0x13;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::PrintingEKLZReport)
		{
			if (errorCode != 0x14)
				printf("Печать отчета ЕКЛЗ ....\n");
			errorCode = 0x14;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::PrintingFNReport)
		{
			if (errorCode != 0x15)
				printf("Печать фискального отчета ....\n");
			errorCode = 0x15;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::DigitPointChangeRight)
		{
			if (errorCode != 0x16)
				printf("Разрешение изменения положения десятичной точки. \n");
			errorCode = 0x16;
			drv->PointPosition = true;
			drv->SetPointPosition();
		}
		else if (drv->ECRMode == (int)TECRMode::WaitingDateConfirm)
		{
			if (errorCode != 0x17)
				printf("Ожидание подтверждения ввода даты. \n");
			errorCode = 0x17;
			int day = 24;
			int month = 10;
			int year = 2018;
			drv->Date.tm_mday = day;
			drv->Date.tm_mon = month - 1;
			drv->Date.tm_year = year - 1900;
			mktime(&drv->Date);
			time_t rawtime;
			struct tm * loctime = localtime(&rawtime);
			time(&rawtime);
			memcpy(&drv->Date, loctime, sizeof(drv->Date));
			drv->ConfirmDate();
		}
		else if (drv->ECRMode == (int)TECRMode::CloseWorkspace)
		{
			if (errorCode != 0x18)
				printf("Смена закрыта. Откроем смену.\n");
			errorCode = 0x18;
			drv->FNOpenSession();
		}
		else if (drv->ECRMode == (int)TECRMode::PrintFNDocument)
		{
			if (errorCode != 0x19)
				printf("Печать чека ....\n");
			errorCode = 0x19;
			break;
		}
		else if (drv->ECRMode == (int)TECRMode::OpenedDocument)
		{
			break;
		}
		else
			printf("Не корректный режим устройства. Ожидается \"Принтер готов\" или \"Смена открыта\" Debug: (%d) %s\n", drv->ECRMode, drv->ECRModeDescription);
		usleep(1000000);
		drv->GetECRStatus();
	}

	while (drv->ECRAdvancedMode != (int)TECRAdvancedMode::DeviceOK)
	{
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::NoPapper)
		{
			if (errorCode != 0x1A)
				printf("Отсутствует бумага. Ожидаем ...\n");
			errorCode = 0x1A;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::PrintingFNDocument)
		{
			if (errorCode != 0x1B)
				printf("Печатается фискальный документ. Ожидаем завершения ...\n");
			errorCode = 0x1B;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::PrintingFNDocument)
		{
			if (errorCode != 0x1C)
				printf("Печатается документ. Ожидаем завершения ...\n");
			errorCode = 0x1C;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::NoPapperError)
		{
			if (errorCode != 0x1D)
				printf("Внимание! Закончилась бумага!\n");
			errorCode = 0x1D;
		}
		if (drv->ECRAdvancedMode == (int)TECRAdvancedMode::WaitPrintContinue)
		{
			if (errorCode != 0x1E)
				printf("Продолжаем печать ...\n");
			errorCode = 0x1E;
			drv->ContinuePrinting();
		}
		usleep(1000000);
		drv->GetECRStatus();
	}
}
//---------------------------------------------------------------------
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
	char myNote[200];
	sprintf(myNote, "[THREAD] KKM: Online KMM thread init");
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	printf("Thread KKM is now working\n");
	QueueType valueKkm;

	struct tm* now;
	time_t rawtime;
	bool error = true;
	while (settings->threadFlag.KKMWatch)
	{
		drv = new DrvFR(settings->kkmParam.kkmPass, 0, 0, 1000, 0, TConnectionType::ctSocket, settings->kkmParam.kkmPort, settings->kkmParam.kkmAddr, true);
		sprintf(myNote, "[THREAD] KKM: Creating KKM driver instance and trying to connect to the device");
		db->Log(DB_EVENT_TYPE_KKM_THREAD, 0, 0, myNote);
		if (drv->Connect() > 0)
		{
			sprintf(myNote, "[THREAD] KKM: Connecting to KKM device successfully");
			db->Log(DB_EVENT_TYPE_KKM_THREAD, 0, 0, myNote);
			settings->workFlag.KKMWatch = 0;
			while (settings->threadFlag.KKMWatch)
			{
				time(&rawtime);
				now = localtime(&rawtime);
				fflush(stdout);
				if (drv->CheckConnection() != 1)
				{
					if (!error)
						printf ("[%s] Connection error!\n", asctime(now));
					error = true;
					fflush(stdout);
					drv->Disconnect();
					break;
				}
				if (error)
				{
					sprintf(myNote, "[THREAD] KKM: Connection worked. Device: %s", drv->UDescription);
					db->Log(DB_EVENT_TYPE_KKM_THREAD, 0, 0, myNote);
					printf ("[%s] Connection OK!\nDevice: %s\n", asctime(now), drv->UDescription);
				}
				error = false;
	
				CheckDevice(drv);
				int result = 0;
				while(queueKkm->QueueGet(&valueKkm) >= 0)
				{
					result = OpenPaymentDocument();
					sprintf(myNote, "[THREAD] KKM: Opening the payment document");
					db->Log(DB_EVENT_TYPE_KKM_FN, 0, 0, myNote);
					result = AddWareString(drv, TCheckType::Sale, 1, valueKkm.eventId, valueKkm.note, TTaxType::NoNds, TPaymentItemSign::Service, TPaymentTypeSign::Payment);
					if ((drv->ResultCode != 0) && (drv->ResultCode != 70))
						sprintf(myNote, "[THREAD] KKM: Adding service to payment document");
					else
						sprintf(myNote, "[ERROR] KKM: Can't add service to payment document: [%d] %s", drv->ResultCode, drv->ResultCodeDescription);
//					printf("%s\n", myNote);
					db->Log(DB_EVENT_TYPE_KKM_ERROR, valueKkm.eventId, 0, myNote);
					if ((drv->ResultCode != 0) && (drv->ResultCode != 70))
						{ queueKkm->QueuePut(valueKkm); CheckDevice(drv);delay_ms(settings->kkmParam.QueryTime); continue;}

					CheckDevice(drv);
					result = ClosePaymentDocument(drv, valueKkm.eventId);
					if ((drv->ResultCode != 0) && (drv->ResultCode != 70))
						sprintf(myNote, "[THREAD] KKM: Close the payment document");
					else
						sprintf(myNote, "[ERROR] KKM: Can't close the payment document: [%d] %s", drv->ResultCode, drv->ResultCodeDescription);
//					printf("%s\n", myNote);
					db->Log(DB_EVENT_TYPE_KKM_ERROR, valueKkm.eventId, 0, myNote);
					if (drv->ResultCode == 69)
						{ drv->CancelCheck(); queueKkm->QueuePut(valueKkm); CheckDevice(drv);delay_ms(settings->kkmParam.QueryTime); continue;}
					if ((drv->ResultCode != 0) && (drv->ResultCode != 70))
						{ queueKkm->QueuePut(valueKkm); CheckDevice(drv);delay_ms(settings->kkmParam.QueryTime); continue;}

					delay_ms(settings->kkmParam.QueryTime);
					CheckDevice(drv);
				}
				settings->workFlag.MoneyWatch = 0;
				delay_ms(settings->kkmParam.QueryTime);
			}
		}
		delay_ms(settings->kkmParam.QueryTime * 5);
	}
	if (db->Close())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	printf("[KKM]: Thread ended.\n");
	return (void*)0;
}