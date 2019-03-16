#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#ifndef WORD
#define DWORD			unsigned int
#define WORD			unsigned short
#define BYTE			unsigned char
#define SDWORD			int
#endif

//***************************************
// MODBUS SLAVE DEVICE REGISTERS MAP
//***************************************
#define SLAVE_REG_DEVICE_ADDR			0x00		// Адрес устройства
#define SLAVE_REG_PORT_BAUD				0x01		// Скорость порта
#define SLAVE_REG_IMPULSE_WIDE_1		0x02		// Ширина импульса (2 байта: байтН - Ширина импульса, байтL - Ширина паузы)
#define SLAVE_REG_IMPULSE_WIDE_2		0x03
#define SLAVE_REG_IMPULSE_WIDE_3		0x04
#define SLAVE_REG_IMPULSE_WIDE_4		0x05
#define SLAVE_REG_IMPULSE_COUNT_1		0x06		// Количество импульсов
#define SLAVE_REG_IMPULSE_COUNT_2		0x07
#define SLAVE_REG_IMPULSE_COUNT_3		0x08
#define SLAVE_REG_IMPULSE_COUNT_4		0x09
#define SLAVE_REG_IMPULSE_DIRECTION		0x0A		// Настройка направления импульса (на повышение или на просадку)

//***************************************
// Internal block
//***************************************

#define DVC_RFID_CARD_READER			 		0		//Считыватель RFID карт
#define DVC_RFID_CARD_LIGHT_CONTROL_RED			10		//Подсветка картоприемника RFID - Красная
#define DVC_RFID_CARD_LIGHT_CONTROL_GRN			11  	//Подсветка картоприемника RFID - Зеленая
#define DVC_BUTTON_STOP							21 		//Кнопка "СТОП"
#define DVC_BUTTON01							21 		//Кнопка 1 - "СТОП"
#define DVC_BUTTON02							22
#define DVC_BUTTON03							23
#define DVC_BUTTON04							24
#define DVC_BUTTON05							25
#define DVC_BUTTON06							26
#define DVC_BUTTON07							27
#define DVC_BUTTON08							28
#define DVC_BUTTON09							29
#define DVC_BUTTON10							30
#define DVC_BUTTON11							31
#define DVC_BUTTON12							32		//Кнопка 12
#define DVC_BUTTON_COLLECTION					33		//Кнопка "инкасация"

#define DVC_ENGINE								35		//Двигатель
#define DVC_SENSOR_BYPASS						45		//Датчик потока

#define DVC_RELAY01								51		//Реле 1
#define DVC_RELAY02								52
#define DVC_RELAY03								53
#define DVC_RELAY04								54
#define DVC_RELAY05								55
#define DVC_RELAY06								56
#define DVC_RELAY07								57
#define DVC_RELAY08								58
#define DVC_RELAY09								59
#define DVC_RELAY10								60		//Реле 10
#define DVC_RELAY11								61
#define DVC_RELAY12								62
#define DVC_RELAY13								63
#define DVC_RELAY14								64		//Реле 14

#define DVC_RELAY_RESET							65		// РелеСброса - Инициализируется в IntThread (не используется 07.03.2018)
#define DVC_RELAY_WARM_LINE						66		// РелеПечки - Включается в th_thermal по температуре вместе с печкой
#define DVC_RELAY_LIGHT							67		// РелеСвета - Инициализируется в IntThread работает как адаптивный свет про балансе более 0
#define DVC_RELAY_ANTIFROST						68		// Работает когда включается зимний режим

#define DVC_VALVE01								71		//Клапан 1
#define DVC_VALVE02								72
#define DVC_VALVE03								73
#define DVC_VALVE04								74
#define DVC_VALVE05								75
#define DVC_VALVE06								76
#define DVC_VALVE07								77
#define DVC_VALVE08								78
#define DVC_VALVE09								79
#define DVC_VALVE10								80		//Клапан 10
#define DVC_LED_DISPLAY_EXT						81		//Дисплей внешний
#define DVC_COIN_PULSE_ACCEPTOR					82		//Монетоприемник в пульсовом режиме
#define DVC_COIN_PULSE_ACCEPTOR_ADD				83		//Монетоприемник в пульсовом режиме
#define DVC_ERROR_LED_1							84
#define DVC_ERROR_LED_2							85
#define DVC_ERROR_LED_3							86
#define DVC_ERROR_LED_4							87

#define DVC_GPIO_EXTENDER_RESET					90
#define DVC_GPIO_EXTENDER_SENSOR1				91
#define DVC_GPIO_EXTENDER_SENSOR2				92

#define DVC_HARDWARE_KEY_PROTECT				98

#define MONEY_COIN_TYPE_COUNT	17

struct MoneyCoinInfo
{
	WORD Count[MONEY_COIN_TYPE_COUNT];
};

struct MoneyCoinWeight
{
	WORD Weight[MONEY_COIN_TYPE_COUNT];
};

struct ExtDeviceInfo
{
    DWORD				objectId;                   // ИД Объекта - Мойка
    DWORD				deviceId;                   // ИД Устройства на объекте
    DWORD				userId;                     // ИД Пользователя
    DWORD				serialNum;					// Серийный номер устройства
	BYTE				isReset;					// Был сброс
	BYTE				button_lastEvent;			// Последняя нажатая кнопка
	BYTE				button_currentLight;		// Текущая светящаяся кнопка
	BYTE				button_newEvent;
	BYTE				coin_Present;				// Были операции с жетонами
	BYTE				bill_Present;				// Были операции с купюрами
	MoneyCoinInfo		coin_newIncomeInfo;			// Новый данные с монетоприемника
	MoneyCoinInfo		coin_incomeInfo;			// Данные с монетоприемника
	MoneyCoinInfo		bill_incomeInfo;			// Данные с купюроприемника
	MoneyCoinInfo		bank_incomeInfo;			// Данные с банковского терминала
	BYTE				rfid_cardPresent;			// RFID карты вставлена
	BYTE				rfid_incomeCardNumber[6];	// Текущая магнитная карта RFID
	BYTE				rfid_prevCardNumber[6];		// Предыдущая магнитная карта RFID
	SDWORD				rfid_errorNum;				// Ошибки чтения номера RFID
	BYTE				monitor_currentText[4];		// Текущие данные на дисплее
	SDWORD				remote_currentBalance;		// Информационное поле: баланс
	SDWORD				remote_currentProgram;		// Информационное поле: программа
	SDWORD				sensor_currentVal[8];		// Состояния датчиков
	BYTE				relay_currentVal[24];		// Состояния реле
	int					extCurrentTemp;				// Текущая температура внешнего бокса
	int					collectionButton;			// Состояние кнопки инкасации
};

struct IntDeviceInfo
{
    DWORD				objectId;                   // ИД Объекта - Мойка
    DWORD				deviceId;                   // ИД Устройства на объекте
    DWORD				userId;                     // ИД Пользователя
    DWORD				serialNum;                  // Серийный номер устройства
	DWORD				allMoney;                   // Всего денег
	BYTE				engine_bypassMode;			// Режим байпаса
	DWORD				engine_currentRpm;			// Обороты двигателя (0- стоп)
	BYTE				program_currentProgram;		// Текущая программа
	BYTE				money_moneyCoinUpdate;		// Инфа о монетах и купюрах обработана
	SDWORD				money_currentBalance;		// Текущий баланс
	SDWORD				sensor_currentVal[10];		// Состояния датчиков
	BYTE				relay_currentVal[24];		// Состояния реле
	BYTE				extPrgNeedUpdate;			// Флаг обновления текущей программы
	int					externalTemp;				// Текущая температура с метеостанции
	BYTE				monitor_currentText[4];		// Текущие данные на дисплее
	char				wrkOpened[30];				// Текст: Когда открыта текущая смена
	BYTE				isReset;					// Был сброс
};

struct NetDeviceCommand
{
	DWORD			accept_CRC;						// Val Xor 0x12345678
	ExtDeviceInfo 	extDeviceInfo;					//
	IntDeviceInfo 	intDeviceInfo;					//
};

struct ThreadFlag
{
	BYTE MainWatch;					// Основной поток
	BYTE RFIDWatch;					// Считыватель карт RFID
	BYTE CoinWatch;					// Монетоприемник
	BYTE CoinPulseWatch;			// Монетоприемник ПУЛЬСОВЫЙ РЕЖИМ
	BYTE MoneyWatch;				// Купюроприемник
	BYTE MoneyCCTalkWatch;			// Монетоприемник
	BYTE EngineWatch;				// Двигатель
	BYTE ButtonWatch;				// Кнопки

	BYTE DataExchangeThread;
	BYTE NetServer;					// Подсистема "Сетевой обмен"
	BYTE NetClient;					// Подсистема "Сетевой обмен"
	BYTE MonitorWatch;				// Подсистема "Отображения - дисплей"
	BYTE ExtCommonThread;			// Подсистема "Общие фукции внешнего бокса"
	BYTE IntCommonThread;			// Подсистема "Общие фукции внутреннего бокса"
	BYTE TimeTickThread;			// Подсистема отсчета времени ВНУТРЕННИЙ БОКС
	BYTE DebugThread;				// Процесс вывода отладочной информации
	BYTE ConsoleWatch;				// Процесс консольного управления
	BYTE ThermalWatch;				// Процесс температурного наблюдения
	BYTE GPIOExtWatch;				// Процесс обнаруживающий зависание MCP23x17
	BYTE QueueLog;					// Процесс обрабатывающий запись очереди Лога в базу

	BYTE Lcd20x4Watch;				// ПЫЛЕСОС: Процесс вывода информации на LCD дисплей

	BYTE RemoteCtrlThread;			// БОКС ПОПОЛНЕНИЯ: Процесс работы с удалеными MODBUS контроллерами
	BYTE ButtonMasterThread;		// БОКС ПОПОЛНЕНИЯ: Процесс работы с кнопками при режиме "БОКС ПОПОЛНЕНИЯ"
	BYTE KKMWatch;					// БОКС ПОПОЛНЕНИЯ: Процесс для работы с Online KKM (Штрих-М)
	BYTE VoiceWatch;				// БОКС ПОПОЛНЕНИЯ: Процесс голосового оповещения
	BYTE RemoteCounterCtrlThread;	// СЧЕТЧИК НА УДАЛЕННЫХ АППАРАТАХ: Процесс для обработки данных с контроллеров на стороннем оборудовании
	BYTE AlienDeviceThread;			// СЧЕТЧИК НА УДАЛЕННЫХ АППАРАТАХ: Процесс для формирования чеков и сбора данных со стороннего оборудования
};

struct ErrorFlag
{
	BYTE MainWatch;					// Основной поток
	BYTE RFIDWatch;					// Считыватель карт RFID
	BYTE CoinWatch;					// Монетоприемник
	BYTE CoinPulseWatch;			// Монетоприемник ПУЛЬСОВЫЙ РЕЖИМ
	BYTE MoneyWatch;				// Купюроприемник
	BYTE EngineWatch;				// Двигатель
	BYTE ButtonWatch;				// Кнопки

	BYTE DataExchangeThread;
	BYTE NetServer;					// Подсистема "Сетевой обмен"
	BYTE NetClient;					// Подсистема "Сетевой обмен"
	BYTE MonitorWatch;				// Подсистема "Отображения - дисплей"
	BYTE ExtCommonThread;			// Подсистема "Общие фукции внешнего бокса"
	BYTE IntCommonThread;			// Подсистема "Общие фукции внутреннего бокса"
	BYTE TimeTickThread;			// Подсистема отсчета времени ВНУТРЕННИЙ БОКС
	BYTE DebugThread;				// Процесс вывода отладочной информации
	BYTE ConsoleWatch;				// Процесс консольного управления
	BYTE ThermalWatch;				// Процесс температурного наблюдения
};

struct DeviceInfo
{
	ExtDeviceInfo 	extDeviceInfo;					//
	IntDeviceInfo 	intDeviceInfo;					//
};

struct CommonParams
{
	int delayScrollingTextMs;		// Задержка прокрутки текста если его длина более 4 символов (мСек)
	int display_ReInitTimeMs;		// Время переинициализации контроллера дисплея (мСек)
	int engine_StartStopTimeMs; 	// Время разгона\торможения для двигателя

	int objectId;					// Идентификатор МОЙКИ как объекта для сводной информации
	int deviceId;					// Идентификатор устройства
	int userId;						// Идентификатор пользователя (НЕ ИСПОЛЬЗУЕТСЯ)

	int raspRev;					// Версия управляющего компа (РАЗНОЕ КОЛИЧЕСТВО GPIO)
	int IDKFA;						// Размер бонуса IDKFA
	int showDemo;
	int diagMode;
};

struct WinterMode
{
	bool winterMode;			// Зимний режим
	bool winterDelay;			// Зимняя прокачка магистралей (вкл/выкл)
	bool winterEngineWork;		// Зимний прогон двигателем
	int winterEngineWorkTime;	// Время в секундах прогона
	int winterEngineWorkFreq;	// Частота прогона
	int winterEngineWorkWait;	// Время простоя между прогонами
	int winterWaitTime;			// Время простоя в сек.
	int winterOnTemperature;	// Температура вклчючения зимнего режима
};

struct Modbus
{
	char portName[250];			// Имя порта для интерфейса MODBUS
	int baudRate;				// Скорость интерфейса
	int dataParity;				// Четность
	int dataBit;				// Количество битов данных
	int stopBit;				// Стоповые биты
	int slaveCount;				// Количество управляемых устройств [0..12]
};

struct NetServerConfig
{
	int PortNumber;				// Номер порта сервера
};

struct NetClientConfig
{
	int PortNumber;				// Номер порта для соединения
	char netServerAddr[20];		// Адрес устройства для соединения
};

struct DatabaseSettings
{
	char serverName[100];
	char databaseName[50];
	char userId[25];
	char userPasswd[25];
};

struct ThermalParam
{
	char portName[250];			// Имя порта датчика температуры
	WORD thermalRelay;			// Номер реле (1-32) используемого для управления отопителем
	int tempOn;					// Температура включения
	int tempOff;				// Температура отключения
};

struct RemoteCounterParam
{
	int DocumentCreationTime;	// Количество секунд, через которое сформируется чек в ККМ
	int PriceIN1;				// Номинал импульса с входа №1
	int PriceIN2;				// Номинал импульса с входа №2
};

struct RejectedCoinInfo
{
	bool coinRejected;
	int dataCnt;
	int dataCrd;
	int dataErr;
	int dataRes;
};

struct KKMParam
{
	char kkmAddr[250];
	int kkmPort;
	BYTE kkmPass;
	int QueryTime;
	// Наименование услуги которая печатается в чеке ККМ
	char ServiceName[250];
	int TaxType;
};

class Settings
{
private:
	static Settings* p_instance;
	Settings();
	Settings( const Settings& );
	Settings& operator=( Settings& );
	//
	// Общие параметры "Устройств" и "Подсистем"
	DWORD		 		pinDevice[100];			// Массив конфигурационных параметров (0x01020304) (Для каждого устройства назначается PIN до 4-х шт)
	BYTE		 		enabledDevice[100];		// Массив флагов разрешения использования
	dictionary*   		ini;

public:
    //
    // Общие параметры "Устройств" и "Подсистем"
	ThreadFlag			threadFlag;				// Флаги отражающие использования потоков подсистем и устройств
	ThreadFlag			busyFlag;				// Флаги отражающие занятость устройств, запрещение их опроса
	ThreadFlag			workFlag;				// Флаги отражающие работы потоков
	ThreadFlag			debugFlag;				// Флаги отражающие отладку потоков
	ErrorFlag			intErrorCode;				// Коды ошибок по потокам
	ErrorFlag			extErrorCode;				// Коды ошибок по потокам
	CommonParams		commonParams;			// Общие параметры системы
	ThermalParam		thermalParam;			// Параметры подогрева, управления отопителем
	WinterMode			winterMode;				// Параметры зимнего режима
	unsigned char		monitorLight;   		// Степень свечения 7сег дисплея
	unsigned int		digitalKey;
	unsigned int		digitalVector;			// Вектор безопастности
	unsigned int		useMCP;					// Используется расширитель MCP2307
	unsigned int		useMCPWatch;			// Используется контроль зависания MCP2307
	unsigned int		useAutoReloadConfig;	// Используется авто загрузка измененныз параметров
	unsigned int		coinPulseWidth;			// Ширина импульса от импульсного монетника в МСек
	unsigned int		resetUsbDevice;			// Сбрасывать USB устройства при перезагрузке программы
	unsigned int		useDatabase;			// Использовать базу данных
	unsigned int		useDatabaseDateTime;	// Использовать базу данных как источник Даты и времени
	unsigned int		useEeprom;				// Использовать EEPROM
	unsigned int		useEepromDateTime;		// Использовать EEPROM как источник Даты и времени
	unsigned int		useEepromParams;		// Использовать EEPROM как параметров программ
	unsigned int		LightTimeOff;			// Время в сек. перед выключением света
	unsigned int		useHWClock;				// Используется RTC и время мы не контролируем
	// ***********************************************************************************************************
  	bool 				getEnabledDevice	(int deviceID);								// Усттройство разрешено ?
  	bool 				setEnabledDevice	(int deviceID, bool state);				// Установить статус устройства
  	unsigned char  		getPinConfig		(int deviceID, unsigned char pinNum);	// Получить конфигурацию по ПИНАМ

	//
	// Управление программами мойки
	BYTE		 		progEnabled[16];		// Разрешение использования программ
	BYTE		 		progPrice[16];			// Стоимость минуты для каждой программы
	char				progName[16][30];		// Имена программ
	DWORD		 		progRelay[16];			//
	DWORD		 		progRelayBp[16];		//
	DWORD		 		progRPM[16];			//
	DWORD		 		progDepends[16];		//
	DWORD		 		progWinterDelay[16];	// В зимнем режиме есть пауза на прокачку
	//
	// Устройство "КУПЮРОПРИЕМНИК"
	MoneyCoinWeight		moneyWeight;

	// Устройство "МОНЕТОПРИЕМНИК"
	MoneyCoinWeight		coinWeight;

	//
	// Подсистема "СЕРВЕР"
	NetServerConfig		netServerConfig;
	//
	// Подсистема "КЛИЕНТ"
	NetClientConfig		extPanelNetConfig;

	//
	// Работа с ККМ
	KKMParam					kkmParam;

	//
	// Параметры счетчика чужих устройств
	RemoteCounterParam			remoteCounterParam;

	//
	// Режим инкасации
	int 						CollectionMode; // Режим инкасации 0 - стандартный режим 1 - общий счетчик

	//
	// Обмен по MODBUS
	Modbus 				modbus;

	//
	// СКИДКИ
	int 				discountSize;			// Текущая скидка на стоиомсть программы из БД
	int 				discountCardDeposit;	// Скидка на пополнение при активной карте из БД
	int 				cardBonus;				// Скидка на пополнение при активной карте из Конф файла
	int 				moneyBonus;				// Скидка на пополнение наличными
	//
	// Работа с БД
	DatabaseSettings	gdatabaseSettings;
	DatabaseSettings	databaseSettings;

	int 				engine_baundRate;
	int					engine_deviceType;
	bool 				useAutoStop;
	int					countAutoStop;
	int					valveTimeOff;

  	bool 				loadConfig			(char* fileName);
  	bool 				saveConfig			(char* fileName);

    static Settings* getInstance() {
        if(!p_instance)
            p_instance = new Settings();
        return p_instance;
    }
};
#endif