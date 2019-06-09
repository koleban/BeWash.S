#ifndef _VISA_H_
#define _VISA_H_

#ifndef BYTE
#define	BYTE 	unsigned char
#define WORD 	unsigned short
#define DWORD 	unsigned int
#endif

class Settings;

class VisaDevice
{
  public:

	BYTE 		buffer[1024];
    bool		isOpened;
	int 		deviceDelayMs;
	int 		comPortBaudRate;
	int 		comPortDataParity;
	int 		comPortDataBit;
	int 		comPortStopBit;
	int 		fd;
	int 		Work;

  	virtual void Init(Settings* setting) = 0;
	virtual bool IsOpened() = 0;
	virtual bool OpenDevice() = 0;
	virtual bool CloseDevice() = 0;
	virtual bool Update() = 0;
	VisaDevice();
	virtual ~VisaDevice();
};

#endif