#ifndef _PAX_D200_H_
#define _PAX_D200_H_

class Settings;

class PaxD200 : public VisaDevice
{
	public:
    	static PaxD200* p_instance;
		static VisaDevice* getInstance();

		void Init(Settings* setting);
		bool IsOpened();
		bool OpenDevice();
		bool CloseDevice();
		bool Update();
	private:
    	PaxD200();
};

#endif