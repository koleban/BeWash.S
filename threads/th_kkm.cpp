#include "../main.h"

using namespace DriverFR;

int errorCode = 0;
static DrvFR* drv;

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

PI_THREAD(KKMWatch)
{
	printf("KKM is starting \n");
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.KKMWatch) return (void*)0;
	printf("KKM is init DB \n");
	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[200];
	sprintf(myNote, "[THREAD] KKM: Online KMM thread init");
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	printf("KKM is wpork done \n");

	struct tm* now;
	time_t rawtime;
	bool error = true;
	while (settings->threadFlag.KKMWatch)
	{
		drv = new DrvFR(settings->kkmParam.kkmPass, 0, 0, 1000, 0, TConnectionType::ctSocket, settings->kkmParam.kkmPort, settings->kkmParam.kkmAddr, true);
		printf("KKM is connection to KKM \n");
		if (drv->Connect() > 0)
		{
			printf("KKM is connected ...\n");
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
					printf ("[%s] Connection OK!\nDevice: %s\n", asctime(now), drv->UDescription);
				error = false;
	
				CheckDevice(drv);

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
