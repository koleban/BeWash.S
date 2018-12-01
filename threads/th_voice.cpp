#include "../main.h"

PI_THREAD(VoiceWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.KKMWatch) return (void*)0;
	char myNote[200];
	while (settings->threadFlag.VoiceWatch)
	{
		break;
		delay_ms(1000);
	}
	return (void*)0;
}