#ifndef _CRT288_H_
#define _CRT288_H_

#ifndef BYTE
#define	BYTE 	unsigned char
#define WORD 	unsigned short
#define DWORD 	unsigned int
#endif

#ifndef DEBUG_PRINT
#define	DEBUG_PRINT 	0
#define	DEBUG_PRINT2 	0
#endif


#ifndef CRT_NOERR
#define CRT_ACK 			0x06
#define CRT_NAK 			0x15
#define CRT_NOERR			0
#define CRT_CMDWRTERR		1
#define CRT_CMDRDERR		2
#define CRT_CMDCRCERR		3
#define CRT_CMDNAKERR		10
#endif

#define COMM_DELAY	(delayVal)
extern int delayVal;

BYTE cmdDeviceInit(int fd, BYTE cardLock);
DWORD cmdReadCardID(int fd);
BYTE cmdLedCtrl(int fd, BYTE state, BYTE blink, BYTE ledNum);
void calcCrc(BYTE* command, WORD bufLen);

class Settings;


class Crt288KDevice
{
private:

	static Crt288KDevice* p_instance;
	Crt288KDevice();
	Crt288KDevice( const Crt288KDevice& );
	Crt288KDevice& operator=( Crt288KDevice& );
	BYTE buffer[1024];
	bool 		tempBool;
  	bool 		isOpened;
	int 		deviceDelayMs;
	int 		comPortNumber;
	int 		comPortBaundRate;
	int 		fd;
    BYTE 		pinLock;
    BYTE 		pinUnlock;
    bool		lockError;
public:
	long 		errorCount;
	bool 		cardPresent;
	BYTE		cardNumber[6];
	DWORD		digCardNumber;
  	void 		Init(Settings* setting);
	bool 		OpenDevice();
	bool 		IsOpened();
	bool 		CloseDevice();
	DWORD 		cmdPoll();
	DWORD 		cmdMultiplePoll(BYTE pollCount);
	void 		Lock(bool lockState);       // Green - Unlock, Red - Lock
	void 		LockError(); 				// Turn off all LED
	void 		ErrorBlink();				// Blink Red LED
	void 		OKBlink();					// Blink Green LED
	void 		Detect();
	bool 		cmdTest();
	static Crt288KDevice* getInstance()
	{
		if (!p_instance)
			p_instance = new Crt288KDevice();
		return p_instance;
	}
};

#endif