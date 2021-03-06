// DVC_BUTTON_OUT_COIN						41		//������ ������ �������
// DVC_SENSOR_COIN_OUT						42		//������ ��������� ������� � �������
// DVC_RELAY_OUT_COIN						43		//������ �� ������ �������

#include "../main.h"

int waitSumm = 0;
int paymentSumm = 0;

int blinkBtn(unsigned char mask)
{
	int l = 0;
	if (mask == 0x00) l++;
	for (int i=0; i < 8; i++)
	{
		if (!settings->getEnabledDevice(DVC_BUTTON01+i)) continue;
		int currentPin = settings->getPinConfig(DVC_BUTTON01+i, 1);
		if ((currentPin == 0) || (currentPin > 0x7F)) continue;
		if (((mask >> i) & 0x01) && (i < 8))
		{
			l++;
			setPinModeMy(currentPin, PIN_OUTPUT);
			setGPIOState(currentPin, 1);
		}
		else
		{
			setGPIOState(currentPin, 0);
			setPinModeMy(currentPin, PIN_INPUT);
  			pullUpDnControl (currentPin, PUD_DOWN) ;
		}
	}
	return l;
}

int sendBalanceToRemoteDevice(NetClient* netClient, int deviceNumber, int sendedBalance)
{
	int deviceNumberCalc = deviceNumber + 101;

	if (!CheckLink(settings->ethName)) { netClient->CloseConnection(); printf("[Net Client]: ERROR Interface %s: link down ...\n", settings->ethName); return 2;}
/*	netClient->Init(settings);
	char panelAddr[100] = {0};
	sprintf(panelAddr, "%s%d", settings->terminalParam.deviceSubnet, deviceNumberCalc);
	if (settings->debugFlag.ButtonTerminalThread)
		printf("NetClient: Remote server: %s\n", panelAddr);
	netClient->CloseConnection();
	strcpy(netClient->hostAddr, panelAddr);
	netClient->OpenConnection();
	if (netClient->isConnected)
		printf("NetClient: Remote server - connected [OK]\n");
	else
		return 1;
	if (netClient->cmdSendBalance(sendedBalance) == 0) { printf("[netClient]: Error cmdSendBalance()\n"); netClient->CloseConnection(); return 1; }
	netClient->CloseConnection();
*/

	char server_ip[100] = {0};
	sprintf(server_ip, "%s%d", settings->terminalParam.deviceSubnet, deviceNumberCalc);
	if (settings->debugFlag.ButtonTerminalThread)
		printf("NetClient: Remote server: %s\n", server_ip);

	int serverFd;
	struct sockaddr_in server;
	int len;
	int port = 3356;
	char buffer[100] = {0};
	sprintf(buffer, "add %d", sendedBalance);
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd < 0) 
	{
		perror("Cannot create socket");
		return 1;
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(server_ip);
	server.sin_port = htons(port);
	len = sizeof(server);
	if (connect(serverFd, (struct sockaddr *)&server, len) < 0) 
	{
		perror("Cannot connect to server");
		return 2;
	}
	if (write(serverFd, buffer, strlen(buffer)) < 0) 
	{
		perror("Cannot write");
		return 3;
	}

	char recv[1024];
	memset(recv, 0, sizeof(recv));
	if (read(serverFd, recv, sizeof(recv)) < 0) 
	{
		perror("cannot read");
		return 4;
	}
	printf("Received %s from server\n", recv);
	close(serverFd);

	return 0;
}

PI_THREAD(ButtonTerminalWatch)
{
	/// ����� ���������
	Settings* settings = Settings::getInstance();
	/// ���� ����� ��������, �� �����������
	if (!(settings->threadFlag.ButtonTerminalThread)) return (void*)0;
	Database* db = new Database();
	NetClient* 		netClient	= new NetClient();
	db->Init(settings);
	db->Log( 0, DB_EVENT_TYPE_THREAD_INIT, 		0, 0, "[THREAD] Button (terminal mode): Button thread init");
	db->Log( 0, DB_EVENT_TYPE_DVC_BUTTON_INIT, 	0, 0, "Button (terminal mode) panel device opened");

	if (settings->debugFlag.ButtonTerminalThread)
		printf("[DEBUG] ButtonTerminalThread: Debug information is showed\n");

	unsigned char blinkMask[] =
				{0b11111111, 0b01010101, 0b10101010, 0b01010101, 0b10101010, 0b00000000,
				 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000,
				 0b10000000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000001, 0b00000000 };

	int index;
	int currentPin;

	delay_ms(1000);

	int btnPins[12] = {0};

	for (int i=0; i<12; i++)
	{
		if (!settings->getEnabledDevice(DVC_BUTTON01+i)) continue;
		btnPins[i] = settings->getPinConfig(DVC_BUTTON01+i, 1);
		if ((btnPins[i] == 0) || (btnPins[i] > 0x7F)) continue;
	}

	for (int i=0; i < 12; i++)
	{
		int currentPin = btnPins[i];
		if ((currentPin == 0) || (currentPin > 0x7F)) continue;
		setGPIOState(btnPins[i], 0);
		setPinModeMy(btnPins[i], PIN_INPUT);
	}

	if (settings->getEnabledDevice(DVC_RELAY_OUT_RFID_CARD))
	{
		currentPin = settings->getPinConfig(DVC_RELAY_OUT_RFID_CARD, 1);
		if (currentPin != 0xFF)
		{
			setPinModeMy(currentPin, PIN_OUTPUT);
			setGPIOState(currentPin, 0);
  		}
	}

	if (settings->getEnabledDevice(DVC_SENSOR_EMPTY_RFID_CARD))
	{
		currentPin = settings->getPinConfig(DVC_SENSOR_EMPTY_RFID_CARD, 1);
		if (currentPin != 0xFF)
		{
			setGPIOState(currentPin, 0);
			setPinModeMy(currentPin, PIN_INPUT);
  			pullUpDnControl (currentPin, PUD_DOWN) ;
  		}
	}

	if (settings->getEnabledDevice(DVC_BUTTON_OUT_RFID_CARD))
	{
		currentPin = settings->getPinConfig(DVC_BUTTON_OUT_RFID_CARD, 1);
		if (currentPin != 0xFF)
		{
			setGPIOState(currentPin, 0);
			setPinModeMy(currentPin, PIN_INPUT);
  			pullUpDnControl (currentPin, PUD_DOWN) ;
  		}
	}

	if (settings->debugFlag.ButtonTerminalThread)
		printf("[DEBUG] ButtonTerminalThread: Button subsystem init  [%3d:%02d:%02d]\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);

	int thread_timeout = 100;
	int emptyRFIDCardSensor = 0;
	int emptyCoinSensor = 0;
	int emptyLightCounter = 0;
	int payRFIDCardCounter = 5;
	int trId = 0;
	int firstBtnInit = 0;
	int counterBtnLight = 0;
	int maskIndex = 0;
	while (settings->threadFlag.ButtonTerminalThread)
	{
		if (thread_timeout > 0)
			delay_ms(thread_timeout);
		thread_timeout = 100;

		///
		/// ��������� ������ ������������ �� �����
		/// � �������� ������� ��������� ��������
		///

		if (status.intDeviceInfo.money_currentBalance == 0)
		{
			if (counterBtnLight++ > 3)
			{
				if (maskIndex >= sizeof(blinkMask)) maskIndex = 0;
				firstBtnInit = 0;
				counterBtnLight = 0;
				if (blinkBtn(blinkMask[maskIndex++]) == 0) counterBtnLight = 5;
			}
		}
		else
		{
			if (!firstBtnInit)
			{
				for (int t = 0; t < 12; t++)
				{
					if (!settings->getEnabledDevice(DVC_BUTTON01+t)) continue;
					int currentPin = settings->getPinConfig(DVC_BUTTON01+t, 1);
					if ((currentPin == 0) || (currentPin > 0x7F)) continue;
					setGPIOState(currentPin, 0);
					setPinModeMy(currentPin, PIN_INPUT);
  					pullUpDnControl (currentPin, PUD_DOWN) ;
  				}
			}
			maskIndex = 0;
			firstBtnInit = 1;
			counterBtnLight = 0;
			//------------------

			for (int i = 0; i < 12; i++)
			{
				if (!settings->getEnabledDevice(DVC_BUTTON01+i)) continue;
				int currentPin = settings->getPinConfig(DVC_BUTTON01+i, 1);
				if ((currentPin == 0) || (currentPin > 0x7F)) continue;
				int timeout = 30;
				while(getGPIOState(currentPin) && (timeout-- > 0)) { delay_ms(1); }
				if (timeout <= 0)
				{
					if (settings->debugFlag.ButtonTerminalThread)
						printf("[DEBUG] ButtonTerminalThread: Pressed state on %d button [PIN: %03d]\n", i, currentPin);
					setGPIOState(currentPin, 1);
					setPinModeMy(currentPin, PIN_OUTPUT);
					setGPIOState(currentPin, 1);
					if (settings->debugFlag.ButtonTerminalThread)
						printf("[DEBUG] ButtonTerminalThread: Sending balance %d to device %d\n", status.intDeviceInfo.money_currentBalance, i);

					int sendedBalance = status.intDeviceInfo.money_currentBalance;
					if (sendBalanceToRemoteDevice(netClient, i, sendedBalance) == 0)
					{
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Send OK\n");
						status.intDeviceInfo.money_currentBalance -= sendedBalance;
					}
					else
					{
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Send ERROR\n");
					}

					delay_ms(2000);

					setGPIOState(currentPin, 0);
					setPinModeMy(currentPin, PIN_INPUT);
				}
			}
		}

		///
		/// ���� ���������� ������ ���� ������� � ��������� ����
		/// ��� ��������� ������� ������ ����� (������)
		/// ��� ���� ����������� ������
		/// ���� ������ ������� � ���������� ����
		/// ********************
		/// progPrice[12] - ����������� ����� ��� ������� �����
		/// progPrice[13] - ��������� ����� ��� ���������� ������ ��� progPrice[12] (����������� ����� ��� ������� �����)
		/// progPrice[14] - ��������� �����
		/// progPrice[15] - ��������� ������
		/// progPrice[16] - ������� ������ ��� ���������� ����� ����� ��� progPrice[12]
		///


		if (
			(status.intDeviceInfo.money_currentBalance > 0) &&
			// ������� �������� ��������� ������� �����
			(payRFIDCardCounter <= 0) &&
			// ��� �������� ����� � ������� ������ ������ ��������� �����
			((status.extDeviceInfo.rfid_cardPresent == 0) && (settings->progPrice[14] <= status.intDeviceInfo.money_currentBalance)) &&
			// ������� ������ ������ �����
			((settings->getEnabledDevice(DVC_BUTTON_OUT_RFID_CARD)) && (settings->getPinConfig(DVC_BUTTON_OUT_RFID_CARD, 1) != 0xFF) && (settings->getPinConfig(DVC_BUTTON_OUT_RFID_CARD, 1) != 0x00)) &&
			// ������� ���� ������ �����
			((settings->getEnabledDevice(DVC_RELAY_OUT_RFID_CARD)) && (settings->getPinConfig(DVC_RELAY_OUT_RFID_CARD, 1) != 0xFF) && (settings->getPinConfig(DVC_RELAY_OUT_RFID_CARD, 1) != 0x00)) &&
			// ��������� ����� ������ 0
			(settings->progPrice[14] > 0)
			)
		{
			// ���� ��������� ������ ������� ������� ���� �������
			if ((settings->getEnabledDevice(DVC_SENSOR_EMPTY_RFID_CARD)) && (settings->getPinConfig(DVC_SENSOR_EMPTY_RFID_CARD, 1) != 0xFF) && (settings->getPinConfig(DVC_SENSOR_EMPTY_RFID_CARD, 1) != 0x00))
			{
				currentPin = settings->getPinConfig(DVC_SENSOR_EMPTY_RFID_CARD, 1);
				setPinModeMy(currentPin, PIN_INPUT);
				delay_ms(1);
				int timeout = 30;
				while((timeout-- > 0) && getGPIOState(currentPin)) { delay_ms(1); }
				if (timeout > 0)
				{
					if ((emptyRFIDCardSensor == 0) && (settings->debugFlag.ButtonTerminalThread))
						printf("[DEBUG] ButtonTerminalThread: [%3d:%02d:%02d] - RFID card dispenser EMPTY !!!\n", ((long)(get_prguptime()/3600)), ((long)(get_prguptime()/60))%60, get_prguptime()%60);
					emptyRFIDCardSensor = 1;
				}
				else
				{
					emptyRFIDCardSensor = 0;
				}
			}
			// ���� ����� ������� ��� ������
			if (emptyRFIDCardSensor == 0)
			{
				emptyLightCounter = 0;
			}

			if ((emptyRFIDCardSensor == 0) && (getGPIOState(currentPin)))
			{
				unsigned int btnError = 0;
				int timeout = 50;
				if (settings->debugFlag.ButtonTerminalThread)
					printf("[DEBUG] ButtonTerminalThread: Pressed state on PAY CARD button [PIN: %03d]\n", currentPin);
				db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, currentPin, "[ButtonTerminalThread]: PAY CARD button pressed");
				//
				currentPin = settings->getPinConfig(DVC_BUTTON_OUT_RFID_CARD, 1);
				setPinModeMy(currentPin, PIN_INPUT);
				delay_ms(1);
				// Press minimal 50ms
				while ((timeout-- > 0) && getGPIOState(currentPin)) { delay_ms(1); }
				// If btn pressed gream 2 sec - ERROR
				while ((btnError++ < 2000) && getGPIOState(currentPin)) { delay_ms(1); }

				if (btnError < 1990)
				{
					if (timeout <= 0)
					{
						int bkpBal = status.intDeviceInfo.money_currentBalance;
						// ������� ������ ������ ��� ����������� ��������� �����
						// ���� ����� ������� �� progPrice[13]
						if ((settings->progPrice[12] > 0) && (status.intDeviceInfo.money_currentBalance >= settings->progPrice[12]))
						{
							status.intDeviceInfo.money_currentBalance -= settings->progPrice[13];
						}
						// ������� ������ ������ ��� ����������� ��������� �����
						// ���� ����� ������� �� progPrice[14]
						else
							status.intDeviceInfo.money_currentBalance -= settings->progPrice[14];

						if (status.intDeviceInfo.money_currentBalance < 0)
						{
							if (settings->debugFlag.ButtonTerminalThread)
								printf("[DEBUG] ButtonTerminalThread: Restoring ballance ... UNKNOWN ERROR\n");
							status.intDeviceInfo.money_currentBalance = bkpBal;
							continue;
						}

						thread_timeout -= 50;
						currentPin = settings->getPinConfig(DVC_RELAY_OUT_RFID_CARD, 1);
						setPinModeMy(currentPin, PIN_OUTPUT);
						delay_ms(5);
						setGPIOState(currentPin, 1);
						delay_ms(50);
						setGPIOState(currentPin, 0);
						thread_timeout -= 50;
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Out RFID card\n");
						db->Log( 0, DB_EVENT_TYPE_CARD_OUT, index, status.intDeviceInfo.money_currentBalance, "[ButtonTerminalThread]: Out RFID card");
						payRFIDCardCounter = 50;
					}
					else
					{
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Pressed state on PAY CARD button [PIN: %03d] - failed [%d ms]\n", currentPin, 50 - timeout);
						db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, 50 - timeout, "[ButtonTerminalThread]: Button PAY CARD don't detected. Failed");
						thread_timeout -= (50 - timeout);
					}
				}
			}
		}

		// ������� ������� ���� ��� ����
		if ((settings->getEnabledDevice(DVC_BUTTON_OUT_RFID_CARD)) && (emptyRFIDCardSensor == 1))
		{
			currentPin = settings->getPinConfig(DVC_BUTTON_OUT_RFID_CARD, 1);
			setPinModeMy(currentPin, PIN_OUTPUT);
			setGPIOState( currentPin, (int)(((int)(emptyLightCounter++/10))%2 == 1));
		}

		// ���� 50 ����� �� ������� ��� ����� �����
		if (payRFIDCardCounter > 0) payRFIDCardCounter--;
		if (status.intDeviceInfo.money_currentBalance <= 0) payRFIDCardCounter = 0;

		///
		/// ���� ���������� ������ ������� ������� � ��������� ����
		/// ��� ��������� ������� ������ ������� (������)
		/// ��� ���� ����������� ������
		/// ���� ������ ������� � �������� ������
		/// *************************
		/// progPrice[15] - ��������� 1 ������
		///
		if (
			(status.intDeviceInfo.money_currentBalance > 0) &&
			// ������� ����� ������ ��������� ������
			(settings->progPrice[15] <= status.intDeviceInfo.money_currentBalance) &&
			// ������ ������ ������ ����������
			((settings->getEnabledDevice(DVC_BUTTON_OUT_COIN)) && (settings->getPinConfig(DVC_BUTTON_OUT_COIN, 1) != 0xFF) && (settings->getPinConfig(DVC_BUTTON_OUT_COIN, 1) != 0x00)) &&
			// �������� ������ ������ �������
			((settings->getEnabledDevice(DVC_RELAY_OUT_COIN)) && (settings->getPinConfig(DVC_RELAY_OUT_COIN, 1) != 0xFF) && (settings->getPinConfig(DVC_RELAY_OUT_COIN, 1) != 0x00)) &&
			// ��������� ������ �����������
			(settings->progPrice[15] > 0)
			)
		{
			currentPin = settings->getPinConfig(DVC_BUTTON_OUT_COIN, 1);
			setPinModeMy(currentPin, PIN_INPUT);
			if (getGPIOState(currentPin))
			{
				unsigned int btnError = 0;
				int timeout = 30;
				if (settings->debugFlag.ButtonTerminalThread)
					printf("[DEBUG] ButtonTerminalThread: Pressed state on PAY COIN button [PIN: %03d]\n", currentPin);
				db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, currentPin, "[ButtonTerminalThread]: PAY COIN button pressed");
				// Press minimal 50ms
				while ((timeout-- > 0) && getGPIOState(currentPin)) { delay_ms(1); }
				// If btn pressed great 2 sec - ERROR
				while ((btnError++ < 2000) && getGPIOState(currentPin)) { delay_ms(1); }
				if ((btnError < 1990) && (timeout <= 0))
				{
					thread_timeout -= 50;
					currentPin = settings->getPinConfig(DVC_RELAY_OUT_COIN, 1);
					setPinModeMy(currentPin, PIN_OUTPUT);
					int discPrice = settings->progPrice[15];
					if ((status.intDeviceInfo.money_currentBalance >= settings->coinDiscount.coinAfterSumm) && (settings->coinDiscount.coinAfterSumm > 0))
					{
						discPrice = settings->progPrice[15] - (int)(((double)settings->progPrice[15]) * (((double)settings->coinDiscount.coinDiscountSize)/100));
					}
					int bonusCoinCount = (int)(((double)status.intDeviceInfo.money_currentBalance/(double)discPrice) - ((double)status.intDeviceInfo.money_currentBalance/(double)settings->progPrice[15]));
					if ((bonusCoinCount > 0) && (settings->debugFlag.ButtonTerminalThread))
						printf("[DEBUG] ButtonTerminalThread: BONUS COIN %d\n", bonusCoinCount);
					// ��������� ������ �������
					int outCoinSensorPin = settings->getPinConfig(DVC_SENSOR_COIN_OUT, 1);
					if (settings->getEnabledDevice(DVC_SENSOR_COIN_OUT))
						setPinModeMy(outCoinSensorPin, PIN_INPUT);
					int errorCoinCount = 0;
					while ((status.intDeviceInfo.money_currentBalance >= settings->progPrice[15]) && (errorCoinCount < 3))
					{
						//	������ �������
						setGPIOState(currentPin, 1);
						if (settings->getEnabledDevice(DVC_SENSOR_COIN_OUT))
						{
							delay_ms(100);
							setGPIOState(currentPin, 0);
							int ctrlTime = 0;
							int ctrlTimeout = 5000;
							while (ctrlTimeout-- > 0)
							{
								while (getGPIOState(outCoinSensorPin) == 0)
									ctrlTime++;
								if ((ctrlTime > 25) && (getGPIOState(outCoinSensorPin))) break;
								delay_ms(1);
							}
							//
							if ((ctrlTime > 25) && (ctrlTimeout > 0))
							{
								status.intDeviceInfo.money_currentBalance -= settings->progPrice[15];
								if (settings->debugFlag.ButtonTerminalThread)
									printf("[DEBUG] ButtonTerminalThread: Out COIN [bal: %d rur]\n", status.intDeviceInfo.money_currentBalance);
								db->Log( 0, DB_EVENT_TYPE_COIN_OUT, discPrice, status.intDeviceInfo.money_currentBalance, "[ButtonTerminalThread]: Out COIN [price, balance]");
							}
							else
							{
								errorCoinCount++;
								printf ("Out coin error: elapsed ctrlTimeout: %d count ctrlTime: %d errCount: \n", 5000-ctrlTimeout, ctrlTime, errorCoinCount);
							}
						}
						else
						{
							delay_ms(100);
							status.intDeviceInfo.money_currentBalance -= settings->progPrice[15];
							setGPIOState(currentPin, 0);
							delay_ms(1000);
							break;
						}
						setGPIOState(currentPin, 0);
					}
					while ((bonusCoinCount > 0)  && (errorCoinCount < 3))
					{

						//	������ �������
						setGPIOState(currentPin, 1);
						if (settings->getEnabledDevice(DVC_SENSOR_COIN_OUT))
						{
							delay_ms(100);
							setGPIOState(currentPin, 0);
							int ctrlTime = 0;
							int ctrlTimeout = 5000;
							while (ctrlTimeout-- > 0)
							{
								while (getGPIOState(outCoinSensorPin) == 0)
									ctrlTime++;
								if ((ctrlTime > 25) && (getGPIOState(outCoinSensorPin))) break;
								delay_ms(1);
							}
							//
							if ((ctrlTime > 25) && (ctrlTimeout > 0))
							{
								if (settings->debugFlag.ButtonTerminalThread)
									printf("[DEBUG] ButtonTerminalThread: Out BONUS COIN [bal: %d rur]\n", status.intDeviceInfo.money_currentBalance);
								db->Log( 0, DB_EVENT_TYPE_COIN_OUT, discPrice, status.intDeviceInfo.money_currentBalance, "[ButtonTerminalThread]: Out COIN [price, balance]");
							}
							else
							{
								errorCoinCount++;
								printf ("Out coin error: elapsed ctrlTimeout: %d count ctrlTime: %d errCount: \n", 5000-ctrlTimeout, ctrlTime, errorCoinCount);
							}
						}
						else
						{
							delay_ms(100);
						}
						setGPIOState(currentPin, 0);
					// ������������� ������ �������
					}
				}
				// ������ ������ ����� 30 ���� ��� ����� 2000 ����
				else
				{
					if (settings->debugFlag.ButtonTerminalThread)
						printf("[DEBUG] ButtonTerminalThread: Pressed state on %d button [PIN: %03d] - failed [%d ms]\n", index, currentPin, 50 - timeout);
					db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, 50 - timeout, "[ButtonTerminalThread]: Button don't detected. Failed");
					thread_timeout -= (50 - timeout);
				}
			}
		}

		///
		/// ��������� ����������
		/// ���� ���������� ���������� �������
		/// � ������� ������ ������
		///
		if (settings->threadFlag.VisaDeviceThread)
		{
			if (settings->visaParam.workMode == 0)
			{
				// ����� ������������� ������
				int pinNum = settings->visaParam.cancelBtn.pinNum;
				if (pinNum > 0)
				{
					setPinModeMy(pinNum, PIN_INPUT);
					if (getGPIOState(pinNum))
					{
						int timeout = 30;
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Pressed state on VISA:CANCEL button [PIN: %03d]\n", pinNum);
						// Press minimal 50ms
						while ((timeout-- > 0) && getGPIOState(pinNum)) { delay_ms(1); }
						db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, pinNum, "[ButtonTerminalThread]: VISA:CANCEL button pressed");
						if ((timeout <= 0))
						{
							setPinModeMy(pinNum, PIN_OUTPUT);
							setGPIOState(pinNum, 1);
							thread_timeout -= 50;
							char strTmp256[256];
							int proc_id = proc_find("./bwpay");
							if (proc_id != -1)
							{
								sprintf(strTmp256, "sudo kill -s SIGUSR1 %d", proc_id);
								system(strTmp256);
							}
							delay_ms(2000);
							proc_id = proc_find("./bwpay");
							if (proc_id != -1)
							{
								sprintf(strTmp256, "sudo kill %d", proc_id);
								system(strTmp256);
							}
							payInfo.result = PAY_RESULT_ERROR;
							payInfo.inUse = 0;
							setGPIOState(pinNum, 0);
							setPinModeMy(pinNum, PIN_INPUT);
						}
					}
				}
				pinNum = settings->visaParam.pay50Btn.pinNum;
				if (pinNum > 0)
				{
					setPinModeMy(pinNum, PIN_INPUT);
					if (getGPIOState(pinNum))
					{
						int timeout = 30;
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Pressed state on VISA:PAY_50 button [PIN: %03d]\n", pinNum);
						// Press minimal 50ms
						while ((timeout-- > 0) && getGPIOState(pinNum)) { delay_ms(1); }
						if ((timeout <= 0))
						{
							setPinModeMy(pinNum, PIN_OUTPUT);
							setGPIOState(pinNum, 1);
							db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, pinNum, "[ButtonTerminalThread]: VISA:PAY_50 button pressed");
							thread_timeout -= 50;
							paymentSumm = 50;
						}
					}
				}
				pinNum = settings->visaParam.pay100Btn.pinNum;
				if (pinNum > 0)
				{
					setPinModeMy(pinNum, PIN_INPUT);
					if (getGPIOState(pinNum))
					{
						int timeout = 30;
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Pressed state on VISA:PAY_100 button [PIN: %03d]\n", pinNum);
						// Press minimal 50ms
						while ((timeout-- > 0) && getGPIOState(pinNum)) { delay_ms(1); }
						if ((timeout <= 0))
						{
							setPinModeMy(pinNum, PIN_OUTPUT);
							setGPIOState(pinNum, 1);
							db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, pinNum, "[ButtonTerminalThread]: VISA:PAY_100 button pressed");
							thread_timeout -= 50;
							paymentSumm = 100;
						}
					}
				}
				pinNum = settings->visaParam.pay150Btn.pinNum;
				if (pinNum > 0)
				{
					setPinModeMy(pinNum, PIN_INPUT);
					if (getGPIOState(pinNum))
					{
						int timeout = 30;
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Pressed state on VISA:PAY_150 button [PIN: %03d]\n", pinNum);
						// Press minimal 50ms
						while ((timeout-- > 0) && getGPIOState(pinNum)) { delay_ms(1); }
						if ((timeout <= 0))
						{
							setPinModeMy(pinNum, PIN_OUTPUT);
							setGPIOState(pinNum, 1);
							db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, pinNum, "[ButtonTerminalThread]: VISA:PAY_150 button pressed");
							thread_timeout -= 50;
							paymentSumm = 150;
						}
					}
				}
				pinNum = settings->visaParam.pay200Btn.pinNum;
				if (pinNum > 0)
				{
					setPinModeMy(pinNum, PIN_INPUT);
					if (getGPIOState(pinNum))
					{
						int timeout = 30;
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Pressed state on VISA:PAY_200 button [PIN: %03d]\n", pinNum);
						// Press minimal 50ms
						while ((timeout-- > 0) && getGPIOState(pinNum)) { delay_ms(1); }
						if ((timeout <= 0))
						{
							setPinModeMy(pinNum, PIN_OUTPUT);
							setGPIOState(pinNum, 1);
							db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, pinNum, "[ButtonTerminalThread]: VISA:PAY_200 button pressed");
							thread_timeout -= 50;
							paymentSumm = 200;
						}
					}
				}
				pinNum = settings->visaParam.pay500Btn.pinNum;
				if (pinNum > 0)
				{
					setPinModeMy(pinNum, PIN_INPUT);
					if (getGPIOState(pinNum))
					{
						int timeout = 30;
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Pressed state on VISA:PAY_500 button [PIN: %03d]\n", pinNum);
						// Press minimal 50ms
						while ((timeout-- > 0) && getGPIOState(pinNum)) { delay_ms(1); }
						if ((timeout <= 0))
						{
							setPinModeMy(pinNum, PIN_OUTPUT);
							setGPIOState(pinNum, 1);
							db->Log( 0, DB_EVENT_TYPE_EXT_NEW_BUTTON, index, pinNum, "[ButtonTerminalThread]: VISA:PAY_500 button pressed");
							thread_timeout -= 50;
							paymentSumm = 500;
						}
					}
				}
				if ((waitSumm == 0) && (paymentSumm > 0))
				{
					printf("Kill payment operation\n");
					if (payInfo.inUse == 1)
					{
						char strTmp256[256];
						int proc_id = proc_find("./bwpay");
						if (proc_id != -1)
						{
							sprintf(strTmp256, "sudo kill -s SIGUSR1 %d", proc_id);
							system(strTmp256);
						}
						delay_ms(1000);
						proc_id = proc_find("./bwpay");
						if (proc_id != -1)
						{
							sprintf(strTmp256, "sudo kill %d -9", proc_id);
							system(strTmp256);
						}
						payInfo.result = PAY_RESULT_ERROR;
						payInfo.inUse = 0;
						delay_ms(2000);
					}
					waitSumm = paymentSumm;
					payInfo.summ = paymentSumm;
					payInfo.deviceNum = trId++;
					paymentSumm = 0;
					cp2utf("������ ������ �� ������ ���������", payInfo.note);
					sprintf(payInfo.r_phone, "");
					sprintf(payInfo.r_email, "test@test.email");
					payInfo.inUse = 1;
					delay_ms(5000);
					pinNum = settings->visaParam.pay50Btn.pinNum;
					if (pinNum > 0)
					{
						setGPIOState(pinNum, 0);
						setPinModeMy(pinNum, PIN_INPUT);
					}
					pinNum = settings->visaParam.pay100Btn.pinNum;
					if (pinNum > 0)
					{
						setGPIOState(pinNum, 0);
						setPinModeMy(pinNum, PIN_INPUT);
					}
					pinNum = settings->visaParam.pay150Btn.pinNum;
					if (pinNum > 0)
					{
						setGPIOState(pinNum, 0);
						setPinModeMy(pinNum, PIN_INPUT);
					}
					pinNum = settings->visaParam.pay200Btn.pinNum;
					if (pinNum > 0)
					{
						setGPIOState(pinNum, 0);
						setPinModeMy(pinNum, PIN_INPUT);
					}
					pinNum = settings->visaParam.pay500Btn.pinNum;
					if (pinNum > 0)
					{
						setGPIOState(pinNum, 0);
						setPinModeMy(pinNum, PIN_INPUT);
					}
				}
				if ((payInfo.inUse == 0) && (waitSumm > 0))
				{
					printf("[Terminal] waitSumm = %d Operation result = %d\n", waitSumm, payInfo.result);
					//DB_EVENT_TYPE_VISA_PAY_DOC_OK
					if (payInfo.result == PAY_RESULT_OK)
					{
						status.intDeviceInfo.money_currentBalance += waitSumm;
						char strTmp256[1024];
						int devId = settings->commonParams.deviceId;
						if (devId > 100) devId -= 100;
						sprintf(strTmp256, "%s (�:%d)", settings->kkmParam.ServiceName, devId);
						queueKkm->QueuePut(0, 0, waitSumm, 1, strTmp256);
						//!!! NEED print in DB payInfo.transactionId;
						db->Log( 0, DB_EVENT_TYPE_VISA_PAY_DOC_OK, waitSumm, payInfo.result, payInfo.note);
					}
					else
					{
						db->Log( 0, DB_EVENT_TYPE_VISA_PAY_DOC_ERROR, waitSumm, payInfo.result, payInfo.note);
					}
					waitSumm = 0;
				}
			}
			if (settings->visaParam.workMode == 1)
			{
				// ����� ������ ������
			}
		}

		if (thread_timeout < 0) thread_timeout = 0;
	}

	db->Log( 0, DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "Button terminal panel device is closed");
	db->Close();
	printf("[DEBUG]: ButtonTerminalThread: Thread is terminate.\n");
	return (void*)0;
}
