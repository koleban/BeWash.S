#include "main.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"

extern char comports[30][60];

Settings* Settings::p_instance = 0;

Settings::Settings()
{
	memset(&coinDiscount, 	0x00, sizeof(coinDiscount));
	memset(&moneyWeight, 	0x00, sizeof(moneyWeight));
	memset(&coinWeight, 	0x00, sizeof(coinWeight));

	memset(&enabledDevice, 	0x00, sizeof(enabledDevice));
	memset(&pinDevice, 		0xFF, sizeof(pinDevice));
	memset(&progPrice, 		0x00, sizeof(progPrice));
	memset(&progStopBP, 	0x00, sizeof(progStopBP));
	memset(&progEnabled, 	0x01, sizeof(progEnabled));
	memset(&progName,		0x00, sizeof(progName));
	memset(&progRelay,		0xFF, sizeof(progRelay));
	memset(&progRelayBp,	0xFF, sizeof(progRelay));
	memset(&progRPM,		0x00, sizeof(progRPM));
	memset(&progDepends,	0x00, sizeof(progDepends));
	memset(&progLimitSumm,	0x00, sizeof(progLimitSumm));
	memset(&progLimitRelay,	0x00, sizeof(progLimitRelay));
	memset(&databaseSettings, 0x00, sizeof(databaseSettings));
	memset(&gdatabaseSettings, 0x00, sizeof(gdatabaseSettings));
	memset(&workTimeDevice, 0x00, sizeof(workTimeDevice));
	memset(&terminalParam, 0x00, sizeof(terminalParam));
	memset(&lcd24Param, 0x00, sizeof(lcd24Param));
	memset(&autoRebootParam, 0x00, sizeof(autoRebootParam));

	memset(&modbus, 0x00, sizeof(modbus));
	memset(&winterMode, 0, sizeof(winterMode));
	memset(&thermalParam, 0, sizeof(thermalParam));
	memset(&threadFlag, 0, sizeof(threadFlag));
	memset(&busyFlag, 0, sizeof(busyFlag));
	memset(&workFlag, 0, sizeof(workFlag));
	memset(&debugFlag, 0, sizeof(debugFlag));
	memset(&intErrorCode, 0, sizeof(intErrorCode));
	memset(&extErrorCode, 0, sizeof(extErrorCode));
	memset(&kkmParam, 0, sizeof(kkmParam));
	memset(&osmosParam, 0, sizeof(osmosParam));
	memset(&remoteCounterParam, 0, sizeof(remoteCounterParam));
	memset(&remoteCounter, 0, sizeof(remoteCounter));
	memset(&remoteCounterSumm, 0, sizeof(remoteCounterSumm));
	memset(&visaParam, 0, sizeof(visaParam));
	memset(&serviceCards, 0, sizeof(serviceCards));

	int model, rev, mem, maker, overVolted ;
	piBoardId (&model, &rev, &mem, &maker, &overVolted) ;
	commonParams.raspRev = rev;
	commonParams.showDemo = 0;
	discountSize = 0;
	discountCardDeposit = 0;

	memset(comports, 0, sizeof(comports));
	for (int index=0; index < 8; index++)
	{
		unsigned int len = sprintf(comports[index], 	"/dev/ttyUSB%d", index);
		comports[index][len] = 0x00;
	}
	sprintf(comports[8], 	"/dev/serial0");
	sprintf(comports[9], 	"/dev/serial1");
	sprintf(comports[10], 	"/dev/serial2");	// !!! WARNING comports[10] - Use for engine communication !!!
	sprintf(comports[11], 	"/dev/ttyAMA0");
	sprintf(comports[12], 	"/dev/ttyACM0");

	engine_deviceType = 0;
	coinPulseWidth = 150;

	monitorLight = 0x0E;
	useMCP = 0;

	// >>
	// Structure remoteCtrl is a array. remoteCtrl descripting
	// remote device. This is interface pro method and properties
	// This loop define all devices modbus number 1 - 30
	//
	memset(&remoteCtrl, 0x00, sizeof(remoteCtrl));
	for (int index=0; index < 30; index++)
		remoteCtrl[index].devId = index;
	// <<
}

bool Settings::getEnabledDevice(int deviceID)
{
	if (deviceID > sizeof(enabledDevice)) return 0;
	return (enabledDevice[deviceID] == 1);
}

bool Settings::setEnabledDevice(int deviceID, bool state)
{
	if (deviceID > sizeof(enabledDevice)) return 0;
	enabledDevice[deviceID] = state;
	return 1;
}

unsigned char Settings::getPinConfig(int deviceID, unsigned char pinNum)
{
	if (deviceID > sizeof(enabledDevice)) return 0;
	if (deviceID > sizeof(pinDevice)) return 0;
	if (pinNum > 4) return 0;
	if (pinNum == 0) pinNum++;
	unsigned char pinNumber = (pinDevice[deviceID] >> ((pinNum - 1) * 8)) & 0xFF;

	return (pinNumber);
}

bool Settings::loadConfig (char* fileName)
{
	int index = 0;
	char paramName[100];
	ini = iniparser_load(fileName);
	if (ini == NULL)
	{
		printf("Error: cannot parse config file: %s\n", fileName);
		return 1;
	}

	memset(paramName, 0, sizeof(paramName));

	printf("Load config file ...");
	commonParams.delayScrollingTextMs 	= iniparser_getuint(ini, "Common:DelayScrollingText", 	 1000);
	commonParams.display_ReInitTimeMs 	= iniparser_getuint(ini, "Common:DisplayReInitTimeMs", 	 1000);
	commonParams.engine_StartStopTimeMs = iniparser_getuint(ini, "Common:EngineStartStopTimeMs", 1200);

	commonParams.objectId 				= iniparser_getuint(ini, "Common:ObjectId", 			0);
	commonParams.deviceId 				= iniparser_getuint(ini, "Common:DeviceId", 			255);
	commonParams.userId 				= iniparser_getuint(ini, "Common:UserId", 				0);

	commonParams.showDemo 				= iniparser_getuint(ini, "Common:ShowDemo", 			1);

	commonParams.IDKFA 					= iniparser_getuint(ini, "Common:IDKFA", 				0);
	useAutoStop 						= iniparser_getuint(ini, "Common:UseAutoStop", 			1);
	countAutoStop 						= iniparser_getuint(ini, "Common:CoinAutoStop", 		8);
	monitorLight 						= iniparser_getuint(ini, "Common:MonitorLight", 		0x0E) & 0x0F;
	useMCP 								= iniparser_getuint(ini, "Common:UseMCP", 				0);
	useMCPWatch							= iniparser_getuint(ini, "Common:UseMCPWatch", 			useMCP);
	useAutoReloadConfig					= iniparser_getuint(ini, "Common:UseAutoLoadCfg", 		0);
	CollectionMode						= iniparser_getuint(ini, "Common:CollectionMode", 		0);
	////////////////////
	// Don't reload config from local file
	// Paramter values setting in database
	useAutoReloadConfig					= 0;
	//
	//
	coinPulseWidth						= iniparser_getuint(ini, "Common:CoinPulseWidth", 		110);
	//************************
	// Clear ResetUSBFlag. If this flag is true device don't respond
	resetUsbDevice						= 0;	//iniparser_getuint(ini, "Common:ResetUsbDevice", 		0);
	//************************
	useDatabase							= iniparser_getuint(ini, "Common:UseDatabase", 			1);
	useEeprom							= iniparser_getuint(ini, "Common:useEeprom", 			1);
	LightTimeOff						= iniparser_getuint(ini, "Common:LightTimeOff",			60);

	useHWClock							= iniparser_getuint(ini, "Common:useHWClock",			0);
	useDatabaseDateTime					= iniparser_getuint(ini, "Common:useDatabaseDateTime", 	(int)((useDatabase > 0) && !(useHWClock > 0)));
	useEepromDateTime					= iniparser_getuint(ini, "Common:useEepromDateTime",	(int)(!(useHWClock > 0)));
	useEepromParams						= iniparser_getuint(ini, "Common:useEepromParams",	0);
	cardBonus							= iniparser_getuint(ini, "Common:CardBonus",			0);
	moneyBonus							= iniparser_getuint(ini, "Common:MoneyBonus",			0);
	showAppLabel						= iniparser_getuint(ini, "Common:ShowAppLabel",			0);
	useStoreBalance						= iniparser_getuint(ini, "Common:UseStoreBalance",		0);
	sprintf(ethName, "%s", iniparser_getstring(ini, "Common:EthName", "eth0"));


	netServerConfig.PortNumber 			= iniparser_getuint(ini, "NetServer:PortNumber", 		3355);
	memset(extPanelNetConfig.netServerAddr, 0, sizeof(extPanelNetConfig.netServerAddr));
	extPanelNetConfig.PortNumber 		= iniparser_getuint(ini, "NetClient:PortNumber", 		3355);
	sprintf(extPanelNetConfig.netServerAddr, "%s", iniparser_getstring(ini, "NetClient:ServerAddr", "127.0.0.1"));

	threadFlag.NetServer				= iniparser_getuint(ini, "ThreadFlag:NetServer", 		1);
	threadFlag.DataExchangeThread		= iniparser_getuint(ini, "ThreadFlag:DataExchangeThread",1);
	threadFlag.MonitorWatch				= iniparser_getuint(ini, "ThreadFlag:MonitorWatch", 	1);
	threadFlag.RFIDWatch				= iniparser_getuint(ini, "ThreadFlag:RFIDWatch", 		0);
	threadFlag.CoinWatch				= iniparser_getuint(ini, "ThreadFlag:CoinWatch", 		0);
	threadFlag.CoinPulseWatch			= iniparser_getuint(ini, "ThreadFlag:CoinPulseWatch", 	0);
	threadFlag.MoneyWatch				= iniparser_getuint(ini, "ThreadFlag:MoneyWatch", 		0);
	threadFlag.MoneyCCTalkWatch			= iniparser_getuint(ini, "ThreadFlag:MoneyCCTalkWatch", 0);
	threadFlag.ButtonWatch				= iniparser_getuint(ini, "ThreadFlag:ButtonWatch", 		0);
	threadFlag.ExtCommonThread			= iniparser_getuint(ini, "ThreadFlag:ExtCommonThread", 	1);
	threadFlag.NetClient				= iniparser_getuint(ini, "ThreadFlag:NetClient", 		0);
	threadFlag.EngineWatch				= iniparser_getuint(ini, "ThreadFlag:EngineWatch", 		1);
	threadFlag.IntCommonThread			= iniparser_getuint(ini, "ThreadFlag:IntCommonThread", 	1);
	threadFlag.TimeTickThread			= iniparser_getuint(ini, "ThreadFlag:TimeTickThread", 	1);
	if (threadFlag.DebugThread == 0)
		threadFlag.DebugThread				= iniparser_getuint(ini, "ThreadFlag:DebugThread", 		1);
	threadFlag.ThermalWatch				= iniparser_getuint(ini, "ThreadFlag:ThermalWatch", 	0);
	threadFlag.GPIOExtWatch				= iniparser_getuint(ini, "ThreadFlag:GPIOExtWatch", 	useMCPWatch & useMCP);
	threadFlag.Lcd20x4Watch				= iniparser_getuint(ini, "ThreadFlag:Lcd20x4Watch", 	0);
	threadFlag.RemoteCtrlThread			= iniparser_getuint(ini, "ThreadFlag:RemoteCtrlThread", 	0);
	threadFlag.RemoteSenderThread		= iniparser_getuint(ini, "ThreadFlag:RemoteSenderThread", 	0);
	threadFlag.ButtonMasterThread		= iniparser_getuint(ini, "ThreadFlag:ButtonMasterThread", 	0) & !threadFlag.ButtonWatch;
	threadFlag.KKMWatch					= iniparser_getuint(ini, "ThreadFlag:KKMWatch", 	0);
	threadFlag.VoiceWatch				= iniparser_getuint(ini, "ThreadFlag:VoiceWatch", 	0);
	threadFlag.RemoteCounterCtrlThread	= iniparser_getuint(ini, "ThreadFlag:RemoteCounterCtrlThread", 	0);
	threadFlag.AlienDeviceThread		= iniparser_getuint(ini, "ThreadFlag:RemoteCounterCtrlThread", 	0);
	threadFlag.OsmosThread				= iniparser_getuint(ini, "ThreadFlag:OsmosThread", 	0);
	threadFlag.ButtonTerminalThread		= iniparser_getuint(ini, "ThreadFlag:ButtonTerminalThread", 	0);
	threadFlag.VisaDeviceThread			= iniparser_getuint(ini, "ThreadFlag:VisaDeviceThread", 	0);

	debugFlag.DebugThread				= threadFlag.DebugThread;
	debugFlag.NetServer					= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:NetServer", 		0);
	debugFlag.DataExchangeThread		= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:DataExchangeThread",0);
	debugFlag.MonitorWatch				= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:MonitorWatch", 		0);
	debugFlag.RFIDWatch					= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:RFIDWatch", 		0);
	debugFlag.CoinWatch					= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:CoinWatch", 		0);
	debugFlag.CoinPulseWatch			= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:CoinPulseWatch", 	0);
	debugFlag.MoneyWatch				= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:MoneyWatch", 		0);
	debugFlag.MoneyCCTalkWatch			= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:MoneyCCTalkWatch", 0);
	debugFlag.ButtonWatch				= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:ButtonWatch", 		0);
	debugFlag.ExtCommonThread			= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:ExtCommonThread", 	0);
	debugFlag.NetClient					= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:NetClient", 		0);
	debugFlag.EngineWatch				= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:EngineWatch", 		0);
	debugFlag.IntCommonThread			= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:IntCommonThread", 	0);
	debugFlag.TimeTickThread			= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:TimeTickThread", 	0);
	debugFlag.ConsoleWatch				= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:ConsoleWatch", 		0);
	debugFlag.ThermalWatch				= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:ThermalWatch", 		0);
	debugFlag.MainWatch					= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:MainWatch", 		0);
	debugFlag.Lcd20x4Watch				= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:Lcd20x4Watch", 	0);
	debugFlag.RemoteCtrlThread			= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:RemoteCtrlThread", 	0);
	debugFlag.RemoteSenderThread		= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:RemoteSenderThread", 	0);
	debugFlag.ButtonMasterThread		= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:ButtonMasterThread", 	0);
	debugFlag.KKMWatch					= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:KKMWatch", 	0);
	debugFlag.VoiceWatch				= (debugFlag.DebugThread == 1) && threadFlag.VoiceWatch;
	debugFlag.RemoteCounterCtrlThread	= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:RemoteCounterCtrlThread", 	0);
	debugFlag.AlienDeviceThread			= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:RemoteCounterCtrlThread", 	0);
	debugFlag.OsmosThread				= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:OsmosThread", 	0);
	debugFlag.ButtonTerminalThread		= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:ButtonTerminalThread", 	0);
	debugFlag.VisaDeviceThread			= (debugFlag.DebugThread == 1) && iniparser_getuint(ini, "DebugFlag:VisaDeviceThread", 	0);

	sprintf(modbus.portName, "%s", iniparser_getstring(ini,	"Modbus:PORT", "/dev/ttyAMA0"	));
	modbus.baudRate 						= iniparser_getuint(ini, 	"Modbus:BAUND",			9600);
	modbus.dataParity 					= (char)(0xFF & iniparser_getuint(ini, 	"Modbus:PARITY",		'N'));	// ПРИНИМАЕТ "СИМВОЛ" (char) Val: 'N' 'O' 'D'
	modbus.dataBit 						= iniparser_getuint(ini, 	"Modbus:BIT",			8);
	modbus.stopBit 						= iniparser_getuint(ini, 	"Modbus:STOPBIT",		0);
	modbus.slaveCount						= iniparser_getuint(ini, 	"Modbus:SlaveCount",	0);

	sprintf(kkmParam.kkmAddr, "%s", iniparser_getstring(ini,	"KKM:ADDR", "192.168.254.222"	));
	sprintf(kkmParam.ServiceName, "%s", iniparser_getstring(ini,	"KKM:ServiceName", "Аренда автомоечного оборудования"));
	kkmParam.kkmPort 						= iniparser_getuint(ini, 	"KKM:PORT",			7778);
	kkmParam.kkmPass 						= (BYTE)iniparser_getuint(ini, 	"KKM:PASS",			30);
	kkmParam.QueryTime						= iniparser_getuint(ini, 	"KKM:QueryTime",			10000);
	kkmParam.TaxType						= iniparser_getuint(ini, 	"KKM:TaxType",			8); // ENVD
	kkmParam.LocalTime						= iniparser_getuint(ini, 	"KKM:LocalTime",			1); // ENVD
	kkmParam.MaxAmount						= iniparser_getuint(ini, 	"KKM:MaxAmount",			1001);
	kkmParam.SharedMode						= iniparser_getuint(ini, 	"KKM:SharedMode",			0);
	kkmParam.CutType						= iniparser_getuint(ini, 	"KKM:CutType",			0);

	osmosParam.gidrodynWaitTime				= iniparser_getuint(ini, 	"OSMOS:gidrodynWaitTime",			600);
	osmosParam.gidrodynTurnOnTime			= iniparser_getuint(ini, 	"OSMOS:gidrodynTurnOnTime",			60);
	osmosParam.waitOnFull					= iniparser_getuint(ini, 	"OSMOS:waitOnFull",					600);
	osmosParam.permiatLoadTime				= iniparser_getuint(ini, 	"OSMOS:permiatLoadTime",			300);
	osmosParam.waitOnEmpty					= iniparser_getuint(ini, 	"OSMOS:waitOnEmpty",				600);

	coinDiscount.coinAfterSumm				= iniparser_getuint(ini, 	"CoinDiscount:CoinAfterSumm",		0);
	coinDiscount.coinDiscountSize			= iniparser_getuint(ini, 	"CoinDiscount:CoinDiscountSize",	0);

	remoteCounterParam.DocumentCreationTime	= iniparser_getuint(ini, 	"RemoteCounterParam:DocumentCreationTime",			45);
	remoteCounterParam.PriceIN1				= iniparser_getuint(ini, 	"RemoteCounterParam:PriceIN1",						1);
	remoteCounterParam.PriceIN2				= iniparser_getuint(ini, 	"RemoteCounterParam:PriceIN2",						10);

	winterMode.winterMode				= iniparser_getuint(ini, "WinterMode:WinterMode", 		0);
	winterMode.winterDelay				= iniparser_getuint(ini, "WinterMode:WinterDelay", 		0);
	winterMode.winterWaitTime			= iniparser_getuint(ini, "WinterMode:WinterWaitTime", 	600);	// Ожидание в секундах до активации зимнего режима
	winterMode.winterOnTemperature		= iniparser_getint(ini, "WinterMode:WinterOnTemp", 	-2);
	winterMode.winterEngineWork 		= iniparser_getuint(ini, "WinterMode:WinterEngineWork", 0);
	winterMode.winterEngineWorkTime 	= iniparser_getuint(ini, "WinterMode:WinterEngineWorkTime", 10);
	winterMode.winterEngineWorkFreq 	= iniparser_getuint(ini, "WinterMode:WinterEngineWorkFreq", 5000);
	winterMode.winterEngineWorkWait 	= iniparser_getuint(ini, "WinterMode:WinterEngineWorkWait", 900);

	sprintf(databaseSettings.serverName, 	"%s", iniparser_getstring(ini, "Database:ServerName", 	"127.0.0.1"	));
	sprintf(databaseSettings.databaseName, 	"%s", iniparser_getstring(ini, "Database:databaseName", "/var/lib/firebird/2.5/data/bewash.fdb"	));
	sprintf(databaseSettings.userId, 		"%s", iniparser_getstring(ini, "Database:userId", 		"SYSDBA"));
	sprintf(databaseSettings.userPasswd, 	"%s", iniparser_getstring(ini, "Database:userPasswd", 	"masterkey"));

	sprintf(gdatabaseSettings.serverName, 	"%s", iniparser_getstring(ini, "GlobalDatabase:ServerName", 	"127.0.0.1"	));
	sprintf(gdatabaseSettings.databaseName, 	"%s", iniparser_getstring(ini, "GlobalDatabase:DatabaseName", "/var/lib/firebird/2.5/data/bewash.fdb"	));
	sprintf(gdatabaseSettings.userId, 		"%s", iniparser_getstring(ini, "GlobalDatabase:UserId", 		"SYSDBA"));
	sprintf(gdatabaseSettings.userPasswd, 	"%s", iniparser_getstring(ini, "GlobalDatabase:UserPasswd", 	"masterkey"));

	sprintf(comports[10], 	"%s", iniparser_getstring(ini, 	"Engine:PORT", 			"/dev/ttyAMA0"	));
	engine_baundRate 			= iniparser_getuint(ini, 	"Engine:BAUND",			9600);
	engine_deviceType 			= iniparser_getuint(ini, 	"Engine:DEVICE_TYPE",	3);
	valveTimeOff 				= iniparser_getuint(ini, 	"Engine:ValveTimeOff",	2);
	engine_relay 				= (DWORD)iniparser_getuint(ini, 	"Engine:Relay",	0);
	bypassCounter				= iniparser_getuint(ini, 	"Engine:BypassCounter",	1);
	bypassTimeMs				= iniparser_getuint(ini, 	"Engine:bypassTimeMs",	100);

	sprintf(thermalParam.portName, 	"%s", iniparser_getstring(ini, 	"Thermal:PORT",	""));
	thermalParam.thermalRelay	= iniparser_getuint(ini, 	"Thermal:ThermalRelay", 	1);
	thermalParam.tempOn			= iniparser_getint(ini, 	"Thermal:tempOn", 			99);
	thermalParam.tempOff		= iniparser_getint(ini, 	"Thermal:tempOff", 			99);

	dayLightSaving				= iniparser_getint(ini, 	"Light:dayLightSaving", 	0);
	dayLightSavingOnHour		= iniparser_getint(ini, 	"Light:OnHour", 			0);
	dayLightSavingOffHour		= iniparser_getint(ini, 	"Light:OffHour", 			0);

	workTimeDevice.UseWorkTime		= iniparser_getint(ini, 	"WorkTime:UseWorkTime",		0);
	workTimeDevice.StartTimeHour	= iniparser_getint(ini, 	"WorkTime:StartTimeHour",		0);
	workTimeDevice.StartTimeMinute	= iniparser_getint(ini, 	"WorkTime:StartTimeMinute",		0);
	workTimeDevice.StopTimeHour		= iniparser_getint(ini, 	"WorkTime:StopTimeHour",		23);
	workTimeDevice.StopTimeMinute	= iniparser_getint(ini, 	"WorkTime:StopTimeMinute",		59);

	lcd24Param.balStrNum = iniparser_getint(ini, 	"LCD24:BalStrNum",		0);
	lcd24Param.prgStrNum = iniparser_getint(ini, 	"LCD24:PrgStrNum",		1);
	lcd24Param.lineStrNum = iniparser_getint(ini, 	"LCD24:LineStrNum",		2);
	lcd24Param.adsStrNum = iniparser_getint(ini, 	"LCD24:AdsStrNum",		3);
	sprintf(lcd24Param.adsMessage, "%s", iniparser_getstring(ini,	"LCD24:AdsMessage", ""));

	serviceCards.prgNumber = iniparser_getuint(ini, "ServiceCards:PrgNumber", 0);
	serviceCards.pinNum = iniparser_getuint(ini, "ServiceCards:pinNum", 0);
	serviceCards.washBalance = iniparser_getuint(ini, "ServiceCards:WashBalance", 100);
	for (index=0; index < 5; index++)
	{
		sprintf(paramName, 	"ServiceCards:CardID_%d", index+1);
		serviceCards.cardId[index] = iniparser_getuint(ini, paramName, 0);
	}

	//
	// пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ
	//
	for (index=0; index < 32; index++)
	{
		sprintf(paramName, 	"PrgConfig:PRG%02d_PRICE", index);
		progPrice[index]						= iniparser_getuint(ini, paramName, 0);

		sprintf(paramName, 	"PrgConfig:PRG%02d_STOP_BP", index);
		progStopBP[index]						= iniparser_getuint(ini, paramName, 0);

		sprintf(paramName, 	"PrgConfig:PRG%02d_ENABLED", index);
		progEnabled[index]						= iniparser_getuint(ini, paramName, 0);

		sprintf(paramName, 	"PrgConfig:PRG%02d_NAME", index);
		sprintf(progName[index],		"%s", iniparser_getstring(ini, paramName, "********"	));

		sprintf(paramName, 	"PrgConfig:PRG%02d_RPM", index);
		progRPM[index]						= (DWORD)iniparser_getuint(ini, paramName, 0);

		sprintf(paramName, 	"PrgConfig:PRG%02d_RELAY", index);
		progRelay[index]						= (DWORD)iniparser_getuint(ini, paramName, 0xFFFFFFFF);

		sprintf(paramName, 	"PrgConfig:PRG%02d_RELAY_BP", index);
		progRelayBp[index]						= (DWORD)iniparser_getuint(ini, paramName, 0xFFFFFFFF);

		sprintf(paramName, 	"PrgConfig:PRG%02d_DEPENDS", index);
		progDepends[index]						= (DWORD)iniparser_getuint(ini, paramName, 0);

		sprintf(paramName, 	"PrgConfig:PRG%02d_WINTER_DELAY", index);
		progWinterDelay[index]						= (DWORD)iniparser_getuint(ini, paramName, 0);

		sprintf(paramName, 	"PrgConfig:PRG%02d_LIMIT_SUMM", index);
		progLimitSumm[index]						= (DWORD)iniparser_getuint(ini, paramName, 0);

		sprintf(paramName, 	"PrgConfig:PRG%02d_LIMIT_RELAY", index);
		progLimitRelay[index]						= (DWORD)iniparser_getuint(ini, paramName, 0xFFFFFFFF);


	}

	//
	// пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ
	//
	for (index=0; index < MONEY_COIN_TYPE_COUNT; index++)
	{
		sprintf(paramName, "Coin_Weight:Weight%02d", index);
		coinWeight.Weight[index]						= iniparser_getuint(ini, paramName, 0);
		sprintf(paramName, "Bill_Weight:Weight%02d", index);
		moneyWeight.Weight[index]						= iniparser_getuint(ini, paramName, 0);
	}

	//
	// PinConfig
	//
	for (index=0; index < 100; index++)
	{
		//
		// Load device PIN config
		sprintf(paramName, 	"Device_PIN:pinDevice_%03d", index);
		pinDevice[index]								= iniparser_getuint(ini, paramName, 0xFFFFFFFF);
		//
		// Load device ENABLED flag
		sprintf(paramName, 	"Device_Enabled:enabledDevice_%03d", index);
		enabledDevice[index]							= iniparser_getuint(ini, paramName, 0);
	}

	sprintf(visaParam.portName, 	"%s", iniparser_getstring(ini, 	"VISA:PORT", 			"/dev/ttyUSB0"	));
	visaParam.baudRate 						= iniparser_getuint(ini, 	"VISA:BAUND",			9600);
	visaParam.dataParity 					= (char)(0xFF & iniparser_getuint(ini, 	"VISA:PARITY",		'N'));	// ПРИНИМАЕТ "СИМВОЛ" (char) Val: 'N' 'O' 'D'
	visaParam.dataBit 						= iniparser_getuint(ini, 	"VISA:BIT",			8);
	visaParam.stopBit 						= iniparser_getuint(ini, 	"VISA:STOPBIT",		0);
	//--
	sprintf(visaParam.login, 	"%s", iniparser_getstring(ini, 	"VISA:Login", 			""	));
	sprintf(visaParam.password, 	"%s", iniparser_getstring(ini, 	"VISA:Password", 			""	));
	visaParam.debug							= iniparser_getuint(ini,"VISA:Debug",	0);
	//--
	visaParam.workMode						= iniparser_getuint(ini, 	"VISA:WorkMode",	0);
	visaParam.sub10Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Sub10Btn_PIN",	0);
	visaParam.sub10Btn.pinEnable			= (BYTE)(visaParam.sub10Btn.pinNum > 0);
	visaParam.add10Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Add10Btn_PIN",	0);
	visaParam.add10Btn.pinEnable			= (BYTE)(visaParam.add10Btn.pinNum > 0);
	visaParam.add50Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Add50Btn_PIN",	0);
	visaParam.add50Btn.pinEnable			= (BYTE)(visaParam.add50Btn.pinNum > 0);
	visaParam.add100Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Add100Btn_PIN",	0);
	visaParam.add100Btn.pinEnable			= (BYTE)(visaParam.add100Btn.pinNum > 0);
	visaParam.add500Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Add500Btn_PIN",	0);
	visaParam.add500Btn.pinEnable			= (BYTE)(visaParam.add500Btn.pinNum > 0);

	visaParam.payBtn.pinNum					= (BYTE)iniparser_getuint(ini, 	"VISA:PayBtn_PIN",	0);
	visaParam.payBtn.pinEnable				= (BYTE)(visaParam.payBtn.pinNum > 0);
	visaParam.cancelBtn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:CancelBtn_PIN",	0);
	visaParam.cancelBtn.pinEnable			= (BYTE)(visaParam.cancelBtn.pinNum > 0);

	visaParam.pay50Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Pay50Btn_PIN",	0);
	visaParam.pay50Btn.pinEnable			= (BYTE)(visaParam.pay50Btn.pinNum > 0);
	visaParam.pay100Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Pay100Btn_PIN",	0);
	visaParam.pay100Btn.pinEnable			= (BYTE)(visaParam.pay100Btn.pinNum > 0);
	visaParam.pay150Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Pay150Btn_PIN",	0);
	visaParam.pay150Btn.pinEnable			= (BYTE)(visaParam.pay150Btn.pinNum > 0);
	visaParam.pay200Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Pay200Btn_PIN",	0);
	visaParam.pay200Btn.pinEnable			= (BYTE)(visaParam.pay200Btn.pinNum > 0);
	visaParam.pay500Btn.pinNum				= (BYTE)iniparser_getuint(ini, 	"VISA:Pay500Btn_PIN",	0);
	visaParam.pay500Btn.pinEnable			= (BYTE)(visaParam.pay500Btn.pinNum > 0);

	sprintf(terminalParam.deviceSubnet, "%s", iniparser_getstring(ini,	"TerminalParam:DeviceSubnet", "192.168.254."));

	autoRebootParam.AutoReboot				= iniparser_getuint(ini, 	"AutoReboot:AutoReboot",	0);
	autoRebootParam.AutoRebootHour			= iniparser_getuint(ini, 	"AutoReboot:AutoRebootHour",	3);

/*
[VISA]
PORT = "/dev/ttyUSB0"
BAUND = 9600
BIT = 8
STOPBIT = 0
;--
WorkMode = 0
;--
PayBtn_PIN = 0
CancelBtn_PIN = 0
;--
Sub10Btn_PIN = 0
Add10Btn_PIN = 0
Add50Btn_PIN = 0
Add100Btn_PIN = 0
Add500Btn_PIN = 0
;--
Pay50Btn_PIN = 0
Pay100Btn_PIN = 0
Pay150Btn_PIN = 0
Pay200Btn_PIN = 0
Pay500Btn_PIN = 0
*/

	digitalKey 		= (unsigned long)iniparser_getlong(ini, 	"License:KEY1",			0xF7ED);
	digitalKey 		= (digitalKey << 16) + (unsigned long)iniparser_getlong(ini, 	"License:KEY2",			0x936C);

	printf("done\n");
	return 0;
}

bool Settings::saveConfig (char* fileName)
{
	FILE * fout;

	if ((fout = fopen(fileName, "w")) == NULL) {
		fprintf(stderr, "iniparser: cannot open %s\n", fileName);
		return -1;
	}

	iniparser_dump_ini(ini, fout);

	fflush(fout);
	fclose(fout);
	return 0;
}
