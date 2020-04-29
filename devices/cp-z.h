#ifndef _CP_Z_H_
#define _CP_Z_H_

#ifndef BYTE
#define	BYTE 	unsigned char
#define WORD 	unsigned short
#define DWORD 	unsigned int
#endif

#ifndef DEBUG_PRINT
#define	DEBUG_PRINT 	0
#define	DEBUG_PRINT2 	0
#endif


extern int delayVal;

class Settings;


class RFID_CPZDevice
{
private:

	static RFID_CPZDevice* p_instance;
	RFID_CPZDevice();
	RFID_CPZDevice( const RFID_CPZDevice& );
	RFID_CPZDevice& operator=( RFID_CPZDevice& );
  	bool 		isOpened;
	int 		deviceDelayMs;
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
	static RFID_CPZDevice* getInstance()
	{
		if (!p_instance)
			p_instance = new RFID_CPZDevice();
		return p_instance;
	}
};

#endif