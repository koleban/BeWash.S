#include "../main.h"
//
// Error CODE
// EE-H - Error EEPROM
// EE-1 - Error MCP21017
// EE-2 - Error COIN NOT DETECTED
// EE-3 - Error BILL NOT DETECTED

PI_THREAD(MonitorWatch)
{
	/// Îáùèå ïàðàìåòðû
	Settings* settings = Settings::getInstance();
	///
	/// Åñëè ÏÎÒÎÊ çàïðåùåí, òî çàâåðøàåìñÿ
	if (!settings->threadFlag.MonitorWatch) return (void*)0;

	/// Óñòðîéñòâî ÌÎÍÈÒÎÐ
	Monitor* monitor = Monitor::getInstance();

	/// Ïîñòàâèì ïàðàìåòðû äèñïëåÿ è èíèöèàëèçèðóåì åãî
	monitor->ReInit();
	if (settings->commonParams.showDemo)
	{
		/// Ïðîâåäåì òåñò äèñïëåÿ (0000 - 9999)
		monitor->testDisplay();
		/// Îæèäàåì çàïóñêà âñåõ îñòàëüíûõ
		delay_ms(1000);
	}

	int display_WaitTimeMs = 250;
	int counter = 0;
	int iter = 0;
	int errIter = 0;
	int reInitCounter = 0;
	int offs = 0;
	int dotPos = 0;
	char digit[40];
	char digit_out[40];
	char errText[30];
	bool tmpFlag = 0;

	///
	/// Ïîêà ÏÎÒÎÊ ÀÊÒÈÂÅÍ îáðàáàòûâàåì äàííûå
	///
	while (settings->threadFlag.MonitorWatch)
	{
		// Ïîäòâåðæäàåì ÷òî ïîòîê ðàáîòàåò
		settings->workFlag.MonitorWatch = 0;

		/// Îáðàáîòêà ðåèíèöèàëèçàöèè äèñïëåÿ
		if (reInitCounter++ > (settings->commonParams.display_ReInitTimeMs/display_WaitTimeMs))
		{ reInitCounter = 0; monitor->ReInit();}
		/// Îáðàáîòêà ñêðîëèíãà òåêñòà
		if (iter++ > (settings->commonParams.delayScrollingTextMs/display_WaitTimeMs))
		{ iter = 0; offs++; }
		///
		/// ÎÒÎÁÐÀÆÅÍÈÅ ÈÍÔÎÐÌÀÖÈÈ ÎÒÍÎÑÈÒÅËÜÍÎ ÑÎÑÒÎßÍÈß È ÒÈÏÀ ÁÎÊÑÀ
		///
		/// Îòîáðàæåíèå ÊÀÑÑÛ åñëè íàæàòà ÊÍÎÏÊÀ ÈÍÊÀÑÀÖÈÈ
		///
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
//		if (status.extDeviceInfo.collectionButton)
		if (deviceWorkMode == TDeviceWorkMode::CollectionMode)
		{
			counter = status.intDeviceInfo.allMoney;
			memset(&digit, 0, sizeof(digit));
			sprintf(&digit[0], "%4d", counter);
			sprintf(&digit_out[0], "%4d", counter);
			int len = strlen(&digit[0]);
			if (len > 4)
			{
				memset(&digit[0], 0x20, len + 8);
				sprintf(&digit[4], "%d", counter);
				digit[strlen(&digit[0])] = 0x20;
				digit[len+8] = 0x00;
				len = strlen(&digit[0]);
				if (len-4 <= offs) offs = 0;
				sprintf(&digit_out[0], "%s", &digit[offs]);
			}

			memcpy(status.extDeviceInfo.monitor_currentText, digit_out, 4);
			monitor->showText(&digit_out[0]);
		}
		///
		/// Îòîáðàæåíèå òåêóùåãî áàëàíñà åñëè åñòü ÎÁÙÈÉ ÏÎÒÎÊ ÂÍÅØÍÅÃÎ ÁËÎÊÀ
		///
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
//		if ((settings->threadFlag.ExtCommonThread != 0) && (status.extDeviceInfo.collectionButton == 0))
		if ((settings->threadFlag.ExtCommonThread) && (deviceWorkMode == TDeviceWorkMode::WorkMode))
		{
			counter = status.extDeviceInfo.remote_currentBalance;
			memset(&digit, 0, sizeof(digit));
			sprintf(&digit[0], "%4d", counter);
			sprintf(&digit_out[0], "%4d", counter);
			int len = strlen(&digit[0]);
			if (len > 4)
			{
				memset(&digit[0], 0x20, len * 3);
				sprintf(&digit[len], "%d", counter);
				digit[strlen(&digit[0])] = 0x20;
				digit[len * 3] = 0x00;
				len = strlen(&digit[0]);
				if (len-4 <= offs) offs = 0;
				sprintf(&digit_out[0], "%s", &digit[offs]);
			}

			if (((globalThreadId == 0)  || ((externalCtrl < 4) && (globalThreadId > 0))) && (settings->threadFlag.IntCommonThread == 0))
			{
				sprintf(&digit_out[0], "%s", "____");
			}
			memcpy(status.extDeviceInfo.monitor_currentText, digit_out, 4);
			monitor->showText(&digit_out[0]);
		}
		///
		/// Îòîáðàæåíèå ÑÒÀÒÓÑÎÂ è ÎØÈÁÎÊ åñëè åñòü ÎÁÙÈÉ ÏÎÒÎÊ ÂÍÓÒÐÅÍÍÅÃÎ ÁËÎÊÀ
		///
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		if ((settings->threadFlag.IntCommonThread != 0) && (!settings->threadFlag.ExtCommonThread) && (deviceWorkMode == TDeviceWorkMode::WorkMode))
		{
			BYTE intFlag = 0;
			for (int ind=0; ind<sizeof(settings->intErrorCode); ind++)
				intFlag |= *(((BYTE*)&settings->intErrorCode)+ind);
			if (intFlag)
			{
				for (int ind=0; ind<sizeof(settings->intErrorCode); ind++)
					intFlag = (*(((BYTE*)&settings->intErrorCode)+ind) == 0)?intFlag:*(((BYTE*)&settings->intErrorCode)+ind);
				sprintf(&errText[0], "E%03d", intFlag);
			}
			else
			{
				if (errIter++ > 20)
				{
					if (errIter < 30)
					{
						unsigned long wrkTimeMin = (unsigned long)((gEngineFullWorkTime+engine->workTimeSec)/60);
						if (errIter%2)
							sprintf(&errText[0], "%04d", (unsigned long)(wrkTimeMin/60));
						else
							sprintf(&errText[0], "%s", "    ");
					}
					else if (errIter < 40)
					{
						sprintf(&errText[0], "BP-%1d", engine->bypassMode);
					}
					else if (errIter < 50)
					{
						sprintf(&errText[0], "%4d", status.extDeviceInfo.remote_currentBalance);
					}
					else if (errIter < 60)
					{
						sprintf(&errText[0], "P%3d", status.intDeviceInfo.program_currentProgram);
					}
					else if (errIter < 70)
					{
						sprintf(&errText[0], "5EC%1d", keyErrorCount);
					}
					else if (errIter < 80)
					{
						sprintf(&errText[0], " %02dG", status.extDeviceInfo.extCurrentTemp);
					}
					if (errIter > 80) errIter = 0;
				}
				else
				{
					dotPos++;
					switch(dotPos)
					{
						default:
							dotPos = 0;
							sprintf(&errText[0], "%s", "    ");
							break;
						case 1:
							sprintf(&errText[0], "%s", "   -");
							break;
						case 2:
							sprintf(&errText[0], "%s", "  - ");
							break;
						case 3:
							sprintf(&errText[0], "%s", " -  ");
							break;
						case 4:
							sprintf(&errText[0], "%s", "-   ");
							break;
						case 5:
							sprintf(&errText[0], "%s", "i   ");
							break;
						case 6:
							sprintf(&errText[0], "%s", "_   ");
							break;
						case 7:
							sprintf(&errText[0], "%s", " _  ");
							break;
						case 8:
							sprintf(&errText[0], "%s", "  _ ");
							break;
						case 9:
							sprintf(&errText[0], "%s", "   _");
							break;
						case 10:
							sprintf(&errText[0], "%s", "   l");
							break;
						case 11:
							sprintf(&errText[0], "%s", "   L");
							break;
						case 12:
							sprintf(&errText[0], "%s", "   ~");
							break;
						case 13:
							sprintf(&errText[0], "%s", "  ~ ");
							break;
						case 14:
							sprintf(&errText[0], "%s", " ~  ");
							break;
						case 15:
							sprintf(&errText[0], "%s", "~   ");
							break;
						case 16:
							sprintf(&errText[0], "%s", "{   ");
							break;
						case 17:
							sprintf(&errText[0], "%s", "-   ");
							break;
						case 18:
							sprintf(&errText[0], "%s", " -  ");
							break;
						case 19:
							sprintf(&errText[0], "%s", "  - ");
							break;
						case 20:
							sprintf(&errText[0], "%s", "   -");
							break;
					}
	        	}
			}

			memcpy(status.intDeviceInfo.monitor_currentText, &errText[0], 4);
    		monitor->showText(&errText[0]);

			if (errIter > 20) delay_ms(100);
		}

		///
		/// Îòîáðàæåíèå â ðåæèìå ÍÀÑÒÐÎÉÊÀ
		///
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (deviceWorkMode == TDeviceWorkMode::SettingsMode)
		{
			memset(&digit, 0, sizeof(digit));
			sprintf(&digit[0], "%1d_%2d", currentPrgPriceIndex+1, eepromPrgPrice[currentPrgPriceIndex]);
			sprintf(&digit_out[0], "%1d_%2d", currentPrgPriceIndex+1, eepromPrgPrice[currentPrgPriceIndex]);
			int len = strlen(&digit[0]);
			if (len > 4)
			{
				memset(&digit[0], 0x20, len + 8);
				sprintf(&digit[4], "%d", counter);
				digit[strlen(&digit[0])] = 0x20;
				digit[len+8] = 0x00;
				len = strlen(&digit[0]);
				if (len-4 <= offs) offs = 0;
				sprintf(&digit_out[0], "%s", &digit[offs]);
			}

			memcpy(status.extDeviceInfo.monitor_currentText, digit_out, 4);
			monitor->showText(&digit_out[0]);
		}

		// <<<<<<<<<<<<<
		// END MAIN LOOP

		delay_ms(display_WaitTimeMs);
	}

	char emptyText[] = "    ";
	monitor->showText(emptyText);
	printf("[7Seg]: Thread ended.\n");
	return (void*)0;
}
