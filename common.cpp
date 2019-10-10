#include "main.h"

#define NET_SOCKET_TIMEOUT			100

Settings* 	settings;
NetServer* 	netServer;
NetClient* 	netClient;

Database* commonDb;

////////////////////////////////////////////////////////////
///
/// EEPROM PARAMETERS
///

/// PRG PRICE
DWORD	eepromPrgPrice[16];
int 	currentPrgPriceIndex;
////////////////////////////////////////////////////////////

// Устройства
Monitor*	monitor;
RelaySwitch* relay;
Sensors*	sensors;
Engine*		engine;
#ifndef _RFID_DEVICE_CRT288K_
RFIDDevice* 		rfid;
#endif
#ifdef _RFID_DEVICE_CRT288K_
Crt288KDevice* rfid;
#endif
CoinDevice* 	coinDevice;
CoinPulseDevice* 	coinPulseDevice;
CCBillDevice* 	billDevice;
//CCTalkBillDevice* ccTalkBillDevice;

RejectedCoinInfo rejectedCoinInfo;

time_t prgStartTimer;					// Время запуска программы

int lightThreadActive = 0;

int winterModeActive;
int winterModeEngineActive;

DWORD remoteCounter[30][2];
DWORD remoteCounterSumm[30][3];

int externalCmd_collectionButton;
int dayLightWork = 0;

TDeviceWorkMode deviceWorkMode;

RemoteCtrl remoteCtrl[30];

char str[255];

// 					1432151
char idkfa[10] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x00, 0x38, 0x39};
char iddqd[50];

DeviceInfo status;

long winterWaitTime;
long winterCurrTime;

PayInfo payInfo;

int CheckLink(char *ifname) {
    int state = -1;
    int socId = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socId < 0) ERROR("Socket failed. Errno = %d\n", errno);

    struct ifreq if_req;
    (void) strncpy(if_req.ifr_name, ifname, sizeof(if_req.ifr_name));
    int rv = ioctl(socId, SIOCGIFFLAGS, &if_req);
    close(socId);

    if ( rv == -1) ERROR("Ioctl failed. Errno = %d\n", errno);

    return (if_req.ifr_flags & IFF_UP) && (if_req.ifr_flags & IFF_RUNNING);
}

void nextButton(char btn)
{
	if ((idkfa[6] == btn) || (btn == 0x30)) return;
	memcpy(&idkfa[0], &idkfa[1], 6);
	idkfa[6] = btn;
	idkfa[7] = 0x00;
}

void nextCoin(int coin)
{
	sprintf(iddqd, "%s%d", iddqd, coin);
	if (strlen(iddqd) >= 10)
		sprintf(iddqd, "%s", &iddqd[strlen(iddqd)-10]);
}

void delay(DWORD delay)
{
	sleep(delay);
}

void delay_ms(DWORD delay)
{
	usleep(delay*1000);
}

void delay_us(DWORD delay)
{
	usleep(delay);
}

bool getGPIOState(BYTE pinNum)
{
	if (pinNum == 0xFF) return 1;
	if ((settings->commonParams.raspRev == 4) && (pinNum>20) && (pinNum<100)) return false;
	return digitalRead(pinNum);
}

void setGPIOState(BYTE pinNum, BYTE state)
{
	if ((settings->commonParams.raspRev == 4) && (pinNum>20) && (pinNum<100)) return;
	if (pinNum != 0xFF)
	{
		if (digitalRead(pinNum) != state)
			digitalWrite(pinNum, state);
	}
}

void setPinModeMy(int pinNum, int direct)
{
	if ((settings->commonParams.raspRev == 4) && (pinNum>20) && (pinNum<100)) return;

	if (direct == 1)
		pinMode(pinNum, INPUT);
	else
		pinMode(pinNum, OUTPUT);
	delay_ms(2);
}

char* getcointype(BYTE type)
{
	memset(str, 0, 100);
	switch(type)
    {
    	default:
    		sprintf(str, "Unknown bill type!!!");
    		break;
		case 1:
		case 2:
    		sprintf(str, "0.50 RUR   [%2d]", type);
			break;
		case 3:
		case 4:
    		sprintf(str, "1 RUR   [%2d]", type);
			break;
		case 5:
		case 6:
    		sprintf(str, "2 RUR   [%2d]", type);
			break;
		case 7:
		case 8:
    		sprintf(str, "5 RUR   [%2d]", type);
			break;
		case 9:
		case 10:
    		sprintf(str, "10 RUR   [%2d]", type);
			break;
    }
    return &str[0];
}

char* getbilltype(BYTE type)
{
	memset(str, 0, 100);
	switch(type)
    {
    	default:
    		sprintf(str, "Unknown bill type!!!");
    		break;
		case 2:
    		sprintf(str, "10 RUR   [%2d]", type);
			break;
		case 3:
    		sprintf(str, "50 RUR   [%2d]", type);
			break;
		case 4:
    		sprintf(str, "100 RUR   [%2d]", type);
			break;
		case 5:
    		sprintf(str, "500 RUR   [%2d]", type);
			break;
		case 6:
    		sprintf(str, "1000 RUR   [%2d]", type);
			break;
    }
    return &str[0];
}

DWORD getCardIDFromBytes(BYTE* card1)
{
	unsigned long long result = 0;
	int index = 0;
	for (index = 2; index < 6; index++)
		result = (result << 8) + *(card1+index);
	return (DWORD)result;
}
// 0 - is equivalent
// 1 - is different
bool compareCardNumber(BYTE* card1, BYTE* card2)
{
	int result = 0;
	int index = 0;
	for (index = 0; index < 6; index++)
		result |= (*(card1+index) - *(card2+index));
	return (result != 0);
}

// 0 - is empty
// 1 - is not empty
bool emptyCardNumber(BYTE* card1)
{
	int result = 0;
	int index = 0;
	for (index = 0; index < 6; index++)
		result |= *(card1+index);
	return (result == 0);
}

BYTE getByteByNum(DWORD valDword, BYTE numByte)
{
	if (numByte > 3) return 0;
	return ((BYTE)((valDword >> (8*numByte)) & 0xFF));
}

bool getCardInfo(BYTE* cardNumberBytes, DB_RFIDCardInfo* cardInfo)
{
	bool result = 0;
//	Database* gDbCard = new Database();
//	gDbCard->Init(&settings->gdatabaseSettings);
	if (gDbCard->Open())
	{
		printf("  ===> GLOBAL IB ERROR: %s\n", gDbCard->lastErrorMessage);
		return false;
	}

	DWORD cardNumber = getCardIDFromBytes(cardNumberBytes);
	printf("[DEBUG] GDB_CARD: Get card info\n");
	if (gDbCard->Query(DB_QUERY_TYPE_GET_CARD_INFO, &cardNumber, cardInfo))
	{
		printf("  ===> GLOBAL IB ERROR: %s\n", gDbCard->lastErrorMessage);
	}

	printf("[%lu]: ID: %08X Money: %4d\n", cardNumber, cardInfo->cardId, cardInfo->cardMoney);

//	gDbCard->Close();

	return result;
}

bool setCardInfo(BYTE* cardNumberBytes, DB_RFIDCardInfo* cardInfo)
{
	bool result = 0;
/*
	DatabaseQuery query;
	query.queryData = cardInfo;
	cardInfo->cardId = getCardIDFromBytes(cardNumberBytes);
	query.queryType = 0;//QUERY_TYPE_SET_CARD_INFO;
//	commonDb->Ping(1);
//	result = commonDb->Query(&query);
*/
	return result;
}

bool setCardMoney(BYTE* cardNumberBytes, SDWORD cardMoney)
{
	bool result = 0;
//	Database* gDbCard = new Database();
//	gDbCard->Init(&settings->gdatabaseSettings);
	if (gDbCard->Open())
	{
		printf("  ===> GLOBAL IB ERROR: %s\n", gDbCard->lastErrorMessage);
		return false;
	}

	DWORD cardNumber = getCardIDFromBytes(cardNumberBytes);
	DB_RFIDCardInfo* cardInfo = new DB_RFIDCardInfo;
	printf("[DEBUG] GDB_CARD: Set card money\n");
	cardInfo->cardId = cardNumber;
	cardInfo->cardMoney = cardMoney;

	if (cardMoney != 0)
		if (gDbCard->Query(DB_QUERY_TYPE_SET_CARD_INFO, cardInfo, NULL))
		{
			printf("  ===> GLOBAL IB ERROR: %s\n", gDbCard->lastErrorMessage);
		}

	printf("[%d]: ID: %08X Money: %4d\n", cardInfo->cardId, cardInfo->cardId, cardInfo->cardMoney);

//	gDbCard->Close();

	return result;
}

ssize_t netSendData(SDWORD sock, BYTE cmd, BYTE* data, WORD dataSize)
{
	ssize_t result;
    BYTE* buffer = (BYTE*)malloc(5+dataSize);
    *(buffer+0) 			= 0x02;
    *(buffer+1) 			= 0xFE;
    *((WORD*)(buffer+2)) 	= 5+dataSize;
    *(buffer+4) 			= cmd;
    memcpy(buffer+5, data, dataSize);

    result = send(sock, buffer, 5+dataSize, 0);

    free((void*)buffer);
    return result;
}

int netReadData(SDWORD sock, BYTE* data, WORD dataSize)
{
	int index = 0;
	int bytes = 0;
	WORD timeout = 0;
	memset(data, 0, dataSize);

	while (timeout < NET_SOCKET_TIMEOUT)
	{
		bytes = recv(sock, (data+index), dataSize-index, MSG_DONTWAIT);
		if (bytes < 1) { timeout++; delay_ms(1); continue; }
		index += bytes;
		timeout = 0;
	}

	if ((timeout == NET_SOCKET_TIMEOUT) && (index < 1)) return -1;

	return index;
}

//
//	Terminal function
//
void cur_gotoxy(int x,int y)
{
    printf("\x1b[%d;%df",y,x);
}

void cur_back(int count)
{
    printf("\x1b[%dD",count);
}

void cur_save()
{
    printf("\x1b[s");
}

void cur_load()
{
    printf("\x1b[u");
}

void cur_saveattr()
{
    printf("\x1b%c", '7');
}

void cur_loadattr()
{
    printf("\x1b%c", '8');
}

void term_getwindowsize(int *x, int *y)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    *y = w.ws_row;
    *x = w.ws_col;
}

void term_setattr(int attr)
{
   	printf("\x1b[%dm", attr);
}

void term_clear()
{
    printf("\x1b[2J");
}

void term_eraseendline()
{
   	printf("\x1b[K");
}

long get_uptime()
{
    struct sysinfo s_info;
    int error;
    error = sysinfo(&s_info);
    if(error != 0)
    {
        printf("code error = %d\n", error);
    }
    return s_info.uptime;
}

long get_prguptime()
{
	time_t currTime = time(NULL);
	struct tm* timeInfo;
	timeInfo = localtime(&currTime);
	return (long)(timeInfo->tm_hour*3600+timeInfo->tm_min*60+timeInfo->tm_sec);
}

int mygetch( )
{
	struct termios oldt,
	newt;
	int ch;
	ch = getchar();
	return ch;
}
/////////////////////////////////////////////////////////////////////////////

void commonDevice_TurnLight(bool flag)
{
	Settings* 		settings 	= Settings::getInstance();
	int pinNum = settings->getPinConfig(DVC_RELAY_LIGHT, 1);
	if ((pinNum == 0xFF) || (pinNum == 0x00)) return;
	if (settings->dayLightSaving == 1)
	{
		time_t currentTime;
		struct tm localTime;
		time(&currentTime);
		localTime = *localtime(&currentTime);
		if ((localTime.tm_hour >= settings->dayLightSavingOffHour) && (localTime.tm_hour <= settings->dayLightSavingOnHour))
		{
			if (dayLightWork == 1) printf("[LIGHT] Day light saving [h_curr: %d h_off: %d h_on: %d]\n", localTime.tm_hour, settings->dayLightSavingOffHour, settings->dayLightSavingOnHour);
			dayLightWork = 0;
		}
		else
		{
			if (dayLightWork == 0) printf("[LIGHT] Day light saving OFF [h_curr: %d h_off: %d h_on: %d]\n", localTime.tm_hour, settings->dayLightSavingOffHour, settings->dayLightSavingOnHour);
			dayLightWork = 1;
		}
	}
	else
		dayLightWork = 1;


	if (flag)
	{
		if (dayLightWork)
			setGPIOState(pinNum, 1);
	}
	else
	{
//		printf("[DEBUG] Start thread -> Turn light OFF\n");
		if (lightThreadActive == 0)
		{
			lightThreadActive  = 1;
			piThreadCreate(TurnLightWatch);
		}
	}
}

void commonDevice_TurnAntiFrost()
{
	/// ********************************************** ///
	/// Control anti-frost mode
	/// If pin DVC_RELAY_ANTIFROST is disable then return and don't work
	/// If pin DVC_RELAY_ANTIFROST is enabled
	/// and PIN number DVC_RELAY_ANTIFROST equ 0, then
	/// Prg relay on program STOP0 = Prg relay on program STOP1
	/// with other case turn relay Anti-Frost
	///
	///

	Settings* 		settings 	= Settings::getInstance();
	if (settings->getEnabledDevice(DVC_RELAY_ANTIFROST) == 0) return;
	int pinNum = settings->getPinConfig(DVC_RELAY_ANTIFROST, 1);
	if (pinNum >= 0x7F) return;
	if (pinNum == 0x00)
	{
		settings->progRelay[0] = settings->progRelay[1];
		settings->progRelayBp[0] = settings->progRelayBp[1];
	}
	else
	{
		setPinModeMy(pinNum, PIN_OUTPUT);
		setGPIOState(pinNum, winterModeActive);
	}
}

pid_t proc_find(const char* name)
{
    DIR* dir;
    struct dirent* ent;
    char* endptr;
    char buf[512];

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {
        /* if endptr is not a null character, the directory is not
         * entirely numeric, so ignore it */
        long lpid = strtol(ent->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        /* try to open the cmdline file */
        snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
        FILE* fp = fopen(buf, "r");

        if (fp) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                /* check the first token in the file, the program name */
                char* first = strtok(buf, " ");
                if (!strcmp(first, name)) {
                    fclose(fp);
                    closedir(dir);
                    return (pid_t)lpid;
                }
            }
            fclose(fp);
        }

    }

    closedir(dir);
    return -1;
}

int chechIniSignature(dictionary* iniFile, char* signature)
{
	int bfSize = 1024*255;
	char* strings = (char*)malloc(bfSize);
	memset (strings, 0, bfSize);
	iniparser_dump_array(iniFile, strings, bfSize);
	for (int i=0; i<1024; i++)
		if (memcmp(strings+i*255, (void*)"[license:lickey]", 16) == 0)
			memset(strings+i*255, 0, 255);
	unsigned long md5Hash[4];
	MD5One((unsigned char*)strings, bfSize, (unsigned char*)md5Hash);
	free((void*)strings);
	char* mySign = (char*)malloc(sizeof(md5Hash)+1);
	sprintf(mySign, "%08X%08X%08X%08X",
			md5Hash[0],
			md5Hash[1],
			md5Hash[2],
			md5Hash[3]);
	return strcmp(signature, mySign);
}

int getIpList( unsigned int* addr_array, int arr_size)
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    int counter = 0;
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr) continue;
        if (counter >= arr_size) continue;
        if (ifa->ifa_addr->sa_family == AF_INET)
			addr_array[counter++]=*((unsigned int*)&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr);
    }

    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);

    return counter;
}

DWORD GetTimeToEeprom(time_t currentTime)
{
	struct tm localTime;
	localTime = *localtime(&currentTime);
	DWORD date_time_eeprom = (localTime.tm_year << 24) + ((localTime.tm_mon+1) << 20) +
  					(localTime.tm_mday << 15) + (localTime.tm_hour << 10) + (localTime.tm_min << 4);
	return date_time_eeprom;
}

time_t GetTimeFromEeprom(DWORD date_time_eeprom)
{
	int day = (date_time_eeprom >> 15) & 0x1F;
	int month = (date_time_eeprom >> 20) & 0x0F;
	int year = (date_time_eeprom >> 24);
	int hour = (date_time_eeprom >> 10) & 0x1F;
	int minute = (date_time_eeprom >> 4) & 0x3F;
	time_t currentTime;
	struct tm localTime;
	localTime.tm_hour = hour; localTime.tm_min = minute; localTime.tm_sec = 0;
	localTime.tm_mon = month-1;  localTime.tm_mday = day; localTime.tm_year = year;
	currentTime = mktime(&localTime);
	return currentTime;
}

void cp2utf( char* str, char* res ) {
	static const long utf[ 256 ] = {
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
		31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,
		59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,
		87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,
		111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,1026,1027,8218,
		1107,8222,8230,8224,8225,8364,8240,1033,8249,1034,1036,1035,1039,1106,8216,8217,
		8220,8221,8226,8211,8212,8250,8482,1113,8250,1114,1116,1115,1119,160,1038,1118,1032,
		164,1168,166,167,1025,169,1028,171,172,173,174,1031,176,177,1030,1110,1169,181,182,
		183,1105,8470,1108,187,1112,1029,1109,1111,1040,1041,1042,1043,1044,1045,1046,1047,
		1048,1049,1050,1051,1052,1053,1054,1055,1056,1057,1058,1059,1060,1061,1062,1063,
		1064,1065,1066,1067,1068,1069,1070,1071,1072,1073,1074,1075,1076,1077,1078,1079,
		1080,1081,1082,1083,1084,1085,1086,1087,1088,1089,1090,1091,1092,1093,1094,1095,
		1096,1097,1098,1099,1100,1101,1102,1103
	};

	int cnt = strlen( str ),
	i = 0, j = 0;

	for(; i < cnt; ++i ) {
		long c = utf[ (unsigned char) str[ i ] ];
		if( c < 0x80 ) {
			res[ j++ ] = c;
		}
		else if( c < 0x800 ) {
			res[ j++ ] = c >> 6 | 0xc0;
			res[ j++ ] = c & 0x3f | 0x80;
		}
		else if( c < 0x10000 ) {
			res[ j++ ] = c >> 12 | 0xe0;
			res[ j++ ] = c >> 6 & 0x3f | 0x80;
			res[ j++ ] = c & 0x3f | 0x80;
		}
	}
	res[ j ] = '\0';
}