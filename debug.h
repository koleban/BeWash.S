#ifndef _DEBUG_H_
#define _DEBUG_H_

class Settings;

class DebugLog
{
	private:
		int temp;
	public:
		DebugLog();
	  	void Init(Settings* settings);
};

#endif	 // _DEBUG_H_