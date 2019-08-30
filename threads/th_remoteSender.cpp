#include "../main.h"

//#pragma region ���������� "��������: ���������� ������"

PI_THREAD(RemoteSenderWatch)
{
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.RemoteCtrlThread) return (void*)0;

	////******************************************************
	////******************************************************
	////******************************************************
	////******************************************************

	while (settings->threadFlag.RemoteSenderThread)
	{
		delay_ms(100);
	}

	printf("[RemoteSender]: Thread ended.\n");
	return (void*)0;
}

//#pragma endregion
