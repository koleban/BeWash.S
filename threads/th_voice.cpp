#include "../main.h"

PI_THREAD(VoiceWatch)
{
	Settings* settings = Settings::getInstance();
	if (!settings->threadFlag.VoiceWatch) return (void*)0;
	char myNote[200];
	int lastProgressState = (int)1;
	while (settings->threadFlag.VoiceWatch)
	{
		if (lastProgressState != btnMasterProgress)
		{
			lastProgressState = btnMasterProgress;
			switch(btnMasterProgress)
			{
				case TBtnMasterProgress::Idle:
					printf("[VOICE] Progress state: TBtnMasterProgress::Idle\n");
					break;
				case TBtnMasterProgress::WaitSelectDevice:
					printf("[VOICE] Progress state: TBtnMasterProgress::WaitSelectDevice\n");
					break;
				case TBtnMasterProgress::SendingCommand:
					printf("[VOICE] Progress state: TBtnMasterProgress::SendingCommand\n");
					break;
				case TBtnMasterProgress::PrintCheck:
					printf("[VOICE] Progress state: TBtnMasterProgress::PrintCheck\n");
					break;
				case TBtnMasterProgress::CollectionMode:
					printf("[VOICE] Progress state: TBtnMasterProgress::CollectionMode\n");
					break;

				default:
					printf("[VOICE] Progress state: UNKNOWN STATE\n");
					break;
			}
		}

/*
		for (int i=1; i <= settings->modbus.slaveCount; i++)
		{
			remoteCtrl[i].cmdRead = 1;
			remoteCtrl[i].cmdResult = 0xFFFFFFFFUL;
			remoteCtrl[i].doCmd = 1;
		}
		delay_ms(2000);
		for (int i=1; i <= settings->modbus.slaveCount; i++)
		{
			remoteCtrl[i].cmdWrite = 1;
			remoteCtrl[i].devImpVal[0] = 5;	// 10 rur/imp
			remoteCtrl[i].devImpVal[1] = 5;	// 1  rur/imp
			remoteCtrl[i].doCmd = 1;
		}
		delay_ms(2500);
*/
		delay_ms(250);
	}
	return (void*)0;
}