#include "../main.h"                                                  

//#pragma region ���������� "����� ��������� ����������� �����"

int turnedRelayPrg = -1;
int turnedLight = 0;
int winterModeActivePrev = 0;
int winterModeEngineActivePrev = 0;

unsigned int summWithCard = 0;
unsigned int summWithCardPrev = 0;

MoneyCoinInfo inCoinInfo;
MoneyCoinInfo inMoneyInfo;
MoneyCoinInfo logCoinInfo;
MoneyCoinInfo logMoneyInfo;
char iddqd_etalon[20];

double dsSize = 0;
extern int dayLightWork;

///////////////////////////////////////////////////////////////////////////////////
/// ��������� ����
/// program - ����� ���������, ������� ���������� ����� ���� ���������
/// ��� ��������� ��� ����� ����� ������� � ���������� �������� ��������� ����
/// � ������ ��������� ����������� 50, ��� ��������� ������� ����� 50 - ���������
///////////////////////////////////////////////////////////////////////////////////
void helper_TurnRelaysOffProgramm(unsigned int program)
{
	Settings* 		settings 	= Settings::getInstance();

	if (program < 0) program = 0;

	if (program < 16)
	{
		unsigned int newPrgNumber = program;
		for (int index=0; index<4; index++)
		{
			int relNum = ((settings->progRelay[newPrgNumber] >> (index*8)) & 0xFF);

			if (relNum < 16)
			{
				status.intDeviceInfo.relay_currentVal[relNum] = 0x00;
				if (((settings->progLimitRelay[newPrgNumber] >> (index*8)) & 0xFF) < 16)
					status.intDeviceInfo.relay_currentVal[((settings->progLimitRelay[newPrgNumber] >> (index*8)) & 0xFF)] = 0x00;
			}
		}
	}
	else if ((program >= 50) && (program < 66))
	{
		unsigned int newPrgNumber = program - 50;
		for (int index=0; index<4; index++)
		{
			int relNum = ((settings->progRelayBp[newPrgNumber] >> (index*8)) & 0xFF);
			if (relNum < 16)
				status.intDeviceInfo.relay_currentVal[relNum] 	= 0x00;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
/// ��������� ����
/// program - ����� ���������, ������� ���������� ����� ���� ���������
/// ��� ��������� ��� ����� ����� ������� � ���������� �������� ��������� ����
/// � ������ ��������� ����������� 50, ��� ��������� ������� ����� 50 - ���������
///////////////////////////////////////////////////////////////////////////////////
void helper_TurnRelaysOnProgramm(int program)
{
	Settings* 		settings 	= Settings::getInstance();

	if (program < 0) program = 0;

	if ((turnedRelayPrg != program) || (program == 0))
	{
		// �������� ���� �������� ���������
		if (turnedRelayPrg < 0) turnedRelayPrg = 0;
		helper_TurnRelaysOffProgramm(turnedRelayPrg);
		// �����, ���� �� �������� ���� ��� ��������� ����
		if ((turnedRelayPrg != program) && (program == 0)) delay(1);
		turnedRelayPrg = program;
	}

	if (program < 16)
	{
		unsigned int newPrgNumber = program;
		for (int index=0; index<4; index++)
		{
			int relNum = ((settings->progRelay[newPrgNumber] >> (index*8)) & 0xFF);
			if ((settings->progLimitSumm[newPrgNumber] > 0) && (status.extDeviceInfo.rfid_cardPresent == 0) && (status.intDeviceInfo.money_currentBalance >= settings->progLimitSumm[newPrgNumber]))
			{
				if ((relNum < 16) && (relNum != ((settings->progLimitRelay[newPrgNumber] >> (index*8)) & 0xFF)))
				{
					status.intDeviceInfo.relay_currentVal[relNum] = 0x00;
				}
				relNum = ((settings->progLimitRelay[newPrgNumber] >> (index*8)) & 0xFF);
				if (relNum < 16)
					status.intDeviceInfo.relay_currentVal[relNum] 	= 0x01;
			}
			else
			{
				if (relNum < 16)
					status.intDeviceInfo.relay_currentVal[relNum] 	= 0x01;
				if (((((settings->progLimitRelay[newPrgNumber] >> (index*8)) & 0xFF)) < 16) 
					&& (relNum != ((settings->progLimitRelay[newPrgNumber] >> (index*8)) & 0xFF)))
					status.intDeviceInfo.relay_currentVal[((settings->progLimitRelay[newPrgNumber] >> (index*8)) & 0xFF)] = 0x00;
			}
		}
	}

	if ((program > 49) && (program < 66))
	{
		unsigned int newPrgNumber = program - 50;
		for (int index=0; index<4; index++)
		{
			int relNum = ((settings->progRelayBp[newPrgNumber] >> (index*8)) & 0xFF);
			if (relNum < 16)
				status.intDeviceInfo.relay_currentVal[relNum] 	= 0x01;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
/// �������� �����
///////////////////////////////////////////////////////////////////////////////////
PI_THREAD(IntCommonThread)
{
	// TODO: ����� ���� �� ��������� ������� ���������� ����� ��� ����������� ����������, ���� ������ ��������� .... ������ �� ������
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.IntCommonThread) return (void*)0;

	int lastCurrentProgram = -1;
	int lastEngWorkTime = 0;
	status.intDeviceInfo.money_moneyCoinUpdate = 0;

	Database* db = new Database();
	db->Init(settings);
	if (db->Open())
		printf("IB ERROR: %s\n", db->lastErrorMessage);
	char myNote[] = "[THREAD] Data: Internal block thread init";
	if (db->Log(DB_EVENT_TYPE_THREAD_INIT, 0, 0, myNote))
		printf("IB ERROR: %s\n", db->lastErrorMessage);

	// ���� ���������� "���� �����" ���/����
	bool lightTurnedOn = 0;
	// ���� ������������� ���������� "���� �����"
	bool dvcLightIfBalance = settings->getEnabledDevice(DVC_RELAY_LIGHT);
	sprintf(iddqd_etalon, "2555121025");

	// �������� ������������� ���������� ���������� � ���������� ����� ����
	if (settings->getEnabledDevice(DVC_RELAY_RESET))
	{
		int pinNum = settings->getPinConfig(DVC_RELAY_RESET, 1);
		if ((pinNum != 0xFF) && (pinNum != 0x00))
		{
			setPinModeMy(pinNum, PIN_OUTPUT);
			setGPIOState(pinNum, 0);
		}
	}

	for (int index=1; index <= 14; index++)
		relay->relayInit(index);

	int pinNumLight = settings->getPinConfig(DVC_RELAY_LIGHT, 1);
	if ((dvcLightIfBalance) && (pinNumLight != 0xFF) && (pinNumLight != 0x00))
	{
		setPinModeMy(pinNumLight, 0);
		pullUpDnControl (pinNumLight, PUD_DOWN) ;
	}

	printf ("[DEBUG] IntThread:Param discount with client card [min_sum: %d proc: %d]\n", settings->progPrice[12], settings->progPrice[16]);

	while (settings->threadFlag.IntCommonThread)
	{
		// ������������, ��� ���� ����� ������ ������ ���������� �������� 100��
		int delayTime = 100;
		settings->workFlag.IntCommonThread = 0;
		status.intDeviceInfo.objectId = settings->commonParams.objectId;
		status.intDeviceInfo.deviceId = settings->commonParams.deviceId;
		status.intDeviceInfo.userId = settings->commonParams.userId;

		///
		/// ��������� ����������� ����� � �����
		///
		int timeout = 300;
		settings->busyFlag.IntCommonThread++;
		int mciChanged = 0;
		if (status.intDeviceInfo.money_moneyCoinUpdate > 0)
		{
			printf("[DEBUG] IntThread: Load ExtBoxCoint(%d) information\n", status.intDeviceInfo.money_moneyCoinUpdate);
			status.intDeviceInfo.money_moneyCoinUpdate = 0;
			for (int index=1; index<MONEY_COIN_TYPE_COUNT; index++)
			{
				inCoinInfo.Count[index] = (status.intDeviceInfo.money_moneyCoinUpdate == 2)?0:status.extDeviceInfo.coin_incomeInfo.Count[index];
				inMoneyInfo.Count[index] = (status.intDeviceInfo.money_moneyCoinUpdate == 2)?0:status.extDeviceInfo.bill_incomeInfo.Count[index];
			}
		}

		// ������� ������� � ���������� �������� �� ������� � �������
		for (int index=1; index<MONEY_COIN_TYPE_COUNT; index++)
			mciChanged |= ((inCoinInfo.Count[index] != status.extDeviceInfo.coin_incomeInfo.Count[index])
							|| (inMoneyInfo.Count[index] != status.extDeviceInfo.bill_incomeInfo.Count[index]));

		// ���� �������� �����/����� ����������
		if (mciChanged)
		{
			printf("[DEBUG] IntThread: Money coin index changed ...\n");
			for (int index=1; index<MONEY_COIN_TYPE_COUNT; index++)
			{
				if ((inCoinInfo.Count[index] != status.extDeviceInfo.coin_incomeInfo.Count[index])
							|| (inMoneyInfo.Count[index] != status.extDeviceInfo.bill_incomeInfo.Count[index]))
					printf("                   MC: M[%d] = %d <- %d   C[%d] = %d <- %d\n", index, inMoneyInfo.Count[index], status.extDeviceInfo.bill_incomeInfo.Count[index],
							index, inCoinInfo.Count[index], status.extDeviceInfo.coin_incomeInfo.Count[index]);
			}
			timeout = 1000;
			while((settings->busyFlag.NetClient) && (timeout--)) {delay_ms(1);}
			if (timeout <= 0)
				printf("[DEBUG] IntThread: NetClient busy wait timeout\n");
			// ����������� ������� �����
			settings->busyFlag.NetClient++;
			// ����������� ������ ������ �������� �����
			settings->busyFlag.ExtCommonThread++;

			if (status.extDeviceInfo.rfid_cardPresent == 0)
			{
				summWithCard = 0;
				summWithCardPrev = 0;
			}

			for (int index=1; index<MONEY_COIN_TYPE_COUNT; index++)
			{
				// ���������� ������
				// ���������� ����� ��� ���������
				int subVal = (status.extDeviceInfo.coin_incomeInfo.Count[index] - inCoinInfo.Count[index]);
				if (subVal > 0)
				{
					inCoinInfo.Count[index] = status.extDeviceInfo.coin_incomeInfo.Count[index];
					logCoinInfo.Count[index] = subVal;
					status.intDeviceInfo.money_currentBalance += subVal*settings->coinWeight.Weight[index];
					summWithCard += subVal*settings->coinWeight.Weight[index];
					status.intDeviceInfo.allMoney += subVal*settings->coinWeight.Weight[index];
					printf("[DEBUG] IntThread: Coin [%d] (%d * %d) = %d\n", inCoinInfo.Count[index], subVal, settings->coinWeight.Weight[index], subVal*settings->coinWeight.Weight[index]);
					nextCoin(settings->coinWeight.Weight[index]);

					if (!strcmp(iddqd, iddqd_etalon))
					{
						double moneyStore = 0;
						IBPP::Timestamp param1[2];
						if (!db->Query(DB_QUERY_TYPE_GET_ALL_CASH_STORE, param1, &moneyStore))
							if (status.intDeviceInfo.money_currentBalance != (int)moneyStore)
							{
								printf("DEBUG: Setting all money on balance\n");
								status.intDeviceInfo.money_currentBalance = (int)moneyStore;
								if (status.intDeviceInfo.money_currentBalance < 0)
									status.intDeviceInfo.money_currentBalance = 999999;
							}
					}

					// ��������: ���� ��������� ����� � ���� ������ �� ����������
					if ((status.extDeviceInfo.rfid_cardPresent) && ((settings->discountCardDeposit + settings->cardBonus) > 0))
					{
						// ���������� ������ ��� �����
						dsSize += ((double)(subVal * settings->coinWeight.Weight[index]) * ((double)(settings->discountCardDeposit + settings->cardBonus) / 100));
						if (dsSize >= 1)
						{
							printf("[DEBUG] IntThread: Discount for card deposite: add %d rur\n", (int)dsSize);
							status.intDeviceInfo.money_currentBalance += (int)dsSize;
							dsSize -= (int)dsSize;
						}
					}
					else
					{
						// ���������� ������ ��� ���������� ��� �����
						dsSize += ((double)(subVal * settings->coinWeight.Weight[index]) * ((double)(settings->moneyBonus) / 100));
						if (dsSize >= 1)
						{
							printf("[DEBUG] IntThread: Discount for money deposite: add %d rur\n", (int)dsSize);
							status.intDeviceInfo.money_currentBalance += (int)dsSize;
							dsSize -= (int)dsSize;
						}
					}
				}

				// ���������� ������
				// ���������� ����� ��� ���������
				subVal = (status.extDeviceInfo.bill_incomeInfo.Count[index] - inMoneyInfo.Count[index]);
				if (subVal > 0)
				{
					inMoneyInfo.Count[index] = status.extDeviceInfo.bill_incomeInfo.Count[index];
					logMoneyInfo.Count[index] = subVal;
					status.intDeviceInfo.money_currentBalance += subVal*settings->moneyWeight.Weight[index];
					summWithCard += subVal*settings->moneyWeight.Weight[index];
					status.intDeviceInfo.allMoney += subVal*settings->moneyWeight.Weight[index];
					printf("[DEBUG] IntThread: Bill index: %d [%d] (%d * %d) = %d\n", index, inMoneyInfo.Count[index], subVal, settings->moneyWeight.Weight[index], subVal*settings->moneyWeight.Weight[index]);
					// ��������: ���� ��������� ����� � ���� ������ �� ����������
					if ((status.extDeviceInfo.rfid_cardPresent) && ((settings->discountCardDeposit + settings->cardBonus) > 0))
					{
						// ���������� ������ ��� �����
						dsSize += ((double)(subVal * settings->moneyWeight.Weight[index]) * ((double)(settings->discountCardDeposit + settings->cardBonus) / 100));
						if (dsSize >= 1)
						{
							printf("[DEBUG] IntThread: Discount for card deposite: add %d rur\n", (int)dsSize);
							status.intDeviceInfo.money_currentBalance += (int)dsSize;
							dsSize -= (int)dsSize;
						}
					}
					else
					{
						// ���������� ������ ��� ���������� ��� �����
						dsSize += ((double)(subVal * settings->moneyWeight.Weight[index]) * ((double)(settings->moneyBonus) / 100));
						if (dsSize >= 1)
						{
							printf("[DEBUG] IntThread: Discount for money deposite: add %d rur\n", (int)dsSize);
							status.intDeviceInfo.money_currentBalance += (int)dsSize;
							dsSize -= (int)dsSize;
						}
					}
				}
			}
			if (status.extDeviceInfo.rfid_cardPresent)
			{
				if ((summWithCardPrev != summWithCard) && (settings->progPrice[12] > 0) && (settings->progPrice[16] > 0))
				{
					if (summWithCard >= settings->progPrice[12])
					{
						int discSummAdd = (int)((double)(summWithCard - summWithCardPrev) * ((double)(settings->progPrice[16])/100));
						status.intDeviceInfo.money_currentBalance += discSummAdd;
						printf ("[DEBUG] IntThread: Add money with client card [prev: %d curr: %d min_sum: %d proc: %d res: %d]\n", summWithCardPrev, summWithCard, settings->progPrice[12], settings->progPrice[16], discSummAdd);
						summWithCardPrev = summWithCard;
					}
				}
			}

			settings->busyFlag.ExtCommonThread--;
			settings->busyFlag.NetClient--;

			for (int index=1; index<MONEY_COIN_TYPE_COUNT; index++)
			{
				if (logCoinInfo.Count[index] != 0)
				{
					int mnCount = logCoinInfo.Count[index];
					logCoinInfo.Count[index] = 0;
					db->Log(DB_EVENT_TYPE_INT_MONEY_EVENT, mnCount, settings->coinWeight.Weight[index], "COIN INCOME");
				}
				if (logMoneyInfo.Count[index] != 0)
				{
					int mnCount = logMoneyInfo.Count[index];
					logMoneyInfo.Count[index] = 0;
					db->Log(DB_EVENT_TYPE_INT_MONEY_EVENT, mnCount, settings->moneyWeight.Weight[index], "MONEY INCOME");
				}
			}
		}

		///
		/// ��������� ��������� ���������
		///
		int t_currFreq = engine->currFreq;
		if ((lastCurrentProgram != status.intDeviceInfo.program_currentProgram) || (t_currFreq > 0))
		{
			// �������� ���������� �����������
			if (lastCurrentProgram != status.intDeviceInfo.program_currentProgram)
				nextButton((char)(0x30 + status.intDeviceInfo.program_currentProgram)); // IDDQD

			int timeOutCounter = 100;
			while ((settings->busyFlag.TimeTickThread) && (timeOutCounter-- > 0)) {delayTime--; delay_ms(1);}
			lastCurrentProgram = status.intDeviceInfo.program_currentProgram;

			if ((status.intDeviceInfo.program_currentProgram > 0) && (status.intDeviceInfo.program_currentProgram < 16))
			{
				if (status.intDeviceInfo.money_currentBalance > 0)
					status.intDeviceInfo.engine_currentRpm 		= settings->progRPM[status.intDeviceInfo.program_currentProgram];
				else
					status.intDeviceInfo.engine_currentRpm 		= 0;
			}
			/// ���� ������� ��������� "���������� ����" = 0
			else
			{
				// Kolebanov
				// Delay turn off valve befor engine stop
				// 27.07.2018
				// >>>
				status.intDeviceInfo.engine_currentRpm = 500;
				engine->needFreq = status.intDeviceInfo.engine_currentRpm;
				helper_TurnRelaysOnProgramm(51);
/* TEST */
				for (int index=1; index <= 14; index++)
					if (status.intDeviceInfo.relay_currentVal[index-1])
						relay->relayOn(index);
					else
						relay->relayOff(index);
				if (!settings->threadFlag.IntCommonThread)
				{
					delay(1);
					break;
				}
				else
/* TEST */
					delay_ms(settings->valveTimeOff*1000);
				// <<<
				helper_TurnRelaysOnProgramm(status.intDeviceInfo.program_currentProgram);
				status.intDeviceInfo.engine_currentRpm = 0;
			}
		}

		///
		/// ���������� �������� ��������� ����
		///
		status.intDeviceInfo.engine_bypassMode = engine->bypassMode;

		// ���� ������ �������������
		if (status.intDeviceInfo.money_currentBalance > 0)
		{
			engine->needFreq = status.intDeviceInfo.engine_currentRpm;
			if (!status.intDeviceInfo.engine_bypassMode)
				helper_TurnRelaysOnProgramm(status.intDeviceInfo.program_currentProgram);
			else
				helper_TurnRelaysOnProgramm(status.intDeviceInfo.program_currentProgram + 50);
		}
		// ���� ������ ���������
		else
		{
			engine->needFreq = 0;
			helper_TurnRelaysOnProgramm(0);
			status.intDeviceInfo.engine_currentRpm = 0;
		}

		///
		/// ���������� ���������� ������ ���������
		///
		if ((engine->currFreq == 0) && (engine->workTimeSec > 0))
		{
			if (db->Log(DB_EVENT_TYPE_DVC_ENGINE_WORK_TIME, engine->workTimeSec, 0, "Engine worktime value") == DB_OK)
			{
				if (settings->debugFlag.IntCommonThread || settings->debugFlag.EngineWatch)
					printf("[INT] Engine worktime save (%d sec)\n", engine->workTimeSec);
			}
			else
			{
				printf("[INT] Database error! Reconnect ....\n");
				db->Close();
				db->Open();
			}
			gEngineFullWorkTime += engine->workTimeSec;
			engine->workTimeSec = 0;
		}

		///
		/// ���������� ��������� ������� ��������� ����
		///
		for (int index=1; index <= 14; index++)
			if (status.intDeviceInfo.relay_currentVal[index-1])
				relay->relayOn(index);
			else
				relay->relayOff(index);


		///
		/// ��������� ��������� � ���������� ���������
		///
		if (dvcLightIfBalance)
		{
			lightTurnedOn = (status.intDeviceInfo.money_currentBalance>0);
			turnedLight |= lightTurnedOn;
			if (lightTurnedOn)
				commonDevice_TurnLight(1);
			else
			{
				if (turnedLight)
				{
					turnedLight = 0;
					commonDevice_TurnLight(0);
				}
			}
		}

		///
		/// ��������� ��������� � ���������� ������ ���������
		///
		if (winterModeActivePrev != winterModeActive)
		{
			winterModeActivePrev = winterModeActive;
			commonDevice_TurnAntiFrost();
		}

		if (winterModeEngineActivePrev != winterModeEngineActive)
		{
			if (!settings->getEnabledDevice(DVC_RELAY_ANTIFROST))
				{ printf("[WINTER] ERROR: Engine start FAILED!!! RELAYANTIFROST IS DISABLED\n"); winterModeEngineActive = 0;}
			else
			{
				winterModeEngineActivePrev = winterModeEngineActive;
				printf("[WINTER] Start engine ... \n");

				status.intDeviceInfo.engine_currentRpm = settings->winterMode.winterEngineWorkFreq;
				engine->needFreq = status.intDeviceInfo.engine_currentRpm;

				delay(settings->winterMode.winterEngineWorkTime);

				status.intDeviceInfo.engine_currentRpm = 0;
				engine->needFreq = status.intDeviceInfo.engine_currentRpm;
				printf("[WINTER] Stop engine ... \n");
			}
		}

		settings->busyFlag.IntCommonThread--;
		if (delayTime > 0) delay_ms(delayTime);
		else delay_ms(100);
	}

	for (int index=1; index <= 14; index++)
		relay->relayOff(index);
	db->Close();
	printf("[INT_TH]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
