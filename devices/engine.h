#ifndef _ENGINE_H_
#define _ENGINE_H_

class Settings;

class Engine
{
  public:

    bool		isOpened;				// Порт устройства открыт
    int			eventId;
    int 		comPortNumber;
    int 		comPortBaundRate;
    int 		streamId;
  	unsigned int lastRegState;
  	unsigned int startTime;
  	int			bypassValve;
    bool		engineRotates;
  	int 		engineCommandTryCount;
  	int 		needFreq;
  	int 		currFreq;
  	int 		errorCode;
  	int			powerA;
  	unsigned char bypassMode;
  	unsigned long workTimeSec;

  	unsigned int regState;

  	virtual void Init(Settings* setting) = 0;
	virtual bool IsOpened() = 0;
	virtual bool OpenDevice() = 0;
	virtual bool CloseDevice() = 0;
	virtual bool engineStart(int freq) = 0;
	virtual bool engineStop() = 0;
	virtual bool engineTestConnection() = 0;
	virtual bool engineUpdate() = 0;
	Engine();
	virtual ~Engine();
};

#endif