#include "../main.h"

//#pragma region Ïîäñèñòåìà "ÂÍÓÒÐÅÍÍÈÉ ÁÎÊÑ: ÎÒÑ×ÅÒ ÂÐÅÌÅÍÈ"

PI_THREAD(TimeTickThread)
{
	int winterDelay = 0;
	long waitTime = time(NULL);
	long waitTimeEng = time(NULL);
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.TimeTickThread) return (void*)0;
	int prgPrice = settings->progPrice[status.intDeviceInfo.program_currentProgram];
	int progStopBP = settings->progStopBP[status.intDeviceInfo.program_currentProgram];
	int prgDelay = 0;
	if (prgPrice > 0)
		prgDelay = (int)(60000/prgPrice);
	else
		prgDelay = 60000;
	int bypassCount;
	int lastProgramm = status.intDeviceInfo.program_currentProgram;
	while (settings->threadFlag.TimeTickThread)
	{
		settings->workFlag.TimeTickThread = 0;
		settings->busyFlag.TimeTickThread++;
		prgPrice = settings->progPrice[status.intDeviceInfo.program_currentProgram];
		if ((prgPrice > 0) && (settings->discountSize != 0))
			prgPrice = ((long)(settings->progPrice[status.intDeviceInfo.program_currentProgram]
					- (settings->progPrice[status.intDeviceInfo.program_currentProgram] * (((double)settings->discountSize)/100))));
		if (prgPrice < 1) prgPrice = 1;
		progStopBP = settings->progStopBP[status.intDeviceInfo.program_currentProgram];
		prgDelay = 0;
		if (prgPrice > 0)
			prgDelay = (int)(60000/prgPrice);

		/// ************************************************************
		//  ÎÁÐÀÁÎÒÊÀ ÇÀÄÅÐÆÊÈ ÄËß ÂÊËÞ×ÅÍÈÉ ÏÐÎÃÐÀÌÌÛ ÐÓÁËÈ => ÑÅÊÓÍÄÛ
		/// ************************************************************
		int currPrg_Tmp = status.intDeviceInfo.program_currentProgram;
		int dCounter = 100;
		while (dCounter-- > 1)
		{
			if (prgPrice > 0)
			{
				settings->workFlag.TimeTickThread = 0;
				//if (settings->progPrice[status.intDeviceInfo.program_currentProgram] > 0)
				//	prgDelay = (int)(60000/settings->progPrice[status.intDeviceInfo.program_currentProgram]);
				prgDelay = (int)(60000/prgPrice);
				if (status.intDeviceInfo.program_currentProgram != currPrg_Tmp) break;
				usleep(prgDelay*10);
			}
		}

		/// ************************************************************
		//  ÎÁÐÀÁÎÒÊÀ ÓÌÅÍÜØÅÍÈß ÁÀËÀÍÑÀ
		/// ************************************************************
		if ((status.intDeviceInfo.money_currentBalance > 0) && (prgPrice > 0))
		{
			if (
				(!settings->winterMode.winterDelay) ||
				(winterDelay++ >= settings->progWinterDelay[status.intDeviceInfo.program_currentProgram])
			   )
			{
				if (
				    (winterDelay == settings->progWinterDelay[status.intDeviceInfo.program_currentProgram]+1) &&
				    (settings->progWinterDelay[status.intDeviceInfo.program_currentProgram] > 0)
				   )
					printf("[DEBUG] TIMER: Done winter time for engine work [%d]\n", winterDelay-1);
				if (progStopBP == 0)
				{
					status.intDeviceInfo.money_currentBalance--;
				}
				else
				{
					if (!engine->bypassMode)
						status.intDeviceInfo.money_currentBalance--;
				}
			}
			else
			{
				printf("[DEBUG] TIMER: Not count time [%d] [%d] [%d]\n", settings->winterMode.winterDelay, winterDelay, settings->progWinterDelay[status.intDeviceInfo.program_currentProgram]);
			}

			if (
				(winterDelay == 1) &&
				(winterDelay++ >= settings->progWinterDelay[status.intDeviceInfo.program_currentProgram])
			   )
				printf("[DEBUG] TIMER: Start Winter time for engine work [%d]\n", settings->progWinterDelay[status.intDeviceInfo.program_currentProgram]);
			/// ************************************************************
			//  ÎÁÐÀÁÎÒÊÀ ÀÂÒÎÑÒÎÏÀ ÏÎ ÁÀÉÏÀÑÓ
			/// ************************************************************
			if (settings->useAutoStop)
			{
				if ((engine->bypassMode) && (engine->needFreq > 100)) bypassCount++;
				else 					bypassCount = 0;
	       		if (bypassCount >= settings->countAutoStop)
				{
					printf ("[DEBUG]: >> AUTOSTOP (SET: %2d; CUR: %2d; C_PRG: %2d; BAL: %5d)\n", settings->countAutoStop, bypassCount, status.intDeviceInfo.program_currentProgram, status.intDeviceInfo.money_currentBalance);
					status.extDeviceInfo.button_lastEvent = 1;
					status.extDeviceInfo.button_currentLight = 1;
					status.intDeviceInfo.program_currentProgram = 1;
					status.intDeviceInfo.extPrgNeedUpdate = 1;
					// Kolebanov
					// 27.07.2018
					// >>>
					int delayTime = 200;
		   			while ((delayTime-- > 0) && (settings->busyFlag.ButtonWatch)) {delayTime--; delay_ms(1); continue;}
   					settings->busyFlag.ButtonWatch++;
					status.extDeviceInfo.button_newEvent = 1;
					status.extDeviceInfo.button_lastEvent = 1;
					status.extDeviceInfo.button_currentLight = 1;
					status.intDeviceInfo.program_currentProgram = 1;
					status.intDeviceInfo.extPrgNeedUpdate = 1;
					settings->busyFlag.ButtonWatch--;
					// <<<
					bypassCount = 0;
				}
			}
		}


		/// ************************************************************
		//  ÎÁÐÀÁÎÒÊÀ ÏÀÓÇÛ Â ÎÒÑ×ÅÒÅ ÏÐÈ ÈÇÌÅÍÅÍÈÈ ÏÐÎÃÐÀÌÌÛ ÄËß ÏÐÎÊÀ×ÊÈ ÌÀÃÈÑÒÐÀËÈ
		/// ************************************************************
		if (lastProgramm != status.intDeviceInfo.program_currentProgram)
		{
			lastProgramm = status.intDeviceInfo.program_currentProgram;
			winterDelay = 0;
			bypassCount = 0; // Åñëè ïîëüçîâàòåëü ñìåíèë ïðîãðàììó òî ñáðîñèì òàéìåð ÀÂÒÎÑÒÎÏÀ
		}

		/// ************************************************************
		//  ÎÁÐÀÁÎÒÊÀ ÃËÞÊ ÏÎ ÁÀËÀÍÑÓ
		/// ************************************************************
		if (status.intDeviceInfo.money_currentBalance < 0)
			status.intDeviceInfo.money_currentBalance = 0;

		/// ************************************************************
		//  ÎÁÐÀÁÎÒÊÀ ÎÊÎÍ×ÀÍÈß ÂÐÅÌÅÍÈ
		/// ************************************************************
		if ((status.intDeviceInfo.program_currentProgram != 0) && (status.intDeviceInfo.money_currentBalance <= 0))
		{
			status.intDeviceInfo.program_currentProgram = 0;
			status.intDeviceInfo.extPrgNeedUpdate = 1;
		}

		settings->busyFlag.TimeTickThread--;
		if (settings->busyFlag.TimeTickThread > 200) settings->busyFlag.TimeTickThread = 0;

		if ((status.intDeviceInfo.program_currentProgram > 0) && (status.intDeviceInfo.program_currentProgram < 99))
		{
			waitTime = time(NULL);
			waitTimeEng = time(NULL);
			winterModeActive = 0;
			winterModeEngineActive = 0;
		}

		if ((settings->winterMode.winterMode) && (settings->threadFlag.IntCommonThread))
		{
			if ((time(NULL) - waitTime) > settings->winterMode.winterWaitTime)
			{
				winterModeActive++;
				waitTime = time(NULL);
				if (winterModeActive == 1)
					printf("[DEBUG] TIMER: WinterMode: Winter mode activate\n");
			}
			if (
				(winterModeActive > 0) &&
				(settings->winterMode.winterEngineWork > 0) &&
				((time(NULL) - waitTimeEng - settings->winterMode.winterEngineWorkTime) > settings->winterMode.winterEngineWorkWait))
			{
				waitTimeEng = time(NULL);
				winterModeEngineActive++;
				if (winterModeEngineActive == 1)
					printf("[DEBUG] TIMER: WinterMode: Winter mode engine start ...\n");
			}
		}
		else
			winterModeActive = 0;
		delay_ms(25);
	}
	printf("[TIME]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
