#ifndef _RFID_H_
#define _RFID_H_

class Settings;

class RFIDDevice
{
private:
	static RFIDDevice* p_instance;

	RFIDDevice();
	RFIDDevice( const RFIDDevice& );
	RFIDDevice& operator=( RFIDDevice& );
	BYTE buffer[1024];
	bool 		tempBool;
  	bool 		isOpened;
	int 		deviceDelayMs;
	int 		comPortNumber;
	int 		comPortBaundRate;
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
	static RFIDDevice* getInstance()
	{
		if(!p_instance)
			p_instance = new RFIDDevice();
		return p_instance;
	}
};

#endif