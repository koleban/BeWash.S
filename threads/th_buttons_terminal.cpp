#include "../main.h"

PI_THREAD(ButtonTerminalWatch)
{
	/// Общие параметры
	Settings* settings = Settings::getInstance();
	/// Если ПОТОК запрещен, то завершаемся
	if (!(settings->threadFlag.ButtonTerminalThread)) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	db->Log(DB_EVENT_TYPE_THREAD_INIT, 		0, 0, "[THREAD] Button (terminal mode): Button thread init");
	db->Log(DB_EVENT_TYPE_DVC_BUTTON_INIT, 	0, 0, "Button (terminal mode) panel device opened");

	if (settings->debugFlag.ButtonTerminalThread)
		printf("[DEBUG] ButtonTerminalThread: Debug information is showed\n");

	int index;
	int currentPin;

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
	int payRFIDCardCounter = 1;
	while (settings->threadFlag.ButtonTerminalThread)
	{
		if (thread_timeout > 0)
			delay_ms(thread_timeout);
		thread_timeout = 100;

		///
		/// Если Устройство ВЫДАЧИ КАРТ активно и разрешены пины
		/// ПИН ВХОДЯЩЕГО СИГНАЛА ВЫДАЧИ КАРТЫ (КНОПКА)
		/// ПИН РЕЛЕ УПРАВЛЕНИЕМ ВЫДАЧИ
		/// Есть анализ СИГНАЛА О ОТСУТСТВИИ КАРТ
		/// ********************
		/// progPrice[12] - Минимальная сумма для покупки карты
		/// progPrice[13] - Стоимость КАРТЫ при пополнении больше чем progPrice[12] (Минимальная сумма для покупки карты)
		/// progPrice[14] - Стоимость КАРТЫ
		/// progPrice[15] - Стоимость ЖЕТОНА
		/// progPrice[16] - Процент бонуса при пополнении карты более чем progPrice[12]
		///


		if (
			// Счетчик ожидания повторной покупки карты
			(payRFIDCardCounter <= 0) &&
			// Нет активной карты и Текущий баланс больше стоиомсти карты
			((status.extDeviceInfo.rfid_cardPresent == 0) && (settings->progPrice[14] <= status.intDeviceInfo.money_currentBalance)) &&
			// Активна кнопка выдачи карты
			((settings->getEnabledDevice(DVC_BUTTON_OUT_RFID_CARD)) && (settings->getPinConfig(DVC_BUTTON_OUT_RFID_CARD, 1) != 0xFF)) &&
			// Активно реле выдачи карты
			((settings->getEnabledDevice(DVC_RELAY_OUT_RFID_CARD)) && (settings->getPinConfig(DVC_RELAY_OUT_RFID_CARD, 1) != 0xFF) ) &&
			// Стоиомсть карты больше 0
			(settings->progPrice[14] > 0)
			)
		{
			if ((settings->getEnabledDevice(DVC_SENSOR_EMPTY_RFID_CARD)) && (settings->getPinConfig(DVC_SENSOR_EMPTY_RFID_CARD, 1) != 0xFF))
			{
				currentPin = settings->getPinConfig(DVC_SENSOR_EMPTY_RFID_CARD, 1);
				setPinModeMy(currentPin, PIN_INPUT);
				delay_ms(5);
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
			currentPin = settings->getPinConfig(DVC_BUTTON_OUT_RFID_CARD, 1);
			setPinModeMy(currentPin, PIN_INPUT);
			if ((emptyRFIDCardSensor == 0) && (getGPIOState(currentPin)))
			{
				int btnError = 0;
				int timeout = 50;
				if (settings->debugFlag.ButtonTerminalThread)
					printf("[DEBUG] ButtonTerminalThread: Pressed state on PAY CARD button [PIN: %03d]\n", currentPin);
				db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, index, currentPin, "[ButtonTerminalThread]: PAY CARD button pressed");
				// Press minimal 50ms
				while ((timeout-- > 0) && getGPIOState(currentPin)) { delay_ms(1); }
				// If btn pressed gream 2 sec - ERROR
				while ((btnError++ < 2000) && getGPIOState(currentPin)) { delay_ms(1); }
				if (btnError < 1990)
				{
					if (timeout <= 0)
					{
						int bkpBal = status.intDeviceInfo.money_currentBalance;
						// Текущий баланс БОЛЬШЕ чем минимальная стоиомсть карты
						// ЦЕНА карты берется из progPrice[13]
						if (status.intDeviceInfo.money_currentBalance >= settings->progPrice[12])
							status.intDeviceInfo.money_currentBalance -= settings->progPrice[13];

						// Текущий баланс МЕНЬШЕ чем минимальная стоиомсть карты
						// ЦЕНА карты берется из progPrice[14]
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
						db->Log(DB_EVENT_TYPE_CARD_OUT, index, status.intDeviceInfo.money_currentBalance, "[ButtonTerminalThread]: Out RFID card");
						payRFIDCardCounter = 20;
					}
					else
					{
						if (settings->debugFlag.ButtonTerminalThread)
							printf("[DEBUG] ButtonTerminalThread: Pressed state on PAY CARD button [PIN: %03d] - failed [%d ms]\n", currentPin, 50 - timeout);
						db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, index, 50 - timeout, "[ButtonTerminalThread]: Button PAY CARD don't detected. Failed");
						thread_timeout -= (50 - timeout);
					}
				}
			}
		}

		// Ждем 20 тиков до покупки еще одной карты
		if (payRFIDCardCounter > 0) payRFIDCardCounter--;
		if (status.intDeviceInfo.money_currentBalance <= 0) payRFIDCardCounter = 0;

		///
		/// Если Устройство ВЫДАЧИ ЖЕТОНОВ активно и разрешены пины
		/// ПИН ВХОДЯЩЕГО СИГНАЛА ВЫДАЧИ ЖЕТОНОВ (КНОПКА)
		/// ПИН РЕЛЕ УПРАВЛЕНИЕМ ВЫДАЧИ
		/// Есть анализ СИГНАЛА О ПРОЦЕССЕ ВЫДАЧИ
		/// *************************
		/// progPrice[15] - Стоимость 1 жетона
		///
		if (
			// Текущий балан больше стоимости жетона 
			(settings->progPrice[15] <= status.intDeviceInfo.money_currentBalance) &&
			// Кнопка выдачи жетона определена
			((settings->getEnabledDevice(DVC_BUTTON_OUT_COIN)) && (settings->getPinConfig(DVC_BUTTON_OUT_COIN, 1) != 0xFF)) &&
			// Контроль выдачи жетона активен
			((settings->getEnabledDevice(DVC_RELAY_OUT_COIN)) && (settings->getPinConfig(DVC_RELAY_OUT_COIN, 1) != 0xFF) )
			)
		{
			currentPin = settings->getPinConfig(DVC_BUTTON_OUT_COIN, 1);
			setPinModeMy(currentPin, PIN_INPUT);
			if (getGPIOState(currentPin))
			{
				int btnError = 0;
				int timeout = 30;
				if (settings->debugFlag.ButtonTerminalThread)
					printf("[DEBUG] ButtonTerminalThread: Pressed state on PAY COIN button [PIN: %03d]\n", currentPin);
				db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, index, currentPin, "[ButtonTerminalThread]: PAY COIN button pressed");
				// Press minimal 50ms
				while ((timeout-- > 0) && getGPIOState(currentPin)) { delay_ms(1); }
				// If btn pressed gream 2 sec - ERROR
				while ((btnError++ < 2000) && getGPIOState(currentPin)) { delay_ms(1); }
				if ((btnError < 1990) && (timeout <= 0))
				{
					thread_timeout -= 50;
					currentPin = settings->getPinConfig(DVC_RELAY_OUT_COIN, 1);
					setPinModeMy(currentPin, PIN_OUTPUT);
					int discPrice = settings->progPrice[15];
					if (status.intDeviceInfo.money_currentBalance >= settings->coinDiscount.coinAfterSumm)
					{
						discPrice = settings->progPrice[15] - (int)(((double)settings->progPrice[15]) * (((double)settings->coinDiscount.coinDiscountSize)/100));
					}
					int bonusCoinCount = (int)(((double)status.intDeviceInfo.money_currentBalance/(double)discPrice) - ((double)status.intDeviceInfo.money_currentBalance/(double)settings->progPrice[15]));
					if ((bonusCoinCount > 0) && (settings->debugFlag.ButtonTerminalThread))
						printf("[DEBUG] ButtonTerminalThread: BONUS COIN %d\n", bonusCoinCount);
					// Запускаем выдачу жетонов
					int outCoinSensorPin = settings->getPinConfig(DVC_SENSOR_COIN_OUT, 1);
					setPinModeMy(outCoinSensorPin, PIN_INPUT);
					int errorCoinCount = 0;
					while ((status.intDeviceInfo.money_currentBalance >= settings->progPrice[15]) && (errorCoinCount < 3))
					{
						//	ВЫДАЧА ЖЕТОНОВ
						setGPIOState(currentPin, 1);
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
							db->Log(DB_EVENT_TYPE_COIN_OUT, discPrice, status.intDeviceInfo.money_currentBalance, "[ButtonTerminalThread]: Out COIN [price, balance]");
						}
						else
						{
							errorCoinCount++;
							printf ("Out coin error: elapsed ctrlTimeout: %d count ctrlTime: %d errCount: \n", ctrlTimeout, ctrlTime, errorCoinCount);
						}
					}

					while ((bonusCoinCount > 0)  && (errorCoinCount < 3))
					{

						//	ВЫДАЧА ЖЕТОНОВ
						setGPIOState(currentPin, 1);
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
							db->Log(DB_EVENT_TYPE_COIN_OUT, bonusCoinCount, status.intDeviceInfo.money_currentBalance, "[ButtonTerminalThread]: Out BONUS COIN [price, balance]");
							bonusCoinCount--;
						}
						else
						{
							errorCoinCount++;
							printf ("Out coin error: elapsed ctrlTimeout: %d count ctrlTime: %d errCount: \n", ctrlTimeout, ctrlTime, errorCoinCount);
						}
					}
					// Останавливаем выдачу жетонов
				}
				// Кнопка нажата менее 30 мсек или более 2000 мсек
				else
				{
					if (settings->debugFlag.ButtonTerminalThread)
						printf("[DEBUG] ButtonTerminalThread: Pressed state on %d button [PIN: %03d] - failed [%d ms]\n", index, currentPin, 50 - timeout);
					db->Log(DB_EVENT_TYPE_EXT_NEW_BUTTON, index, 50 - timeout, "[ButtonTerminalThread]: Button don't detected. Failed");
					thread_timeout -= (50 - timeout);
				}
			}
		}

		///
		/// Обработка эквайринга
		/// Если устройство эквайринга активно
		/// и активны режимы работы
		///
		if (visaDevice->Work)
		{
		}

		if (thread_timeout < 0) thread_timeout = 0;
	}

	db->Log(DB_EVENT_TYPE_DVC_CLOSE, 0, 0, "Button terminal panel device is closed");
	db->Close();
	printf("[DEBUG]: ButtonTerminalThread: Thread is terminate.\n");
	return (void*)0;
}
