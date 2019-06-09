#include "../main.h"

PaxD200* PaxD200::p_instance = NULL;

PaxD200::PaxD200()
{
	memset(buffer, 0, sizeof(buffer));
    isOpened = 0;
	deviceDelayMs = 50;
	comPortBaudRate = 9600;
	comPortDataParity = 0;
	comPortDataBit = 0;
	comPortStopBit = 0;
	fd = -1;
	Work = 0;
}

void PaxD200::Init(Settings* settings)
{
	isOpened = 0;
	comPortBaudRate = settings->visaParam.baudRate;
	comPortDataParity = settings->visaParam.dataParity;
	comPortDataBit = settings->visaParam.dataBit;
	comPortStopBit = settings->visaParam.stopBit;
}

bool PaxD200::IsOpened()
{
	return isOpened;
}

bool PaxD200::OpenDevice()
{
	if (IsOpened()) return true;
	isOpened = 0;

	fd = open(settings->visaParam.portName, O_RDWR|O_NONBLOCK);
	if (fd < 0) 
	{
		perror("[PaxD200::OpenDevice()] Unable to open device");
		isOpened = 0;
		return 0;
	}

	isOpened = 1;
	return isOpened;
}

bool PaxD200::CloseDevice()
{
	if (fd >= 0)
		{ close(fd); fd = -1;}
	isOpened = 0;
	return true;
}

bool PaxD200::Update()
{
	delay_ms(500);
	return true;
}

VisaDevice* PaxD200::getInstance()
{
    if(!PaxD200::p_instance)
        PaxD200::p_instance = new PaxD200();
    return (VisaDevice*)PaxD200::p_instance;
}
