#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#ifndef WORD
#define DWORD			unsigned int
#define WORD			unsigned short
#define BYTE			unsigned char
#define SDWORD			int
#endif

//***************************************
// bwPay Application ERROR CODE
//***************************************
#define BWPAY_ERROR						0x00		// ����� ����������
#define BWPAY_NO_ERROR					0x01		// ����� ����������

#define PAY_RESULT_OK					0x00		// ����� ����������
#define PAY_RESULT_ERROR				0x01		// ����� ����������


//***************************************
// MODBUS SLAVE DEVICE REGISTERS MAP
//***************************************
#define SLAVE_REG_DEVICE_ADDR			0x00		// ����� ����������
#define SLAVE_REG_PORT_BAUD				0x01		// �������� �����
#define SLAVE_REG_IMPULSE_WIDE_1		0x02		// ������ �������� (2 �����: ����� - ������ ��������, ����L - ������ �����)
#define SLAVE_REG_IMPULSE_WIDE_2		0x03
#define SLAVE_REG_IMPULSE_WIDE_3		0x04
#define SLAVE_REG_IMPULSE_WIDE_4		0x05
#define SLAVE_REG_IMPULSE_COUNT_1		0x06		// ���������� ���������
#define SLAVE_REG_IMPULSE_COUNT_2		0x07
#define SLAVE_REG_IMPULSE_COUNT_3		0x08
#define SLAVE_REG_IMPULSE_COUNT_4		0x09
#define SLAVE_REG_IMPULSE_DIRECTION		0x0A		// ��������� ����������� �������� (�� ��������� ��� �� ��������)

//***************************************
// Internal block
//***************************************

#define DVC_RFID_CARD_READER			 		0		//����������� RFID ����
#define DVC_RFID_CARD_LIGHT_CONTROL_RED			10		//��������� �������������� RFID - �������
#define DVC_RFID_CARD_LIGHT_CONTROL_GRN			11  	//��������� �������������� RFID - �������
#define DVC_BUTTON_STOP							21 		//������ "����"
#define DVC_BUTTON01							21 		//������ 1 - "����"
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
#define DVC_BUTTON12							32		//������ 12
#define DVC_BUTTON_COLLECTION					33		//������ "���������"
#define DVC_BUTTON_SETTINGS						34		//������ "���������"

#define DVC_ENGINE								35		//���������

#define DVC_BUTTON_OUT_COIN						41		//������ ������ �������
#define DVC_SENSOR_COIN_OUT						42		//������ ��������� ������� � �������
#define DVC_RELAY_OUT_COIN						43		//������ �� ������ �������

#define DVC_SENSOR_BYPASS						45		//������ ������

#define DVC_BUTTON_OUT_RFID_CARD				46		//������ ������ ����� ����������
#define DVC_SENSOR_EMPTY_RFID_CARD				47		//������ ��������� ���� � ����������
#define DVC_RELAY_OUT_RFID_CARD					48		//������ �� ������ ����� ����������


#define DVC_RELAY01								51		//���� 1
#define DVC_RELAY02								52
#define DVC_RELAY03								53
#define DVC_RELAY04								54
#define DVC_RELAY05								55
#define DVC_RELAY06								56
#define DVC_RELAY07								57
#define DVC_RELAY08								58
#define DVC_RELAY09								59
#define DVC_RELAY10								60		//���� 10
#define DVC_RELAY11								61
#define DVC_RELAY12								62
#define DVC_RELAY13								63
#define DVC_RELAY14								64		//���� 14

#define DVC_RELAY_RESET							65		// ���������� - ���������������� � IntThread (�� ������������ 07.03.2018)
#define DVC_RELAY_WARM_LINE						66		// ��������� - ���������� � th_thermal �� ����������� ������ � ������
#define DVC_RELAY_LIGHT							67		// ��������� - ���������������� � IntThread �������� ��� ���������� ���� ��� ������� ����� 0
#define DVC_RELAY_ANTIFROST						68		// �������� ����� ���������� ������ �����. ���� ��� ��������� ��� 0, �� 
														// � �������� ��������� ��������� ���� ������������ ���� � ��������� ����1 (����-�����.)

#define DVC_VALVE01								71		//������ 1
#define DVC_VALVE02								72
#define DVC_VALVE03								73
#define DVC_VALVE04								74
#define DVC_VALVE05								75
#define DVC_VALVE06								76
#define DVC_VALVE07								77
#define DVC_VALVE08								78
#define DVC_VALVE09								79
#define DVC_VALVE10								80		//������ 10
#define DVC_LED_DISPLAY_EXT						81		//������� �������
#define DVC_COIN_PULSE_ACCEPTOR					82		//�������������� � ��������� ������
#define DVC_COIN_PULSE_ACCEPTOR_INHIBIT			83		//������ ��������
#define DVC_ERROR_LED_1							84
#define DVC_ERROR_LED_2							85
#define DVC_ERROR_LED_3							86
#define DVC_ERROR_LED_4							87

#define DVC_OSMOS_BANK_FULL						88
#define DVC_OSMOS_LOW_PRESSURE					89

#define DVC_GPIO_EXTENDER_RESET					90
#define DVC_GPIO_EXTENDER_SENSOR1				91
#define DVC_GPIO_EXTENDER_SENSOR2				92

#define DVC_HARDWARE_KEY_PROTECT				98

#define MONEY_COIN_TYPE_COUNT	17

#define MENU_X									4
#define MENU_Y									16

enum TDeviceWorkMode
{
	WorkMode = 1,
	CollectionMode = 2,
	SettingsMode = 3,
	VISAMode = 4
};

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
    DWORD				objectId;                   // �� ������� - �����
    DWORD				deviceId;                   // �� ���������� �� �������
    DWORD				userId;                     // �� ������������
    DWORD				serialNum;					// �������� ����� ����������
	BYTE				isReset;					// ��� �����
	BYTE				button_lastEvent;			// ��������� ������� ������
	BYTE				button_currentLight;		// ������� ���������� ������
	BYTE				button_newEvent;
	BYTE				coin_Present;				// ���� �������� � ��������
	BYTE				bill_Present;				// ���� �������� � ��������
	MoneyCoinInfo		coin_newIncomeInfo;			// ����� ������ � ���������������
	MoneyCoinInfo		coin_incomeInfo;			// ������ � ���������������
	MoneyCoinInfo		bill_incomeInfo;			// ������ � ���������������
	MoneyCoinInfo		bank_incomeInfo;			// ������ � ����������� ���������
	BYTE				rfid_cardPresent;			// RFID ����� ���������
	BYTE				rfid_incomeCardNumber[6];	// ������� ��������� ����� RFID
	BYTE				rfid_prevCardNumber[6];		// ���������� ��������� ����� RFID
	SDWORD				rfid_errorNum;				// ������ ������ ������ RFID
	BYTE				monitor_currentText[4];		// ������� ������ �� �������
	SDWORD				remote_currentBalance;		// �������������� ����: ������
	SDWORD				remote_currentProgram;		// �������������� ����: ���������
	SDWORD				sensor_currentVal[8];		// ��������� ��������
	BYTE				relay_currentVal[24];		// ��������� ����
	int					extCurrentTemp;				// ������� ����������� �������� �����
	int					collectionButton;			// ��������� ������ ���������
};

struct IntDeviceInfo
{
    DWORD				objectId;                   // �� ������� - �����
    DWORD				deviceId;                   // �� ���������� �� �������
    DWORD				userId;                     // �� ������������
    DWORD				serialNum;                  // �������� ����� ����������
	DWORD				allMoney;                   // ����� �����
	BYTE				engine_bypassMode;			// ����� �������
	DWORD				engine_currentRpm;			// ������� ��������� (0- ����)
	BYTE				program_currentProgram;		// ������� ���������
	BYTE				money_moneyCoinUpdate;		// ���� � ������� � ������� ����������
	SDWORD				money_currentBalance;		// ������� ������
	SDWORD				sensor_currentVal[10];		// ��������� ��������
	BYTE				relay_currentVal[24];		// ��������� ����
	BYTE				extPrgNeedUpdate;			// ���� ���������� ������� ���������
	int					externalTemp;				// ������� ����������� � ������������
	BYTE				monitor_currentText[4];		// ������� ������ �� �������
	char				wrkOpened[30];				// �����: ����� ������� ������� �����
	BYTE				isReset;					// ��� �����
};

struct NetDeviceCommand
{
	DWORD			accept_CRC;						// Val Xor 0x12345678
	ExtDeviceInfo 	extDeviceInfo;					//
	IntDeviceInfo 	intDeviceInfo;					//
};

struct ThreadFlag
{
	BYTE MainWatch;					// �������� �����
	BYTE RFIDWatch;					// ����������� ���� RFID
	BYTE CoinWatch;					// ��������������
	BYTE CoinPulseWatch;			// �������������� ��������� �����
	BYTE MoneyWatch;				// ��������������
	BYTE MoneyCCTalkWatch;			// ��������������
	BYTE EngineWatch;				// ���������
	BYTE ButtonWatch;				// ������

	BYTE DataExchangeThread;
	BYTE NetServer;					// ���������� "������� �����"
	BYTE NetClient;					// ���������� "������� �����"
	BYTE MonitorWatch;				// ���������� "����������� - �������"
	BYTE ExtCommonThread;			// ���������� "����� ������ �������� �����"
	BYTE IntCommonThread;			// ���������� "����� ������ ����������� �����"
	BYTE TimeTickThread;			// ���������� ������� ������� ���������� ����
	BYTE DebugThread;				// ������� ������ ���������� ����������
	BYTE ConsoleWatch;				// ������� ����������� ����������
	BYTE ThermalWatch;				// ������� �������������� ����������
	BYTE GPIOExtWatch;				// ������� �������������� ��������� MCP23x17
	BYTE QueueLog;					// ������� �������������� ������ ������� ���� � ����

	BYTE Lcd20x4Watch;				// �������: ������� ������ ���������� �� LCD �������

	BYTE RemoteCtrlThread;			// ���� ����������: ������� ������ � ��������� MODBUS �������������
	BYTE RemoteSenderThread;		// ���� ����������: ������� ���������� ��������� ������
	BYTE ButtonMasterThread;		// ���� ����������: ������� ������ � �������� ��� ������ "���� ����������"
	BYTE KKMWatch;					// ���� ����������: ������� ��� ������ � Online KKM (�����-�)
	BYTE VoiceWatch;				// ���� ����������: ������� ���������� ����������
	BYTE RemoteCounterCtrlThread;	// ������� �� ��������� ���������: ������� ��� ��������� ������ � ������������ �� ��������� ������������
	BYTE AlienDeviceThread;			// ������� �� ��������� ���������: ������� ��� ������������ ����� � ����� ������ �� ���������� ������������
	BYTE OsmosThread;				// ����� ������: ���������� ���������� ������
	BYTE ButtonTerminalThread;		// ����� ��������� ����������, ������ ���� � �������
	BYTE VisaDeviceThread;			// ����� ���������� ���������� (PAX D200)
};

struct ErrorFlag
{
	BYTE MainWatch;					// �������� �����
	BYTE RFIDWatch;					// ����������� ���� RFID
	BYTE CoinWatch;					// ��������������
	BYTE CoinPulseWatch;			// �������������� ��������� �����
	BYTE MoneyWatch;				// ��������������
	BYTE EngineWatch;				// ���������
	BYTE ButtonWatch;				// ������

	BYTE DataExchangeThread;
	BYTE NetServer;					// ���������� "������� �����"
	BYTE NetClient;					// ���������� "������� �����"
	BYTE MonitorWatch;				// ���������� "����������� - �������"
	BYTE ExtCommonThread;			// ���������� "����� ������ �������� �����"
	BYTE IntCommonThread;			// ���������� "����� ������ ����������� �����"
	BYTE TimeTickThread;			// ���������� ������� ������� ���������� ����
	BYTE DebugThread;				// ������� ������ ���������� ����������
	BYTE ConsoleWatch;				// ������� ����������� ����������
	BYTE ThermalWatch;				// ������� �������������� ����������
};

struct DeviceInfo
{
	ExtDeviceInfo 	extDeviceInfo;					//
	IntDeviceInfo 	intDeviceInfo;					//
};

struct CoinDiscount
{
	int				coinAfterSumm;					//	����� �����	����� ������ �� ������
	int 			coinDiscountSize;				//  ������ �� ��������� ������ ����� ������������ ������
};

struct CommonParams
{
	int delayScrollingTextMs;		// �������� ��������� ������ ���� ��� ����� ����� 4 �������� (����)
	int display_ReInitTimeMs;		// ����� ����������������� ����������� ������� (����)
	int engine_StartStopTimeMs; 	// ����� �������\���������� ��� ���������

	int objectId;					// ������������� ����� ��� ������� ��� ������� ����������
	int deviceId;					// ������������� ����������
	int userId;						// ������������� ������������ (�� ������������)

	int raspRev;					// ������ ������������ ����� (������ ���������� GPIO)
	int IDKFA;						// ������ ������ IDKFA
	int showDemo;
	int diagMode;
};

struct WinterMode
{
	bool winterMode;			// ������ �����
	bool winterDelay;			// ������ �������� ����������� (���/����)
	bool winterEngineWork;		// ������ ������ ����������
	int winterEngineWorkTime;	// ����� � �������� �������
	int winterEngineWorkFreq;	// ������� �������
	int winterEngineWorkWait;	// ����� ������� ����� ���������
	int winterWaitTime;			// ����� ������� � ���.
	int winterOnTemperature;	// ����������� ���������� ������� ������
};

struct Modbus
{
	char portName[250];			// ��� ����� ��� ���������� MODBUS
	int baudRate;				// �������� ����������
	int dataParity;				// ��������
	int dataBit;				// ���������� ����� ������
	int stopBit;				// �������� ����
	int slaveCount;				// ���������� ����������� ��������� [0..12]
};

struct Osmos
{
	int gidrodynWaitTime;		// ���������� � ���. ����� �������� ���������� ����������������� �������� �� ��������� ������
	int gidrodynTurnOnTime;		// ����� � ���. �� �������� ���������� ����������������� �������� �� ��������� ������
	int waitOnFull;				// ����� � ���. ������ �������
	int permiatLoadTime;		// ����� � ���. ���������� ����� ���������
	int waitOnEmpty;			// ����� � ���. ������� �� ������� �� ������ ������
};

struct NetServerConfig
{
	int PortNumber;				// ����� ����� �������
};

struct NetClientConfig
{
	int PortNumber;				// ����� ����� ��� ����������
	char netServerAddr[20];		// ����� ���������� ��� ����������
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
	char portName[250];			// ��� ����� ������� �����������
	WORD thermalRelay;			// ����� ���� (1-32) ������������� ��� ���������� ����������
	int tempOn;					// ����������� ���������
	int tempOff;				// ����������� ����������
};

struct RemoteCounterParam
{
	int DocumentCreationTime;	// ���������� ������, ����� ������� ������������ ��� � ���
	int PriceIN1;				// ������� �������� � ����� �1
	int PriceIN2;				// ������� �������� � ����� �2
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
	int CutType;
	// ������������ ������ ������� ���������� � ���� ���
	char ServiceName[250];
	int TaxType;
	int LocalTime;			//	1 - ������������ ��������� ����� ��� (������ +4 ����)
	int MaxAmount;			//	����� ������������ ������� ��� �������� ����.
							//	�����  - �� ������, ������ ������
	int SharedMode;			// ����� ������������ ������� (�������������, ������ ���, �������)
};

struct AutoRebootParam
{
	int AutoReboot;
	int AutoRebootHour;
};

struct WorkTimeDevice		// ������� ����� �������
{
	int 	UseWorkTime;
	int		StartTimeHour;
	int 	StartTimeMinute;
	int		StopTimeHour;
	int 	StopTimeMinute;
};

struct VISAButton
{
	BYTE pinNum;
	BYTE pinEnable;
};

struct VISAParam			// ��������� ����������
{
	char portName[250];			// ��� ����� ��� ���������� MODBUS
	int baudRate;				// �������� ����������
	int dataParity;				// ��������
	int dataBit;				// ���������� ����� ������
	int stopBit;				// �������� ����

	char login[126];			// ����� �������
	char password[126];			// ������ �������
	int	 debug;					// ���� ������ �������

	int workMode;				// ����� ������
								// 0 - ����� ���������������� ����� ������� �������
								// 1 - ������ -10 +10 +50 �������� ������
	VISAButton sub10Btn;
	VISAButton add10Btn;
	VISAButton add50Btn;
	VISAButton add100Btn;
	VISAButton add500Btn;

	VISAButton payBtn;
	VISAButton cancelBtn;

	VISAButton pay50Btn;
	VISAButton pay100Btn;
	VISAButton pay150Btn;
	VISAButton pay200Btn;
	VISAButton pay500Btn;
};

struct MenuDescriptor
{
	int 	menuID;
	int 	menuSlave;
	char 	menuName[4];
};

struct ServiceCards
{
	int prgNumber;
	int washBalance;
	DWORD cardId[5];
};

struct TerminalParam
{
	char deviceSubnet[255];
};

struct LCD24Param
{
	int balStrNum;
	int prgStrNum;
	int lineStrNum;
	int adsStrNum;
};

class Settings
{
private:
	static Settings* p_instance;
	Settings();
	Settings( const Settings& );
	Settings& operator=( Settings& );
	//
	// ����� ��������� "���������" � "���������"
	DWORD		 		pinDevice[100];			// ������ ���������������� ���������� (0x01020304) (��� ������� ���������� ����������� PIN �� 4-� ��)
	BYTE		 		enabledDevice[100];		// ������ ������ ���������� �������������
	dictionary*   		ini;

public:
    //
    // ����� ��������� "���������" � "���������"
	ThreadFlag			threadFlag;				// ����� ���������� ������������� ������� ��������� � ���������
	ThreadFlag			busyFlag;				// ����� ���������� ��������� ���������, ���������� �� ������
	ThreadFlag			workFlag;				// ����� ���������� ������ �������
	ThreadFlag			debugFlag;				// ����� ���������� ������� �������
	ErrorFlag			intErrorCode;				// ���� ������ �� �������
	ErrorFlag			extErrorCode;				// ���� ������ �� �������
	CommonParams		commonParams;			// ����� ��������� �������
	ThermalParam		thermalParam;			// ��������� ���������, ���������� ����������
	WinterMode			winterMode;				// ��������� ������� ������
	unsigned char		monitorLight;   		// ������� �������� 7��� �������
	unsigned int		digitalKey;
	unsigned int		digitalVector;			// ������ �������������
	unsigned int		useMCP;					// ������������ ����������� MCP2307
	unsigned int		useMCPWatch;			// ������������ �������� ��������� MCP2307
	unsigned int		useAutoReloadConfig;	// ������������ ���� �������� ���������� ����������
	unsigned int		coinPulseWidth;			// ������ �������� �� ����������� ��������� � ����
	unsigned int		resetUsbDevice;			// ���������� USB ���������� ��� ������������ ���������
	unsigned int		useDatabase;			// ������������ ���� ������
	unsigned int		useDatabaseDateTime;	// ������������ ���� ������ ��� �������� ���� � �������
	unsigned int		useEeprom;				// ������������ EEPROM
	unsigned int		useEepromDateTime;		// ������������ EEPROM ��� �������� ���� � �������
	unsigned int		useEepromParams;		// ������������ EEPROM ��� ���������� ��������
	unsigned int		LightTimeOff;			// ����� � ���. ����� ����������� �����
	unsigned int		useHWClock;				// ������������ RTC � ����� �� �� ������������
	unsigned int		useStoreBalance;		// ��������� ������ � EEPROM
	char				ethName[40];			// ��� �������� ���������� (default: eth0)
	// ***********************************************************************************************************
  	bool 				getEnabledDevice	(int deviceID);								// ����������� ��������� ?
  	bool 				setEnabledDevice	(int deviceID, bool state);				// ���������� ������ ����������
  	unsigned char  		getPinConfig		(int deviceID, unsigned char pinNum);	// �������� ������������ �� �����

	//
	// ���������� ����������� �����
	BYTE		 		progEnabled[32];		// ���������� ������������� ��������
	WORD		 		progPrice[32];			// ��������� ������ ��� ������ ���������
												// prgPrice[14] ��������� ����� ��� ���������� ����
												// prgPrice[15] ��������� ������ ��� �������
	BYTE		 		progStopBP[32];			// ������������� ������ ������� ��� ��������
	char				progName[32][30];		// ����� ��������
	DWORD		 		progRelay[32];			//
	DWORD		 		progRelayBp[32];		//
	DWORD		 		progRPM[32];			//
	DWORD		 		progDepends[32];		//
	DWORD		 		progWinterDelay[32];	// � ������ ������ ���� ����� �� ��������
	DWORD		 		progLimitSumm[32];			//
	DWORD		 		progLimitRelay[32];			//
	//
	// ���������� ��� ��� ������ ���������
	// ��� ����������� ���������� ���������� ��� � �������
	int					showAppLabel;
	//
	// ������ �� ������
	CoinDiscount		coinDiscount;
	//
	// ���������� "��������������"
	MoneyCoinWeight		moneyWeight;

	// ���������� "��������������"
	MoneyCoinWeight		coinWeight;

	//
	// ���������� "������"
	NetServerConfig		netServerConfig;
	//
	// ���������� "������"
	NetClientConfig		extPanelNetConfig;

	//
	// ������ � ���
	KKMParam					kkmParam;

	//
	// ��������� ������
	Osmos						osmosParam;

	//
	// ��������� �������� ����� ���������
	RemoteCounterParam			remoteCounterParam;

	//
	// ����� ���������
	int 						CollectionMode; // ����� ��������� 0 - ����������� ����� 1 - ����� �������

	//
	// ����� �� MODBUS
	Modbus 				modbus;

	//
	// ��������� ����������
	VISAParam visaParam;

	//
	// ��������� ���������
	TerminalParam terminalParam;

	//
	// ��������� LCD24x4
	LCD24Param lcd24Param;
	//
	// ��������� �����
	// [ServiceCards]
	// PrgNumber = 8
	// CardID_1 = xxxxxx
	// WashBalance = 123
	ServiceCards serviceCards;

	//
	// ��������� �������������� ������������
	AutoRebootParam autoRebootParam;

	//
	// ������
	int 				discountSize;			// ������� ������ �� ��������� ��������� �� ��
	int 				discountCardDeposit;	// ������ �� ���������� ��� �������� ����� �� ��
	int 				cardBonus;				// ������ �� ���������� ��� �������� ����� �� ���� �����
	int 				moneyBonus;				// ������ �� ���������� ���������
	//
	// ������ � ��
	DatabaseSettings	gdatabaseSettings;
	DatabaseSettings	databaseSettings;

	//
	// ������� ����� �������
	WorkTimeDevice		workTimeDevice;

	int 				engine_baundRate;
	int					engine_deviceType;
	bool 				useAutoStop;
	int					countAutoStop;
	int					valveTimeOff;
	DWORD				engine_relay;

	int 				dayLightSaving;
	int 				dayLightSavingOnHour;
	int 				dayLightSavingOffHour;

  	bool 				loadConfig			(char* fileName);
  	bool 				saveConfig			(char* fileName);

  	// MENU
  	MenuDescriptor		menu[MENU_X][MENU_Y];
  	int 				menuX = 0;
  	int					menuY = 0;
	void menuLeft()
	{if (menuX == 0) menuX = MENU_X-1; else menuX--; menuY = 0;}
	void menuRight()
	{if (menuX == MENU_X-1) menuX = 0; else menuX++; menuY = 0;}
	void menuUp()
	{if (menuY == 0) menuY = MENU_Y-1; else menuY--;}
	void menuDown()
	{if (menuY == MENU_Y-1) menuY = 0; else menuY++;}

    static Settings* getInstance() {
        if(!p_instance)
            p_instance = new Settings();
        return p_instance;
    }
};
#endif