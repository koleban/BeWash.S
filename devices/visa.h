#ifndef _VISA_H_
#define _VISA_H_

#ifndef BYTE
#define	BYTE 	unsigned char
#define WORD 	unsigned short
#define DWORD 	unsigned int
#endif

#include "../protocols/serial.h"

struct PayInfo
{
	public:
	unsigned long id;
	int 	inUse;
	double 	summ;
	int 	deviceNum;
	char 	note[128];
	char 	r_email[128];
	char 	r_phone[128];
	char	transactionId[1024];
	int 	result;
};

static const char *mTransactionId = NULL;
static char sTransactionId[1024] = {0};

class Settings;

class VisaDevice
{
	public:

	char user_name[100] = {0};
	char user_password[100] = {0};
	int debug_enabled = 0;

	unsigned long transactionCount = 1;
	int errorCode = 0;

	void Init(Settings* setting);
	bool IsOpened();
	bool OpenDevice();
	bool CloseDevice();
	bool Update();
	bool DoPayment(PayInfo* payInfo);
	VisaDevice();
	~VisaDevice();
};

#endif