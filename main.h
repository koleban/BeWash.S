//#pragma message "Compiling ..."
//#include <iostream>
//#include <queue>
//using namespace std;
#pragma GCC diagnostic ignored "-Wwrite-strings"
//#define CRYPTOPP
//#define __SECURITY_DEBUG_

#ifndef _RFID_DEVICE_CRT288K_
#define _RFID_DEVICE_CRT288K_
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringSerial.h>
#include <linux/i2c-dev.h>
#include <linux/kernel.h>       /* for struct sysinfo */
#include <sys/sysinfo.h>
#include <mcp23017.h>
#include <dirent.h>
#include <netinet/in.h>   // IPPROTO_IP
#include <net/if.h>       // IFF_*, ifreq
#include <errno.h>        // errno
#include <linux/usbdevice_fs.h>
#include <stdint.h>

#ifdef CRYPTOPP
#include "rsa.h"
#include "osrng.h"
#include "integer.h"
#include "sha.h"
#include "hex.h"
#include "filters.h"
#include "pssr.h"
#endif

#include "ibpp/ibpp.h"
#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"
#include "netserver.h"
#include "netclient.h"
#include "settings.h"
#include "status.h"
#include "db.h"
#include "debug.h"
#include "devices/engine.h"
#include "devices/engine_E9300.h"
#include "devices/engine_E8100.h"
#include "devices/engine_E8400.h"
#include "devices/engine_Emu.h"
#include "devices/relays.h"
#include "devices/sensors.h"
#include "devices/monitor.h"
#include "devices/eeprom.h"
#include "devices/rfid.h"
#include "devices/ccbill.h"
#include "devices/cctalkbill.h"
#include "devices/coin.h"
#include "devices/coinpulse.h"
#include "devices/crt288.h"
#include "devices/lcd.h"
#include "devices/remoteCtrl.h"
#include "devices/visa.h"
#include "protocols/ccnet.h"
#include "protocols/cctalk.h"
#include "protocols/max7219.h"
#include "protocols/rs232.h"
#include "protocols/rs485.h"
#include "other/queue.h"
#include "other/md5.h"
#include "tlv/tlv_box.h"

#include "drvfr/DrvFR_Conn.h"

extern "C"{
#include "SDK/PaymentController.h"
#include "SDK/Result.h"
#include "SDK/WebObject.h"
}

#define ERROR(fmt, ...) do { printf(fmt, __VA_ARGS__); return -1; } while(0)


#ifndef TRUE
#define TRUE (1==1)
#endif

#ifndef FALSE
#define FALSE (!TRUE)
#endif

#ifndef NULL
#define NULL			0
#endif

#ifndef MAX_PARAM_ARRAY_ITEM
#define MAX_PARAM_ARRAY_ITEM	999
#endif

#define DWORD			unsigned int
#define WORD			unsigned short
#define BYTE			unsigned char
#define SDWORD			int

#ifndef PIN_INPUT
#define PIN_INPUT	1
#define PIN_OUTPUT	0
#endif

#ifndef EEPROM_DELAY
#define EEPROM_DELAY	10
#endif


PI_THREAD(RFIDWatch);
PI_THREAD(CoinWatch);
PI_THREAD(CoinPulseWatch);
PI_THREAD(MoneyWatch);
PI_THREAD(BV_MoneyWatch);
PI_THREAD(EngineWatch);
PI_THREAD(ButtonWatch);

PI_THREAD(DataExchangeThread);
PI_THREAD(RFIDExchangeThread);
PI_THREAD(MonitorWatch);
PI_THREAD(ExtCommonThread);
PI_THREAD(IntCommonThread);
PI_THREAD(TimeTickThread);

PI_THREAD(NetServerThread);
PI_THREAD(NetServerClientThread);
PI_THREAD(NetClientThread);

PI_THREAD(DebugThread);
PI_THREAD(ConsoleThread);
PI_THREAD(ThermalThread);

PI_THREAD(DigitalKey);

// Other sevice threads
PI_THREAD(start_blink);
PI_THREAD(load_params_from_db);
PI_THREAD(gpioext_wd);
PI_THREAD(EepromThread);

PI_THREAD(Lcd20x4);

PI_THREAD(DiscountWatch);
PI_THREAD(ClearQueueLog);
PI_THREAD(TurnLightWatch);

PI_THREAD(RemoteCtrlWatch);
PI_THREAD(RemoteSenderWatch);
PI_THREAD(ButtonMasterWatch);
PI_THREAD(KKMWatch);
PI_THREAD(VoiceWatch);

PI_THREAD(RemoteCounterCtrlWatch);
PI_THREAD(AlienDeviceWatch);

PI_THREAD(OsmosWatch);
PI_THREAD(ButtonTerminalWatch);

PI_THREAD(VisaDeviceWatch);

// пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ
extern Settings* 		settings;
extern NetServer* 		netServer;
extern NetClient* 		netClient;
extern Database*		commonDb;

// пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ
extern Monitor*			monitor;
extern RelaySwitch* 	relay;
extern Sensors*			sensors;
extern Engine*			engine;
extern VisaDevice*		visaDevice;
#ifndef _RFID_DEVICE_CRT288K_
extern RFIDDevice* 		rfid;
#endif
#ifdef _RFID_DEVICE_CRT288K_
extern Crt288KDevice* rfid;
#endif
extern CoinDevice* 		coinDevice;
extern CoinPulseDevice* coinPulseDevice;
extern CCBillDevice* 	billDevice;
extern EEPROM* 			eeprom;
//extern CCTalkBillDevice* ccTalkBillDevice;

extern QueueArray* queueLog;
extern QueueArray* queueKkm;

extern RejectedCoinInfo rejectedCoinInfo;

extern DeviceInfo 		status;

extern int 				winterModeActive;
extern int 				winterModeEngineActive;
extern int 				exitStatus;

extern DBParam_DiscountDate		DiscountDates[MAX_PARAM_ARRAY_ITEM + 1];
extern DBParam_DiscountParam	DiscountParams[MAX_PARAM_ARRAY_ITEM + 1];

extern RemoteCtrl remoteCtrl[30];

extern int				wrkOpenedFlag;
extern time_t			wrkOpenedDateTime;

extern char idkfa[10];
extern char iddqd[50];

extern time_t prgStartTimer;
extern long winterWaitTime;
extern long winterCurrTime;

extern int lightThreadActive;

extern unsigned long gEngineFullWorkTime;

extern int Cport[30];
extern char comports[30][60];

extern unsigned long answer;
extern unsigned long keyErrorCount;
extern int licenseError;

extern float prgVer;
extern int prgBuild;
extern int globalThreadId;
extern int externalCtrl;

extern volatile bool detectInProgress;
extern int btnMasterProgress;

extern DWORD remoteCounter[30][2];
extern DWORD remoteCounterSumm[30][3];

// ФЛАГИ: ВЫПОЛНЯЕТСЯ ВНЕШНЯЯ КОМАНДА //
// Нажатие кнопки инкасации
extern int externalCmd_collectionButton;

// Количество помех по ИМПУЛЬСНЫМ ВХОДАМ
// Для отображения в режиме отладки
//
extern unsigned long warningPulseCount[4];
extern unsigned long warningPulseLength[4];

extern PayInfo payInfo;

//////////////////////
/// EEPROM PARAMETERS
///
extern DWORD eepromPrgPrice[16];
extern int 	currentPrgPriceIndex;

// GlobalDB for RFID CARD
extern Database* gDbCard;

// Global DEVICE WORK MODE
extern TDeviceWorkMode deviceWorkMode;
//
//--------------------------------------------------
//
int usbreset(char *argv);

int CheckLink(char *ifname);

void nextButton(char btn);
void nextCoin(int coin);
void load_params_from_db();

void delay(DWORD delay);
void delay_ms(DWORD delay);
void delay_us(DWORD delay);
bool getGPIOState(BYTE pinNum);
void setGPIOState(BYTE pinNum, BYTE state);
void setPinModeMy(int pinNum, int direct);

char* getbilltype(BYTE type);
char* getcointype(BYTE type);
void debugMessage(char* debugStr);

unsigned long getDeviceSerialNumber();

DWORD getCardIDFromBytes(BYTE* card1);
bool compareCardNumber(BYTE* card1, BYTE* card2);
bool emptyCardNumber(BYTE* card1);
BYTE getByteByNum(DWORD valDword, BYTE numByte);
bool getCardInfo(BYTE* cardNumberBytes, DB_RFIDCardInfo* cardInfo);
bool setCardInfo(BYTE* cardNumberBytes, DB_RFIDCardInfo* cardInfo);
bool setCardMoney(BYTE* cardNumberBytes, SDWORD cardMoney);

ssize_t netSendData(SDWORD sock, BYTE cmd, BYTE* data, WORD dataSize);
int netReadData(SDWORD sock, BYTE* data, WORD dataSize);

void cur_gotoxy(int x,int y);
void cur_back(int count);
void cur_save();
void cur_load();
void cur_saveattr();
void cur_loadattr();
void term_getwindowsize(int *x, int *y);
void term_setattr(int attr);
void term_clear();
void term_eraseendline();

long get_uptime();
long get_prguptime();
int mygetch( );

void commonDevice_TurnLight(bool flag);
void commonDevice_TurnAntiFrost();

pid_t proc_find(const char* name);
int getIpList( unsigned int* addr_array, int arr_size);

DWORD GetTimeToEeprom(time_t currentTime);
time_t GetTimeFromEeprom(DWORD date_time_eeprom);

void cp2utf( char* str, char* res );

typedef enum
{
	Idle = 1,
	WaitSelectDevice = 2,
	SendingCommand = 3,
	PrintCheck = 4,
	CollectionModeWork = 5
} TBtnMasterProgress;

#ifdef CRYPTOPP
int char2int(char input);
void hex2bin(const char* src, byte* target);
char* bin2hex(byte* buffer, int length);
int check(char* keyFileName);
#endif
