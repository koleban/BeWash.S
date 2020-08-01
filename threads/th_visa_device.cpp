#include "../main.h"

VisaDevice* visaDevice;

PI_THREAD(VisaDeviceWatch)
{
	/// Общие параметры
	Settings* settings = Settings::getInstance();
	/// Если ПОТОК запрещен, то завершаемся
	if (!(settings->threadFlag.VisaDeviceThread)) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	db->Log(DB_EVENT_TYPE_THREAD_INIT, 		0, 0, "[THREAD] VISA Device: VISA thread init");
	db->Log(DB_EVENT_TYPE_DVC_BUTTON_INIT, 	0, 0, "VISA (card terminal) device thread opened");

	VisaDevice* visaDevice = new VisaDevice();

	visaDevice->Init(settings);

	if (settings->debugFlag.VisaDeviceThread)
		printf("[DEBUG] ButtonTerminalThread: Debug information is showed\n");

	while (settings->threadFlag.VisaDeviceThread)
	{
		if ((settings->visaParam.workMode == 2) && (settings->visaParam.paymentSumm != 0) && (payInfo.inUse == 0))
		{
			payInfo.summ = settings->visaParam.paymentSumm;
			payInfo.deviceNum = settings->commonParams.deviceId;
			cp2utf("Оплата картой за услуги автомойки", payInfo.note);
			sprintf(payInfo.r_phone, settings->visaParam.tel);
			sprintf(payInfo.r_email, settings->visaParam.email);
			payInfo.inUse = 1;
		}
		if (visaDevice->IsOpened())
			visaDevice->Update();
		else
			if (!visaDevice->OpenDevice()) {delay(5);}
		if ((visaDevice->IsOpened()) && (settings->visaParam.workMode == 2) && (payInfo.inUse == 0))
		{
			if (payInfo.result == PAY_RESULT_OK)
			{
				status.intDeviceInfo.money_currentBalance += settings->visaParam.paymentSumm;
				char strTmp256[1024];
				int devId = settings->commonParams.deviceId;
				if (devId > 100) devId -= 100;
				sprintf(strTmp256, "%s (П:%d)", settings->kkmParam.ServiceName, devId);
				queueKkm->QueuePut(0, settings->visaParam.paymentSumm, 1, strTmp256);
				db->Log(DB_EVENT_TYPE_VISA_PAY_DOC_OK, settings->visaParam.paymentSumm, payInfo.result, payInfo.note);
				time_t eventTime = time(NULL);
				db->CreateKKMVisaDoc(eventTime, settings->commonParams.deviceId, double(settings->visaParam.paymentSumm), strTmp256);
			}
		}
		delay_ms(500);
	}
	visaDevice->CloseDevice();
	db->Log(DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "VISA (card terminal) device thread is closed");
	db->Close();
	printf("[DEBUG]: VISA Thread: Thread is terminate.\n");
	return (void*)0;
}
