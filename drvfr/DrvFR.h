#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma once
#ifndef _DRVFR_H
#define _DRVFR_H

#include <time.h>

#define DWORD			unsigned int
#define WORD			unsigned short
#define BYTE			unsigned char
#define SDWORD			int

void delay(DWORD delay);
void delay_ms(DWORD delay);
void delay_us(DWORD delay);

namespace DriverFR
{

#ifndef MAX_LEN
#define MAX_LEN 1024
#endif

/****************************************************
 * Shtrih-M  connamds                               *
 ****************************************************/

#define DUMP_REQUEST				0x01
#define GET_DATA					0x02
#define INTERRUPT_DATA_STREAM		0x03
#define GET_ECR_STATUS				0x11
#define PRINT_WIDE_STRING			0x12
#define BEEP						0x13
#define SET_EXCHANGE_PARAM			0x14
#define GET_EXCHANGE_PARAM			0x15
#define RESET_SETTINGS				0x16
#define PRINT_STRING				0x17
#define PRINT_DOCUMENT_TITLE		0x18
#define TEST						0x19
#define GET_CASH_REG				0x1A
#define GET_OPERATION_REG			0x1B
#define WRITE_LICENSE				0x1C
#define READ_LICENSE				0x1D
#define WRITE_TABLE					0x1E
#define READ_TABLE					0x1F
#define SET_POINT_POSITION			0x20
#define SET_TIME					0x21
#define SET_DATE					0x22
#define CONFIRM_DATE				0x23
#define INIT_TABLE					0x24
#define CUT_CHECK					0x25
#define RESET_SUMMARY				0x27
#define OPEN_DRAWER					0x28
#define FEED_DOCUMENT				0x29
#define EJECT_SLIP_DOC				0x2A
#define INTERRUPT_TEST				0x2B
#define PRINT_OPERATION_REG			0x2C
#define GET_TABLE_STRUCT			0x2D
#define GET_FIELD_STRUCT			0x2E
#define PRINT_STRING_WITH_FONT			0x2F
#define PRINT_REPORT_WITHOUT_CLEANING	0x40
#define PRINT_REPORT_WITH_CLEANING	0x41
#define CASH_INCOME					0x50
#define CASH_OUTCOME				0x51
#define SET_SERIAL_NUMBER			0x60
#define INIT_FM						0x61
#define GET_FM_RECORDS_SUM			0x62
#define GET_LAST_FM_RECORD_DATE		0x63
#define GET_RANGE_DATES_AND_SESSIONS	0x64
#define FISCALIZATION				0x65
#define FISCAL_REPORT_FOR_DATES_RANGE	0x66
#define FISCAL_REPORT_FOR_SESSION_RANGE	0x67
#define INTERRUPT_FULL_REPORT		0x68
#define GET_FISCALIZATION_PARAMETERS	0x69
#define FISCAL_PRINT_SLIP_DOC		0x70
#define PRINT_SLIP_DOC				0x71
#define SALE						0x80
#define BUY							0x81
#define RETURN_SALE					0x82
#define RETURN_BUY					0x83
#define STORNO						0x84
#define CLOSE_CHECK					0x85
#define DISCOUNT					0x86
#define CHARGE						0x87
#define CANCEL_CHECK				0x88
#define CHECK_SUBTOTAL				0x89
#define STORNO_DISCOUNT				0x8A
#define STORNO_CHARGE				0x8B
#define OPEN_CHECK					0x8D
#define DOZE_OIL_CHECK				0x90
#define SUMM_OIL_CHECK				0x91
#define CORRECTION					0x92
#define SET_DOZE_IN_MILLILITERS		0x93
#define SET_DOZE_IN_MONEY			0x94
#define OIL_SALE					0x95
#define STOP_RK						0x96
#define LAUNCH_RK					0x97
#define RESET_RK					0x98
#define RESET_ALL_TRK				0x99
#define SET_RK_PARAMETERS			0x9A
#define GET_LITER_SUM_COUNTER		0x9B
#define GET_CURRENT_DOZE			0x9E
#define GET_RK_STATUS				0x9F
#define ECT_DEP_DATE_REP			0xA0
#define ECT_DEP_SHIFT_REP			0xA1
#define ECT_SHIFT_DATE_REP			0xA2
#define ECT_SHIFT_SHIFT_REP			0xA3
#define ECT_SHIFT_TOTAL				0xA4
#define ECT_PAY_DOC_BY_NUMBER		0xA5
#define ECT_BY_SHIFT_NUMBER			0xA6
#define ECT_REPORT_INTR				0xA7
#define CONTINUE_PRINTING			0xB0
#define LOAD_LINE_DATA				0xC0
#define DRAW						0xC1
#define PRINT_BARCODE				0xC2
#define FN_OPEN_SESSION				0xE0
#define GET_DEVICE_METRICS			0xFC
#define CTRL_ADD_DEVICE				0xFD
#define FN_CANCEL_CHECK				0xFF08
#define FN_GET_STATUS				0xFF01
#define FN_CLOSE_CHECK_EX			0xFF45
#define FN_OPERATION				0xFF46
#define FN_FNGETINFOEXCHANGESTATUS	0xFF39
#define FN_FNREQUESTFISCALDOCUMENTTLV	0xFF3A
#define FN_FNREADFISCALDOCUMENTTLV	0xFF3B

#define	TLV_DATA_FN				1041	// ФН
#define	TLV_DATA_RNKKT			1037	// РН ККТ
#define	TLV_DATA_INN			1018	// ИНН
#define	TLV_DATA_FD				1040	// ФД
#define	TLV_DATA_DATE_TIME		1012	// ДАТА И ВРЕМЯ
#define	TLV_DATA_FP				1077	// ФП
#define	TLV_DATA_SESSION		1038	// СМЕНА
#define	TLV_DATA_DOCNUM			1042	// НОМЕР ЧЕКА ЗА СМЕНУ
#define	TLV_DATA_DOCTYPE		1054	// ПРИЗНАК РАСЧЕТА (Покупка и т.д.)
#define	TLV_DATA_SUMM			1020	// ИТОГ
#define	TLV_DATA_DEVNUMBER		1036	// НОМЕР АВТОМАТА
#define	TLV_DATA_SUMM_CASH		1031	// СУММА НАЛИЧНЫЕ
#define	TLV_DATA_SUMM_CARD		1081	// СУММА КАРТА
#define	TLV_DATA_FNS_LINK		1060	// САЙТ ФНС
#define	TLV_DATA_ADDR_DOC		1187	// МЕСТО РАСЧЕТА
#define	TLV_DATA_FFD_VER		1209	// ВЕРСИЯ ФФД (2 = 1.05)
#define	TLV_DATA_SUMM_WO_NDS	1105	// СУММА БЕЗ НДС
#define	TLV_DATA_USERNAME		1048	// НАИМЕНОВАНИЕ ПОЛЬЗОВАТЕЛЯ
#define	TLV_DATA_USERADDR		1009	// АДРЕС РАСЧЕТОВ
#define	TLV_DATA_TAX_TYPE		1055	// СИСТЕМА НАЛОГООБЛАЖЕНИЯ (2 = УСН Доходы)

#define false			0
#define true			1

#define ENQ			0x05
#define STX			0x02
#define ACK			0x06
#define NAK			0x15

class DrvFR_Conn;

enum TCheckType
{
	Sale = 1,
	ReturnSale = 2,
	Purchase = 3,
	RerurnPurchase = 4
};

enum TFNDocType
{
	RegistrationReport 			= 1,	// Отчёт о регистрации
	OpenSessionReport 			= 2,	// Отчёт об открытии смены
	PaymentCheck				= 3,	// Кассовый чек
	DocBSO						= 4,	// БСО
	CloseSessionReport			= 5,	// Отчёт о закрытии смены
	CloseFNReport				= 6,	// Отчёт о закрытии фискального накопителя
	OperatorConfirm				= 7,	// Подтверждение оператора
	RegistrationPrmChangeReport = 11, 	// Отчет об изменении параметров регистрации
	MoneyStateReport			= 21,	// Отчет о состоянии расчетов
	CorrectionPaymentCheck 		= 31,	// Кассовый чек коррекции
	DocBSOCorrection			= 41    // Бланк строгой отчетности коррекции
};

enum TTaxType
{
	NoNds = 0,
	Nds18 = 1,
	Nds10 = 2,
	Nds0 = 3,
	NoNds2 = 4,
	Nds18_118 = 5,
	Nds10_110 = 6
};

enum TPaymentTypeSign
{
	Prepayment100 = 1,          // Предоплата 100%
	PartialPayment = 2,         // Частичная оплата
	Prepaid = 3,                // Аванс
	Payment = 4,                // Полный расчет
	PartialPaymentCredit = 5,   // Частичный расчет и Кредит
	Credit = 6,   // Передача в кредит
	PaidCredit = 7    // Оплата кредита
};


enum TPaymentItemSign
{
	Ware = 1,           //  Товар
	ExcisableWare = 2,  //  Подакцизный товар
	Work = 3,           //  Работа
	Service = 4,        //  Услуга
	Bet = 5,            //  Ставка азартной игры
	WinningBet = 6,     //  Выигрыш азартной игры
	LotteryTicket = 7,  //  Лотерейный билет
	WinningTicket = 8,  //  Выигрыш лотереи
	ProvisionRIDs = 9,  //  Предоставление РИД
	Paymant = 10,       //  Платеж
	CompoundSubject = 11,   // Составной предмет расчета
	AnotherSubject = 12 // Иной предмет расчета
};

enum TDocumentTaxType
{
	Nds = 1,
	Tax6 = 2,
	Tax15 = 4,
	Envd = 8,
	Eshn = 16,
	TaxPatent = 32
};

// GetECRStatus
// ПолучитьСостояниеККМ
//
enum TECRMode
{
	WorkMode = 0,	//	Принтер в рабочем режиме
	DataInProgress = 1,	//	Выдача данных
	OpenWorkspace = 2,	//	Открытая смена, 24 часа не кончились
	OpenWorkspace24h = 3,	//	Открытая смена, 24 часа кончились
	CloseWorkspace = 4,	//	Закрытая смена
	Blocked_WrongPasswordTaxInspector = 5,	//	Блокировка по неправильному паролю налогового инспектора
	WaitingDateConfirm = 6,	//	Ожидание подтверждения ввода даты
	DigitPointChangeRight = 7,	//	Разрешение изменения положения десятичной точки
	OpenedDocument = 8,	//	Открытый документ
	TechResetEnableMode = 9,	//	Режим разрешения технологического обнуления
	TestPrinting = 10, 	//	Тестовый прогон
	PrintingFNReport = 11, 	//	Печать полного фискального отчета
	PrintingEKLZReport = 12, 	//	Печать длинного отчета ЭКЛЗ
	WorkWithFNDocument = 13, 	//	Работа с фискальным подкладным документом
	PrintFNDocument = 14, 	//	Печать подкладного документа
	WorkFNDocumentDone = 15 	//	Фискальный подкладной документ сформирован
};

enum TECRAdvancedMode
{
	DeviceOK = 0,               // Бумага есть. Ждет команды
	NoPapper = 1,               // Нет бумаги. Все ОК
	NoPapperError = 2,          // Нет бумаги состояние ОШИБКА
	WaitPrintContinue = 3,      // После ошибки ждет команду ПродолжитьПечать
	Printing = 4,               // Печатает длинный отчет. Ожидайте ...
	PrintingFNDocument = 5      // Печатает фискальный чек. Ожидайте ...
};

class DrvFR
{
public:
	DrvFR_Conn* conn = NULL;
	int Connected;

	int BatteryCondition;
	int BaudRate;
	char BarCode[MAX_LEN];
	double Change;
	int CheckIsClosed;
	int CheckIsMadeOut;
	int CheckType;
	int ComNumber;
	/*
		Тип подключения
		0 Локально
		1 Сервер ККМ (TCP)
		2 Сервер ККМ (DCOM)
		3 ESCAPE
		4 Не используется
		5 Эмулятор
		6 Подключение через ТСР-сокет
	*/
	int ConnectionType;
	double ContentsOfCashRegister;
	int ContentsOfOperationRegister;
	int CurrentDozeInMilliliters;
	double CurrentDozeInMoney;
	int CutType;
	unsigned char DataBlock[32];
	int DataBlockNumber;
	int FontType;
	struct tm Date;
	int Department;
	int DeviceCode;
	char DeviceCodeDescription[MAX_LEN];
	double DiscountOnCheck;
	char DocumentName[32];
	int DocumentNumber;
	int DocumentType;
	int DataLength;
	int OpenDocumentNumber;
	int DozeInMilliliters;
	double DozeInMoney;
	int DrawerNumber;
	int ECRAdvancedMode;
	char ECRAdvancedModeDescription[MAX_LEN];
	int ECRBuild;
	int ECRMode;
	int ECRMode8Status;
	char ECRModeDescription[MAX_LEN];
	struct tm ECRSoftDate;
	char ECRSoftVersion[MAX_LEN];
	int EKLZIsPresent;
	int EmergencyStopCode;
	char EmergencyStopCodeDescription[MAX_LEN];
	char FieldName[MAX_LEN];
	int FieldNumber;
	int FieldSize;
	int FieldType;
	int FirstLineNumber;
	struct tm FirstSessionDate;
	int FirstSessionNumber;
	int FM1IsPresent;
	int FM2IsPresent;
	int FMBuild;
	int FMOverflow;
	struct tm FMSoftDate;
	char FMSoftVersion[MAX_LEN];
	int FreeRecordInFM;
	int FreeRegistration;
	char INN[15];
	char IPAddress[20];
	int JournalRibbonIsPresent;
	int JournalRibbonOpticalSensor;
	int JournalRibbonLever;
	int KPKNumber;
	int LastFMRecordType;
	int LastLineNumber;
	struct tm LastSessionDate;
	int LastSessionNumber;
	char License[6];
	int LicenseIsPresent;
	int LidPositionSensor;
	int LogicalNumber;
	int LineNumber;
	unsigned char LineData[40];
	int MAXValueOfField;
	int MessageCount;
	int MessageState;
	int MINValueOfField;
	int Motor;
	char NameCashReg[MAX_LEN];
	char NameOperationReg[MAX_LEN];
	int NewPasswordTI;
	int OperatorNumber;
	int Password;
	int PaymentTypeSign;
	int PaymentItemSign;
	int ProtocolType;
	int Pistol;
	int PointPosition;
	int PortNumber;
	double Price;
	double Quantity;
	int ReceiptRibbonIsPresent;
	int ReceiptRibbonOpticalSensor;
	int ReceiptRibbonLever;
	int RegisterNumber;
	int ReportType;
	int RegistrationNumber;
	int ResultCode;
	char ResultCodeDescription[MAX_LEN];
	int RKNumber;
	char RNM[MAX_LEN];
	int RoughValve;
	int RowNumber;
	int RoundingSumm;
	int RunningPeriod;
	char SerialNumber[MAX_LEN];
	int SessionNumber;
	int SlipDocumentIsMoving;
	int SlipDocumentIsPresent;
	int SlowingInMilliliters;
	int SlowingValve;
	int StatusRK;
	char StatusRKDescription[MAX_LEN];
	char StringForPrinting[MAX_LEN];
	int StringQuantity;
	double Summ1;				// Оплата НАЛИЧНЫМИ
	short Summ1Enabled;
	double Summ2;
	short Summ2Enabled;
	double Summ3;
	short Summ3Enabled;
	double Summ4;				// Оплата КАРТОЙ
	short Summ4Enabled;
	double Summ5;
	double Summ6;
	double Summ7;
	double Summ8;
	double Summ9;
	double Summ10;
	double Summ11;
	double Summ12;
	double Summ13;
	double Summ14;
	double Summ15;
	double Summ16;
	char TableName[MAX_LEN];
	int TableNumber;
	double TaxValue;
	double TaxValue1;
	double TaxValue2;
	double TaxValue3;
	double TaxValue4;
	double TaxValue5;
	double TaxValue6;
	short TaxValueEnabled;
	int Tax1;
	int Tax2;
	int Tax3;
	int Tax4;
	int TaxType;
	int TCPPort;
	struct tm Time;
	int Timeout;
	BYTE TLVData[4096];
	int TRKNumber;
	int TypeOfSumOfEntriesFM;
	bool UseIPAddress;
	int UseJournalRibbon;
	int UseReceiptRibbon;
	int UseSlipDocument;
	int UModel;
	int UMajorType;
	int UMinorType;
	int UMajorProtocolVersion;
	int UMinorProtocolVersion;
	int UCodePage;
	char UDescription[MAX_LEN];
	int ValueOfFieldInteger;
	char ValueOfFieldString[MAX_LEN];
	int InfoExchangeStatus;

	/****************************************************
	 * Shtrih-M interface functions                     *
	 ****************************************************/

	int Connect(void);
	int Disconnect(void);
	int Beep(void);
	int Buy(void);
	int CancelCheck(void);
	int CashIncome(void);
	int CashOutcome(void);
	int Charge(void);
	int CheckSubTotal(void);
	int CloseCheck(void);
	int ConfirmDate(void);
	int ContinuePrinting(void);
	int Correction(void);
	int CutCheck(void);
	int DampRequest(void);
	int Discount(void);
	int DozeOilCheck(void);
	int Draw(void);
	int EjectSlipDocument(void);
	int EKLZDepartmentReportInDatesRange(void);
	int EKLZDepartmentReportInSessionsRange(void);
	int EKLZJournalOnSessionNumber(void);
	int EKLZSessionReportInDatesRange(void);
	int EKLZSessionReportInSessionRange(void);
	int FeedDocument(void);
	int Fiscalization(void);
	int FiscalReportForDatesRange(void);
	int FiscalReportForSessionRange(void);
	int GetData(void);
	int GetDeviceMetrics(void);
	int GetExchangeParam(void);
	int GetFieldStruct(void);
	int GetFiscalizationParameters(void);
	int GetFMRecordsSum(void);
	int GetECRStatus(void);
	int GetLastFMRecordDate(void);
	int GetLiterSumCounter(void);
	int GetCashReg(void);
	int GetOperationReg(void);
	int GetRangeDatesAndSessions(void);
	int GetRKStatus(void);
	int GetTableStruct(void);
	int InitFM(void);
	int InitTable(void);
	int InterruptDataStream(void);
	int InterruptFullReport(void);
	int InterruptTest(void);
	int LaunchRK(void);
	int LoadLineData(void);
	int OilSale(void);
	int OpenDrawer(void);
	int PrintBarCode(void);
	int PrintDocumentTitle(void);
	int PrintOperationReg(void);
	int PrintReportWithCleaning(void);
	int PrintReportWithoutCleaning(void);
	int PrintString(void);
	int PrintStringWithFont(void);
	int PrintWideString(void);
	int ReadEKLZDocumentOnKPK(void);
	int ReadEKLZSessionTotal(void);
	int ReadLicense(void);
	int ReadTable(void);
	int ResetAllTRK(void);
	int ResetRK(void);
	int ResetSettings(void);
	int ResetSummary(void);
	int ReturnBuy(void);
	int ReturnSale(void);
	int Sale(void);
	int SetDate(void);
	int SetDozeInMilliliters(void);
	int SetDozeInMoney(void);
	int SetExchangeParam(void);
	int SetPointPosition(void);
	int SetRKParameters(void);
	int SetSerialNumber(void);
	int SetTime(void);
	int StopEKLZDocumentPrinting(void);
	int StopRK(void);
	int Storno(void);
	int StornoCharge(void);
	int StornoDiscount(void);
	int SummOilCheck(void);
	int Test(void);
	int WriteLicense(void);
	int WriteTable(void);
	int OpenCheck(void);
	/*******************************************************************
	 *   WORK WITH FN                                                           *
	 *******************************************************************/
	int FNGetStatus(void);
	int FNOperation(void);
	int FNCloseCheckEx(void);
	int FNOpenSession(void);
	int FNCloseSession(void);
	int FNCancelCheck(void);
	int FNGetInfoExchangeStatus(void);
	int FNRequestFiscalDocumentTLV(void);
	int FNReadFiscalDocumentTLV(void);

	int PrintErrorDescription(void);
	int CheckConnection(void);
	int		errhand(void *a);
	int     evalint(unsigned char*, int);
	__int64_t evalint64(unsigned char*, int);
	void    evaldate(unsigned char*, struct tm*);
	void    DefineECRModeDescription(void);
	void	evaltime(unsigned char *str, struct tm *time);

	/****************************************************
	 * Shtrih-M driver constructor                      *
	 ****************************************************/
private:
	DrvFR() {}
public:
		DrvFR(int password,
			int comNumber,
			int baudRate,
			int timeout,
			int protocolType,
			int connectionType,
			int tcpPort,
			char* ipAddress,
			bool useIPAddress
		);
};

}

#endif
