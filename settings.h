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

#define DVC_ENGINE								35		//���������
#define DVC_SENSOR_BYPASS						45		//������ ������

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
#define DVC_RELAY_ANTIFROST						68		// �������� ����� ���������� ������ �����

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
#define DVC_COIN_PULSE_ACCEPTOR_ADD				83		//�������������� � ��������� ������
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
	BYTE ButtonMasterThread;		// ���� ����������: ������� ������ � �������� ��� ������ "���� ����������"
	BYTE KKMWatch;					// ���� ����������: ������� ��� ������ � Online KKM (�����-�)
	BYTE VoiceWatch;				// ���� ����������: ������� ���������� ����������
	BYTE RemoteCounterCtrlThread;	// ������� �� ��������� ���������: ������� ��� ��������� ������ � ������������ �� ��������� ������������
	BYTE AlienDeviceThread;			// ������� �� ��������� ���������: ������� ��� ������������ ����� � ����� ������ �� ���������� ������������
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
	// ������������ ������ ������� ���������� � ���� ���
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
	// ***********************************************************************************************************
  	bool 				getEnabledDevice	(int deviceID);								// ����������� ��������� ?
  	bool 				setEnabledDevice	(int deviceID, bool state);				// ���������� ������ ����������
  	unsigned char  		getPinConfig		(int deviceID, unsigned char pinNum);	// �������� ������������ �� �����

	//
	// ���������� ����������� �����
	BYTE		 		progEnabled[16];		// ���������� ������������� ��������
	BYTE		 		progPrice[16];			// ��������� ������ ��� ������ ���������
	char				progName[16][30];		// ����� ��������
	DWORD		 		progRelay[16];			//
	DWORD		 		progRelayBp[16];		//
	DWORD		 		progRPM[16];			//
	DWORD		 		progDepends[16];		//
	DWORD		 		progWinterDelay[16];	// � ������ ������ ���� ����� �� ��������
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
	// ��������� �������� ����� ���������
	RemoteCounterParam			remoteCounterParam;

	//
	// ����� ���������
	int 						CollectionMode; // ����� ��������� 0 - ����������� ����� 1 - ����� �������

	//
	// ����� �� MODBUS
	Modbus 				modbus;

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