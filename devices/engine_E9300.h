#ifndef _ENGINE9300_H_
#define _ENGINE9300_H_

#define CTRL_ENGINE_DEVICE_ID		0x01

#define ERR_ENGINE_NO_ERROR			0x00;
#define	ERR_ENGINE_NO_CONNECTION	0x02

class Settings;

class Engine9300 : public Engine
{
	public:
    	static Engine9300* p_instance;
		static Engine* getInstance();

		void Init(Settings* setting);
		bool IsOpened();
		bool OpenDevice();
		bool CloseDevice();
		bool engineStart(int freq);
		bool engineStop();
		bool engineTestConnection();
		bool engineUpdate();

	private:
    	Engine9300();
};

#endif