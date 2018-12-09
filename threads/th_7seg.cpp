//
// Error CODE
// EE-H - Error EEPROM
// EE-1 - Error MCP21017
// EE-2 - Error COIN NOT DETECTED
// EE-3 - Error BILL NOT DETECTED

#include "../main.h"

PI_THREAD(MonitorWatch)
{
	/// ����� ���������
	Settings* settings = Settings::getInstance();
	///
	/// ���� ����� ��������, �� �����������
	if (!settings->threadFlag.MonitorWatch) return (void*)0;

	/// ���������� �������
	Monitor* monitor = Monitor::getInstance();

	/// �������� ��������� ������� � �������������� ���
	monitor->ReInit();
	if (settings->commonParams.showDemo)
	{
		/// �������� ���� ������� (0000 - 9999)
		monitor->testDisplay();
		/// ������� ������� ���� ���������
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
	/// ���� ����� ������� ������������ ������
	///
	while (settings->threadFlag.MonitorWatch)
	{
		// ������������ ��� ����� ��������
		settings->workFlag.MonitorWatch = 0;

		/// ��������� ��������������� �������
		if (reInitCounter++ > (settings->commonParams.display_ReInitTimeMs/display_WaitTimeMs))
		{ reInitCounter = 0; monitor->ReInit();}
		/// ��������� ��������� ������
		if (iter++ > (settings->commonParams.delayScrollingTextMs/display_WaitTimeMs))
		{ iter = 0; offs++; }
		///
		/// ����������� ���������� ������������ ��������� � ���� �����
		///
		/// ����������� ��������� ������ �� EEPROM
		///
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (settings->intErrorCode.MainWatch == 222)
		{
			if (errIter++ > 20)
			{
				settings->intErrorCode.MainWatch = 0;
				errIter = 0;
			}
			memset(&digit_out, 0, sizeof(digit_out));
			tmpFlag ^= (iter % 5 == 0);
			if (tmpFlag)
				sprintf(&digit_out[0], "EE- ");
			else
				sprintf(&digit_out[0], "EE-H");


			memcpy(status.intDeviceInfo.monitor_currentText, digit_out, 4);
			memcpy(status.extDeviceInfo.monitor_currentText, digit_out, 4);
			monitor->showText(&digit_out[0]);
			delay_ms(200);
			continue;
		}
		if (settings->intErrorCode.MainWatch == 221)
		{
			if (errIter++ > 20)
			{
				settings->intErrorCode.MainWatch = 0;
				errIter = 0;
			}
			memset(&digit_out, 0, sizeof(digit_out));
			tmpFlag ^= (iter % 5 == 0);
			if (tmpFlag)
				sprintf(&digit_out[0], "EE- ");
			else
				sprintf(&digit_out[0], "EE-1");


			memcpy(status.intDeviceInfo.monitor_currentText, digit_out, 4);
			memcpy(status.extDeviceInfo.monitor_currentText, digit_out, 4);
			monitor->showText(&digit_out[0]);
			delay_ms(200);
			continue;
		}
		if (settings->intErrorCode.MainWatch == 220)
		{
			if (errIter++ > 20)
			{
				settings->intErrorCode.MainWatch = 0;
				errIter = 0;
			}
			memset(&digit_out, 0, sizeof(digit_out));
			tmpFlag ^= (iter % 5 == 0);
			if (tmpFlag)
				sprintf(&digit_out[0], "EE- ");
			else
				sprintf(&digit_out[0], "EE-2");


			memcpy(status.intDeviceInfo.monitor_currentText, digit_out, 4);
			memcpy(status.extDeviceInfo.monitor_currentText, digit_out, 4);
			monitor->showText(&digit_out[0]);
			delay_ms(200);
			continue;
		}
		if (settings->intErrorCode.MainWatch == 219)
		{
			if (errIter++ > 20)
			{
				settings->intErrorCode.MainWatch = 0;
				errIter = 0;
			}
			memset(&digit_out, 0, sizeof(digit_out));
			tmpFlag ^= (iter % 5 == 0);
			if (tmpFlag)
				sprintf(&digit_out[0], "EE- ");
			else
				sprintf(&digit_out[0], "EE-3");


			memcpy(status.intDeviceInfo.monitor_currentText, digit_out, 4);
			memcpy(status.extDeviceInfo.monitor_currentText, digit_out, 4);
			monitor->showText(&digit_out[0]);
			delay_ms(200);
			continue;
		}
		///
		/// ����������� ���������� ������������ ��������� � ���� �����
		///
		/// ����������� ����� ���� ������ ������ ���������
		///
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (status.extDeviceInfo.collectionButton)
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
		/// ����������� �������� ������� ���� ���� ����� ����� �������� �����
		///
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		if ((settings->threadFlag.ExtCommonThread != 0) && (status.extDeviceInfo.collectionButton == 0))
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
		/// ����������� �������� � ������ ���� ���� ����� ����� ����������� �����
		///
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		if ((settings->threadFlag.IntCommonThread != 0) && (settings->threadFlag.ExtCommonThread == 0) && (status.extDeviceInfo.collectionButton == 0))
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

			if (licenseError > 10)
			{
				sprintf(&errText[0], "E-06%s", "");
				delay_ms(1000);
			}
			memcpy(status.intDeviceInfo.monitor_currentText, &errText[0], 4);
    		monitor->showText(&errText[0]);

			if (errIter > 20) delay_ms(100);
		}

		delay_ms(display_WaitTimeMs);
	}

	char emptyText[] = "    ";
	monitor->showText(emptyText);
	printf("[7Seg]: Thread ended.\n");
	return (void*)0;
}
