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
		if (visaDevice->IsOpened())
			visaDevice->Update();
		else
			if (!visaDevice->OpenDevice()) {delay(5);}
		delay_ms(500);
	}
	visaDevice->CloseDevice();
	db->Log(DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "VISA (card terminal) device thread is closed");
	db->Close();
	printf("[DEBUG]: VISA Thread: Thread is terminate.\n");
	return (void*)0;
}
