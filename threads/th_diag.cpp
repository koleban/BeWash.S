#include "../main.h"

PI_THREAD(start_blink)
{
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	int device = DVC_ERROR_LED_1;
	int value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(300);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(300);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(300);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(300);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(100);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(100);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(100);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(100);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 1;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	delay_ms(100);
	/// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	device = DVC_ERROR_LED_1;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_2;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_3;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	device = DVC_ERROR_LED_4;
	value = 0;
	if (settings->getEnabledDevice(device))
	{
		int warmPinNum = settings->getPinConfig(device, 1);
		if ((warmPinNum != 0xFF) && (warmPinNum != 0x00))
		{
			setPinModeMy(warmPinNum, 0);
			pullUpDnControl (warmPinNum, PUD_DOWN) ;
			setGPIOState(warmPinNum, value);
		}
	}
	return (void*)0;
}
