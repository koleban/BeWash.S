#ifndef _DB_H_
#define _DB_H_
class Settings;

#define BYTE				unsigned char
#define WORD				unsigned short
#define DWORD				unsigned int
#define SDWORD				int

///
/// DATABASE ERROR CODE
#define DB_OK							0
#define DB_ERROR						1
#define DB_CONNECTION_OPEN_ERROR  		2
#define DB_CONNECTION_CLOSE_ERROR		3
#define DB_CONNECTION_CLOSED			4
#define DB_QUERY_ERROR					5
#define DB_UNKNOWN_QUERY_TYPE			6
///
/// DATABASE QUERY TYPE
#define DB_QUERY_TYPE_TEST				  	  1
#define DB_QUERY_TYPE_GETRECORDCOUNT	  	  2
#define DB_QUERY_TYPE_SYSTIME				100
#define DB_QUERY_TYPE_WORKSPACE_OPEN		101
#define DB_QUERY_TYPE_WORKSPACE_CLOSE		102
#define DB_QUERY_TYPE_GET_OPENED_WORKSPACE	103
#define DB_QUERY_TYPE_GET_CASH_STORE		104
#define DB_QUERY_TYPE_GET_CARD_INFO			105
#define DB_QUERY_TYPE_SET_CARD_INFO			106
#define DB_QUERY_TYPE_GET_ENGINE_WORK_TIME	107
#define DB_QUERY_TYPE_GET_PARAM				108
#define DB_QUERY_TYPE_GET_NEW_PARAM			109
#define DB_QUERY_TYPE_GET_PARAM_ARRAY		110
#define DB_QUERY_TYPE_GET_ALL_CASH_STORE	111

///
/// DATABASE EVENT TYPE
#define DB_EVENT_TYPE_TEST					100
#define DB_EVENT_TYPE_SYSTEMSTART			101
#define DB_EVENT_TYPE_MONEY_INCOME			102
#define DB_EVENT_TYPE_THREAD_INIT			103
#define DB_EVENT_TYPE_DVC_BILL_WARNING		104
#define DB_EVENT_TYPE_DVC_BILL_ERROR		105
#define DB_EVENT_TYPE_DVC_BUTTON_INIT		106
#define DB_EVENT_TYPE_DVC_COIN_INIT			107
#define DB_EVENT_TYPE_INT_CARD_INSERTED		108
#define DB_EVENT_TYPE_INT_CARD_GONE			109
#define DB_EVENT_TYPE_INT_PROGRAM_CHANGED	110
#define DB_EVENT_TYPE_INT_MONEY_EVENT		111
#define DB_EVENT_TYPE_INT_IDKFA_EVENT		113
#define DB_EVENT_TYPE_DVC_CLOSE				114
#define DB_EVENT_TYPE_EXT_MONEY_EVENT		115
#define DB_EVENT_TYPE_EXT_SRV_ADD_MONEY		116
#define DB_EVENT_TYPE_EXT_SRV_NEW_PRG		117
#define DB_EVENT_TYPE_EXT_NEW_BUTTON		118
#define DB_EVENT_TYPE_EXT_COLL_BUTTON		119
#define DB_EVENT_TYPE_NET_RECONNECT			120
#define DB_EVENT_TYPE_NET_CONNECTED			121
#define DB_EVENT_TYPE_NET_LINKDOWN			122
#define DB_EVENT_TYPE_GPIO_EXTENDER_FAIL	123

#define DB_EVENT_TYPE_MODBUS_SLAVE_CTRL		127
#define DB_EVENT_TYPE_CARD_OUT				130
#define DB_EVENT_TYPE_COIN_OUT				131

#define DB_EVENT_TYPE_KKM_THREAD			140
#define DB_EVENT_TYPE_KKM_FN				141
#define DB_EVENT_TYPE_KKM_FN_OPEN_DOC		142
#define DB_EVENT_TYPE_KKM_ERROR				143
#define DB_EVENT_TYPE_KKM_AMOUNT_ERROR		144
#define DB_EVENT_TYPE_KKM_FN_CLOSE_DOC		145
#define DB_EVENT_TYPE_KKM_FN_CANCEL_DOC		146

#define DB_EVENT_TYPE_VISA_PAY_DOC_OK		160
#define DB_EVENT_TYPE_VISA_PAY_DOC_ERROR	161

#define DB_EVENT_TYPE_SYSTEM_LIVE			200
#define DB_EVENT_TYPE_HARDWARE_ERROR		201
#define DB_EVENT_TYPE_DVC_ENGINE_WORK_TIME	202
#define DB_EVENT_TYPE_CARD_EXT_ADD_MONEY	203		//  Внешнее пополнение карты
#define DB_EVENT_TYPE_EXT_COIN_REJECTED		204

///
/// DATABASE PARAM ID
/// Data1 - params mod
/// Data2 - params value
#define DB_PARAM_COMMON_USE_AUTOSTOP		 10
#define DB_PARAM_COMMON_COIN_AUTOSTOP		 11
#define DB_PARAM_COMMON_PRG_PRICE		 	 12
#define DB_PARAM_COMMON_PRG_RPM		 	 	 13
#define DB_PARAM_THERMAL_TEMP_ON		 	 14
#define DB_PARAM_THERMAL_TEMP_OFF		 	 15
#define DB_PARAM_ANTIFROST		 	 		 16
#define DB_PARAM_COIN_WEIGHT	 	 		 17
#define DB_PARAM_BILL_WEIGHT	 	 		 18
#define DB_PARAM_ANTIFROST_DELAY 	 		 19
#define DB_PARAM_ANTIFROST_TEMP				 20
#define DB_PARAM_ANTIFROST_ENGINE 	 		 21
#define DB_PARAM_ANTIFROST_ENGINE_TIME 		 22
#define DB_PARAM_ANTIFROST_ENGINE_FREQ 		 23
#define DB_PARAM_ANTIFROST_ENGINE_WAIT 		 24
#define DB_PARAM_COMMON_USE_RESET_USB 		 25
// DISCOUNT'S
// DATA1 - 0x----FF DISCOUNT SIZE 0xFFFF-- = DAYxMONTH
// NOTE - DICOUNT NAME
#define DB_PARAM_DISCOUNT_DAY_OF_MONTH 		 26
// DATA1 - 0xXXYY
// YY - DISCOUNT SIZE
// XX - DAY OF WEEK bit 0000001 - monday 1000000 - sunday (0x40)
// DATA2 - 0xCCDDEEFF
// CC:DD and EE:FF - discount time 00:40 and 17:46
// NOTE - DICOUNT NAME
#define DB_PARAM_DISCOUNT_PARAM 		 	 27
// DISCOUNT FOR DEPOSIT RFID CARD
// DATA1 - DISCOUNT SIZE
#define DB_PARAM_DISCOUNT_CARD_DEPOSIT	 	 28

struct DBParam_DiscountParam
{
	// DATA1
	unsigned char DiscountSize;
	unsigned char DiscountDayOfWeek;
	// DATA2
	unsigned char DiscountStartHour;
	unsigned char DiscountStartMin;
	unsigned char DiscountStopHour;
	unsigned char DiscountStopMin;
};

struct DBParam_DiscountDate
{
	unsigned char DiscountSize;
	unsigned char DiscountDay;
	unsigned char DiscountMonth;
};

struct DB_RFIDCardInfo
{
	DWORD		cardId;
	char		cardNumber[12];
	char		cardOwner[102];
	SDWORD		cardMoney;
};


class Database
{
private:
	IBPP::Database 				db;
	BYTE						isOpened;
	void 						eventId2eventText(int eventId, char* outputBuffer);
public:
	DatabaseSettings			databaseSettings;
	unsigned char				useDatabase;
	unsigned long				lastError;
	char						lastErrorMessage[1024];

	Database();
	void Init(Settings* settings);
	void Init(DatabaseSettings* dbSettings);
	int Open();
	bool IsOpened() {return isOpened;};
	int Close();

	int LogDb(time_t eventTime, int eventId, double data1, double data2, char* note);
	int Log(int eventId, double data1, double data2, char* note);
	int Query(DWORD queryType, void* queryParam, void* queryOutput);
};
#endif