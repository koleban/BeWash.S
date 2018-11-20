#ifndef _MONITOR_H_
#define _MONITOR_H_

#ifndef DWORD
#define DWORD			unsigned int
#define WORD			unsigned short
#define BYTE			unsigned char
#define SDWORD			int
#endif
class Settings;

class Monitor
{
private:
	static Monitor* p_instance;

	Monitor();
	Monitor( const Monitor& );
	Monitor& operator=( Monitor& );
	unsigned char DIN;
	unsigned char CLOCK;
	unsigned char LOAD;
public:
  	void Init(Settings* setting);
  	void ReInit();
	void setWorkPin(BYTE pinDIN, BYTE pinCLOCK, BYTE pinLOAD);
	void showText(char* textStr);
	void testDisplay();
	static Monitor* getInstance()
	{
		if(!p_instance)
			p_instance = new Monitor();
		return p_instance;
	}
};

#endif