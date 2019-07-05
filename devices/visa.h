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

  	void Init(Settings* setting);
	bool IsOpened();
	bool OpenDevice();
	bool CloseDevice();
	bool Update();
	VisaDevice();
	~VisaDevice();
};

#endif