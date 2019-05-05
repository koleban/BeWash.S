#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include "errdefs.h"
#include "DrvFR_Conn.h"

//#define DEBUG

using namespace DriverFR;

DrvFR::DrvFR(int password,
	int comNumber,
	int baudRate,
	int timeout,
	int protocolType,
	int connectionType,
	int tcpPort,
	char* ipAddress,
	bool useIPAddress)
{
	Password = password;
	ComNumber = comNumber;
	BaudRate = baudRate;
	Timeout = timeout;
	ProtocolType = protocolType;
	ConnectionType = connectionType;
	TCPPort = tcpPort;
	strcpy(IPAddress, (const char *)ipAddress);
	UseIPAddress = useIPAddress;
	#ifdef DEBUG
	printf("DrvFR constructor:\n  Password: %d\n  Timeout: %d\n  IP: %s\n", Password, Timeout, IPAddress);
	#endif
}
//-----------------------------------------------------------------------------
//*****************************************************************************
//-----------------------------------------------------------------------------
int DrvFR::errhand(void *a)
{
	if (((answer*)a)->buff[0] != 0xFF)
		ResultCode = ((answer*)a)->buff[1];
	else
		ResultCode = ((answer*)a)->buff[2];
	strcpy(ResultCodeDescription, errmsg[ResultCode]);
	if (ResultCode != 0)
	{
		printf("fn: errhand (%d) --> %s\n", ResultCode, ResultCodeDescription);
		fflush(stdout);
	}
	return ResultCode;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintErrorDescription(void)
{
	strcpy(ResultCodeDescription, errmsg[ResultCode]);
	if (ResultCode != 0)
	{
		printf("fn: errhand (%d) --> %s\n", ResultCode, ResultCodeDescription);
		fflush(stdout);
	}
	return ResultCode;
}
//-----------------------------------------------------------------------------
int DrvFR::evalint(unsigned char *str, int len)
{
	int result = 0;
	while (len--)
	{
		result <<= 8;
		result += str[len];
	};
	return result;
}
//-----------------------------------------------------------------------------
__int64_t DrvFR::evalint64(unsigned char *str, int len)
{
	__int64_t result = 0;
	while (len--)
	{
		result <<= 8;
		result += str[len];
	};
	return result;
}
//-----------------------------------------------------------------------------
void DrvFR::evaldate(unsigned char *str, struct tm *date)
{
	date->tm_mday = evalint(str, 1);
	date->tm_mon = evalint(str + 1, 1) - 1;
	date->tm_year = evalint(str + 2, 1) + 100;
	mktime(date);
}
//-----------------------------------------------------------------------------
void DrvFR::evaltime(unsigned char *str, struct tm *time)
{
	time->tm_hour = evalint(str, 1);
	time->tm_min = evalint(str + 1, 1);
	time->tm_sec = evalint(str + 2, 1);
	mktime(time);
}
//-----------------------------------------------------------------------------
void DrvFR::DefineECRModeDescription(void)
{
	ECRMode8Status = ECRMode >> 4;
	if ((ECRMode & 8) == 8)
	{
		strcpy(ECRModeDescription, ecrmode8desc[ECRMode8Status]);
		return;
	};
	strcpy(ECRModeDescription, ecrmodedesc[ECRMode]);
};

/**********************************************************
 *       Implementation of the interface functions        *
 **********************************************************/
int DrvFR::Connect(void)
{
	int tries = 0;
	int state = 0;
	answer      a;

	if (conn == NULL)
	{
		#ifdef DEBUG
		printf("DrvFR::Connect Create new instance:\n  Password: %d\n  Timeout: %d\n  IP: %s\n", Password, Timeout, IPAddress);
		#endif
		conn = new DrvFR_Conn(this);
	}

	if (Connected) conn->closedev();

	if (conn->opendev() == -1)
	{
		#ifdef DEBUG
		printf("fn: DrvFR::Connect - can't opendev()\n");
		#endif
		Connected = false;
		return -1;
	}
	else
		Connected = true;

	while (tries < MAX_TRIES)
	{
		state = conn->checkstate();
		switch (state)
		{
		case NAK:
			Connected = true;
			#ifdef DEBUG
			printf("fn: DrvFR::Connect - receive NAK\n");
			#endif
			if (GetECRStatus() < 0) { tries++; continue; }
			if (GetDeviceMetrics() < 0) { tries++; continue; }
			return 1;
		case ACK:
			Connected = true;
			#ifdef DEBUG
			printf("fn: DrvFR::Connect - receive ACK\n");
			#endif
			conn->readanswer(&a);
			conn->checkstate();
			if (GetECRStatus() < 0) { tries++; usleep(500000); continue; }
			if (GetDeviceMetrics() < 0) { tries++; usleep(500000); continue; }
			return 1;
		case 0xFF:
			tries++;
		};
	};
	#ifdef DEBUG
	printf("fn: DrvFR::Connect - can't receive answer\n");
	#endif
	Connected = false;
	return -1;
}
//-----------------------------------------------------------------------------
int DrvFR::Disconnect(void)
{
	Connected = false;
	return conn->closedev();
}
//-----------------------------------------------------------------------------
int DrvFR::Beep(void)
{
	parameter  p;
	answer     a;

	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (conn->sendcommand(BEEP, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != BEEP) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::CutCheck(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 1;

	if (!Connected) return -1;

	p.buff[0] = CutType;

	if (conn->sendcommand(CUT_CHECK, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CUT_CHECK) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintString(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));
	p.len = 41;

	if (!Connected) return -1;

	p.buff[0] = (UseJournalRibbon == true) ? 1 : 0;
	p.buff[0] |= (UseReceiptRibbon == true) ? 2 : 0;

	strncpy((char*)&p.buff + 1, (char*)StringForPrinting, 40);

	if (conn->sendcommand(PRINT_STRING, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != PRINT_STRING) return -1;

	if (errhand(&a) != 0) return  ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintStringWithFont(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));
	p.len = 41;

	if (!Connected) return -1;

	p.buff[0] = (UseJournalRibbon == true) ? 1 : 0;
	p.buff[0] |= (UseReceiptRibbon == true) ? 2 : 0;
	p.buff[1] = FontType;

	strncpy((char*)&p.buff + 2, (char*)StringForPrinting, 40);

	if (conn->sendcommand(PRINT_STRING_WITH_FONT, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != PRINT_STRING) return -1;

	if (errhand(&a) != 0) return  ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintWideString(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 21;

	if (!Connected) return -1;

	p.buff[0] = (UseJournalRibbon == true) ? 1 : 0;
	p.buff[0] |= (UseReceiptRibbon == true) ? 2 : 0;

	strncpy((char*)&p.buff + 1, (char*)StringForPrinting, 20);

	if (conn->sendcommand(PRINT_WIDE_STRING, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != PRINT_WIDE_STRING) return -1;

	if (errhand(&a) != 0) return  ResultCode;;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FeedDocument(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 2;

	if (!Connected) return -1;

	p.buff[0] = (UseJournalRibbon == true) ? 1 : 0;
	p.buff[0] |= (UseReceiptRibbon == true) ? 2 : 0;
	p.buff[0] |= (UseSlipDocument == true) ? 4 : 0;

	p.buff[1] = StringQuantity;

	if (conn->sendcommand(FEED_DOCUMENT, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != FEED_DOCUMENT) return -1;

	if (errhand(&a) != 0) return  ResultCode;;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::SetExchangeParam(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 3;

	if (!Connected) return -1;

	p.buff[0] = PortNumber;
	p.buff[1] = BaudRate;
	p.buff[2] = Timeout;

	if (conn->sendcommand(SET_EXCHANGE_PARAM, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != SET_EXCHANGE_PARAM) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::GetExchangeParam(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 1;

	if (!Connected) return -1;

	p.buff[0] = PortNumber;

	if (conn->sendcommand(GET_EXCHANGE_PARAM, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_EXCHANGE_PARAM) return -1;

	if (errhand(&a) != 0) return ResultCode;

	BaudRate = a.buff[2];
	Timeout = a.buff[3];
	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::GetECRStatus(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;
	if (conn->sendcommand(GET_ECR_STATUS, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) { return -1; }
	if (a.buff[0] != GET_ECR_STATUS) { return -1; }
	if (errhand(&a))	return  ResultCode;

	OperatorNumber = a.buff[2];

	ECRSoftVersion[0] = a.buff[3];
	ECRSoftVersion[1] = 0x2e;
	ECRSoftVersion[2] = a.buff[4];
	ECRSoftVersion[3] = 0;

	ECRBuild = evalint((unsigned char*)&a.buff + 5, 2);
	evaldate((unsigned char*)&a.buff + 7, &ECRSoftDate);
	LogicalNumber = evalint((unsigned char*)&a.buff + 10, 1);
	OpenDocumentNumber = evalint((unsigned char*)&a.buff + 11, 2);
	ReceiptRibbonIsPresent = (a.buff[13] & 1) == 1; //0
	JournalRibbonIsPresent = (a.buff[13] & 2) == 2; //1
	SlipDocumentIsPresent = (a.buff[13] & 4) == 4; //2
	SlipDocumentIsMoving = (a.buff[13] & 8) == 8; //3
	PointPosition = (a.buff[13] & 16) == 16; //4
	EKLZIsPresent = (a.buff[13] & 32) == 32; //5
	JournalRibbonOpticalSensor = (a.buff[13] & 64) == 64; //6
	ReceiptRibbonOpticalSensor = (a.buff[13] & 128) == 128; //6

	JournalRibbonLever = (a.buff[14] & 1) == 1; //0
	ReceiptRibbonLever = (a.buff[14] & 2) == 2; //1
	LidPositionSensor = (a.buff[14] & 4) == 4; //2

	ECRMode = evalint((unsigned char*)&a.buff + 15, 1);
	DefineECRModeDescription();


	ECRAdvancedMode = evalint((unsigned char*)&a.buff + 16, 1);
	strcpy(ECRAdvancedModeDescription, ecrsubmodedesc[ECRAdvancedMode]);

	PortNumber = evalint((unsigned char*)&a.buff + 17, 1);
	FMSoftVersion[0] = a.buff[18];
	FMSoftVersion[1] = 0x2e;
	FMSoftVersion[2] = a.buff[19];
	FMSoftVersion[3] = 0;
	FMBuild = evalint((unsigned char*)&a.buff + 20, 2);
	evaldate((unsigned char*)&a.buff + 22, &FMSoftDate);
	evaldate((unsigned char*)&a.buff + 25, &Date);
	evaltime((unsigned char*)&a.buff + 28, &Time);
	FM1IsPresent = (a.buff[31] & 1) == 1;
	FM2IsPresent = (a.buff[31] & 2) == 2;
	LicenseIsPresent = (a.buff[31] & 4) == 4;
	FMOverflow = (a.buff[31] & 8) == 8;
	BatteryCondition = (a.buff[31] & 16) == 16;
	sprintf(SerialNumber, "%d", evalint((unsigned char*)&a.buff + 32, 4));
	SessionNumber = evalint((unsigned char*)&a.buff + 36, 2);
	FreeRecordInFM = evalint((unsigned char*)&a.buff + 38, 2);
	RegistrationNumber = evalint((unsigned char*)&a.buff + 40, 1);
	FreeRegistration = evalint((unsigned char*)&a.buff + 41, 1);
	sprintf(INN, "%.0lg", (double)evalint64((unsigned char*)&a.buff + 42, 6));

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::GetDeviceMetrics(void)
{
	int len = 0;
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 1;

	if (!Connected) return -1;

	if (conn->sendcommand(GET_DEVICE_METRICS, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_DEVICE_METRICS) return -1;
	if (errhand(&a)) return  ResultCode;
	len = a.len - 7;
	UMajorType = evalint((unsigned char*)&a.buff + 2, 1);
	UMinorType = evalint((unsigned char*)&a.buff + 3, 1);
	UMajorProtocolVersion = evalint((unsigned char*)&a.buff + 4, 1);
	UMinorProtocolVersion = evalint((unsigned char*)&a.buff + 5, 1);
	UModel = evalint((unsigned char*)&a.buff + 6, 1);
	UCodePage = evalint((unsigned char*)&a.buff + 7, 1);
	strncpy(UDescription, (char*)a.buff + 8, len);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::Test(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 1;

	if (!Connected) return -1;

	p.buff[0] = RunningPeriod;

	if (conn->sendcommand(TEST, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != TEST) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::InterruptTest(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(INTERRUPT_TEST, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != INTERRUPT_TEST) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::ContinuePrinting(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(CONTINUE_PRINTING, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CONTINUE_PRINTING) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::OpenDrawer(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 1;

	if (!Connected) return -1;

	p.buff[0] = DrawerNumber;

	if (conn->sendcommand(OPEN_DRAWER, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != OPEN_DRAWER) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintDocumentTitle(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 32;

	if (!Connected) return -1;

	strncpy((char*)p.buff, (char*)DocumentName, 30);
	memcpy(&p.buff + 30, &DocumentNumber, 2);

	if (conn->sendcommand(PRINT_DOCUMENT_TITLE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != PRINT_DOCUMENT_TITLE) return -1;

	if (errhand(&a) != 0) return  ResultCode;;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::ResetSettings(void)
{
	parameter  p;

	memset(&p, 0, sizeof(p));

	if (!Connected) return -1;

	if (conn->sendcommand(RESET_SETTINGS, Password, &p) < 0) return -1;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::ResetSummary(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (conn->sendcommand(RESET_SUMMARY, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != RESET_SUMMARY) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::Buy(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 55;

	if (!Connected) return -1;

	__int64_t quant = llround(Quantity * 1000);
	__int64_t price = llround(Price * 100);

	memcpy(p.buff, &quant, 5);
	memcpy(p.buff + 5, &price, 5);
	p.buff[10] = Department;

	p.buff[11] = Tax1;
	p.buff[12] = Tax2;
	p.buff[13] = Tax3;
	p.buff[14] = Tax4;

	strncpy((char*)p.buff + 15, (char*)StringForPrinting, 40);

	if (conn->sendcommand(BUY, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != BUY) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::ReturnBuy(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 55;

	if (!Connected) return -1;

	__int64_t quant = llround(Quantity * 1000);
	__int64_t price = llround(Price * 100);

	memcpy(p.buff, &quant, 5);
	memcpy(p.buff + 5, &price, 5);
	p.buff[10] = Department;

	p.buff[11] = Tax1;
	p.buff[12] = Tax2;
	p.buff[13] = Tax3;
	p.buff[14] = Tax4;

	strncpy((char*)p.buff + 15, (char*)StringForPrinting, 40);

	if (conn->sendcommand(RETURN_BUY, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != RETURN_BUY) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::Sale(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 55;

	if (!Connected) return -1;

	__int64_t quant = llround(Quantity * 1000);
	__int64_t price = llround(Price * 100);

	memcpy(p.buff, &quant, 5);
	memcpy(p.buff + 5, &price, 5);
	p.buff[10] = Department;

	p.buff[11] = Tax1;
	p.buff[12] = Tax2;
	p.buff[13] = Tax3;
	p.buff[14] = Tax4;

	strncpy((char*)p.buff + 15, (char*)StringForPrinting, 40);

	if (conn->sendcommand(SALE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != SALE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::ReturnSale(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 55;

	if (!Connected) return -1;

	__int64_t quant = llround(Quantity * 1000);
	__int64_t price = llround(Price * 100);

	memcpy(p.buff, &quant, 5);
	memcpy(p.buff + 5, &price, 5);
	p.buff[10] = Department;

	p.buff[11] = Tax1;
	p.buff[12] = Tax2;
	p.buff[13] = Tax3;
	p.buff[14] = Tax4;

	strncpy((char*)p.buff + 15, (char*)StringForPrinting, 40);

	if (conn->sendcommand(RETURN_SALE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != RETURN_SALE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::CancelCheck(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (conn->sendcommand(CANCEL_CHECK, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CANCEL_CHECK) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::CashIncome(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 5;

	if (!Connected) return -1;

	__int64_t  sum = llround(Summ1 * 100);

	memcpy(p.buff, &sum, 5);

	if (conn->sendcommand(CASH_INCOME, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CASH_INCOME) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	OpenDocumentNumber = evalint((unsigned char*)&a.buff + 3, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::CashOutcome(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 5;

	if (!Connected) return -1;

	__int64_t  sum = llround(Summ1 * 100);

	memcpy(p.buff, &sum, 5);

	if (conn->sendcommand(CASH_OUTCOME, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CASH_OUTCOME) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	OpenDocumentNumber = evalint((unsigned char*)&a.buff + 3, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::Charge(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 49;

	if (!Connected) return -1;

	__int64_t  sum = llround(Summ1 * 100);

	memcpy(p.buff, &sum, 5);

	p.buff[5] = Tax1;
	p.buff[6] = Tax2;
	p.buff[7] = Tax3;
	p.buff[8] = Tax4;

	strncpy((char*)p.buff + 9, (char*)StringForPrinting, 40);

	if (conn->sendcommand(CHARGE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CHARGE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::StornoCharge(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 49;

	if (!Connected) return -1;

	__int64_t  sum = llround(Summ1 * 100);

	memcpy(p.buff, &sum, 5);

	p.buff[5] = Tax1;
	p.buff[6] = Tax2;
	p.buff[7] = Tax3;
	p.buff[8] = Tax4;

	strncpy((char*)p.buff + 9, (char*)StringForPrinting, 40);

	if (conn->sendcommand(STORNO_CHARGE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != STORNO_CHARGE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::Discount(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 49;

	if (!Connected) return -1;

	__int64_t  sum = llround(Summ1 * 100);

	memcpy(p.buff, &sum, 5);

	p.buff[5] = Tax1;
	p.buff[6] = Tax2;
	p.buff[7] = Tax3;
	p.buff[8] = Tax4;

	strncpy((char*)p.buff + 9, (char*)StringForPrinting, 40);

	if (conn->sendcommand(DISCOUNT, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != DISCOUNT) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::StornoDiscount(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	p.len = 49;

	if (!Connected) return -1;

	__int64_t sum = llround(Summ1 * 100);

	memcpy(p.buff, &sum, 5);

	p.buff[5] = Tax1;
	p.buff[6] = Tax2;
	p.buff[7] = Tax3;
	p.buff[8] = Tax4;

	strncpy((char*)p.buff + 9, (char*)StringForPrinting, 40);

	if (conn->sendcommand(STORNO_DISCOUNT, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != STORNO_DISCOUNT) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::CheckSubTotal(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (conn->sendcommand(CHECK_SUBTOTAL, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CHECK_SUBTOTAL) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	Summ1 = evalint64((unsigned char*)&a.buff + 3, 5);
	Summ1 /= 100;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::CloseCheck(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	__int64_t  sum;
	p.len = 67;

	sum = llround(Summ1 * 100);
	memcpy(p.buff, &sum, 5);			// 0-4
	sum = llround(Summ2 * 100);
	memcpy(p.buff + 5, &sum, 5);			// 5-9
	sum = llround(Summ3 * 100);
	memcpy(p.buff + 10, &sum, 5);			//10-14
	sum = llround(Summ4 * 100);
	memcpy(p.buff + 15, &sum, 5);			//15-19

	sum = llround(DiscountOnCheck * 100);
	memcpy(p.buff + 20, &sum, 3);			//20-22

	p.buff[23] = Tax1;				//23
	p.buff[24] = Tax2;				//24
	p.buff[25] = Tax3;				//25
	p.buff[26] = Tax4;				//26

	strncpy((char*)p.buff + 27, (char*)StringForPrinting, 40);
	if (conn->sendcommand(CLOSE_CHECK, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CLOSE_CHECK) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	Change = evalint64((unsigned char*)&a.buff + 3, 5);
	Change /= 100;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::Storno(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	__int64_t quant = llround(Quantity * 1000);
	__int64_t price = llround(Price * 100);
	p.len = 55;

	memcpy(p.buff, &quant, 5);
	memcpy(p.buff + 5, &price, 5);
	p.buff[10] = Department;

	p.buff[11] = Tax1;
	p.buff[12] = Tax2;
	p.buff[13] = Tax3;
	p.buff[14] = Tax4;

	strncpy((char*)p.buff + 15, (char*)StringForPrinting, 40);

	if (conn->sendcommand(STORNO, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != STORNO) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintReportWithCleaning(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (conn->sendcommand(PRINT_REPORT_WITH_CLEANING, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != PRINT_REPORT_WITH_CLEANING) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	SessionNumber = evalint((unsigned char*)&a.buff + 3, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintReportWithoutCleaning(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (conn->sendcommand(PRINT_REPORT_WITHOUT_CLEANING, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != PRINT_REPORT_WITHOUT_CLEANING) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	SessionNumber = evalint((unsigned char*)&a.buff + 3, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintOperationReg(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (conn->sendcommand(PRINT_OPERATION_REG, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != PRINT_OPERATION_REG) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::DampRequest(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	p.len = 1;

	p.buff[0] = DeviceCode;

	if (conn->sendcommand(DUMP_REQUEST, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != DUMP_REQUEST) return -1;

	if (errhand(&a) != 0) return -1;

	return a.buff[2];  			//number of data blocks to return
}
//-----------------------------------------------------------------------------
int DrvFR::GetData(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(GET_DATA, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_DATA) return -1;

	if (errhand(&a) != 0) return ResultCode;

	DeviceCode = a.buff[2];
	strcpy(DeviceCodeDescription, devcodedesc[DeviceCode]);
	DataBlockNumber = evalint((unsigned char*)&a.buff + 3, 2);
	memcpy(DataBlock, a.buff + 5, 32);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::InterruptDataStream(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(INTERRUPT_DATA_STREAM, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != INTERRUPT_DATA_STREAM) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::GetCashReg(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 1;

	p.buff[0] = RegisterNumber;

	if (conn->sendcommand(GET_CASH_REG, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_CASH_REG) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	ContentsOfCashRegister = evalint64((unsigned char*)&a.buff + 3, 6);
	ContentsOfCashRegister /= 100;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::GetOperationReg(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 1;

	p.buff[0] = RegisterNumber;

	if (conn->sendcommand(GET_OPERATION_REG, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_OPERATION_REG) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	ContentsOfOperationRegister = evalint((unsigned char*)&a.buff + 3, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::SetSerialNumber(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 4;
	int num = atol(SerialNumber);

	memcpy(p.buff, &num, sizeof(int));

	if (conn->sendcommand(SET_SERIAL_NUMBER, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != SET_SERIAL_NUMBER) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::SetPointPosition(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 1;

	p.buff[0] = PointPosition;

	if (conn->sendcommand(SET_POINT_POSITION, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != SET_POINT_POSITION) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::SetTime(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 3;

	p.buff[2] = Time.tm_sec;
	p.buff[1] = Time.tm_min;
	p.buff[0] = Time.tm_hour;

	if (conn->sendcommand(SET_TIME, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != SET_TIME) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::SetDate(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 3;

	p.buff[0] = Date.tm_mday;
	p.buff[1] = Date.tm_mon + 1;
	p.buff[2] = Date.tm_year - 100;

	if (conn->sendcommand(SET_DATE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != SET_DATE) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::ConfirmDate(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 3;

	p.buff[0] = Date.tm_mday;
	p.buff[1] = Date.tm_mon + 1;
	p.buff[2] = Date.tm_year - 100;

	if (conn->sendcommand(CONFIRM_DATE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CONFIRM_DATE) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::InitTable(void)
{
	parameter  p;
	memset(&p, 0, sizeof(p));

	if (!Connected) return -1;

	if (conn->sendcommand(INIT_TABLE, Password, &p) < 0) return -1;
	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::WriteTable(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	int      len;
	char    *tmp;

	if (!Connected) return -1;

	GetFieldStruct();
	if (FieldType == 1)
	{
		len = strlen(ValueOfFieldString);
		tmp = ValueOfFieldString;
	}
	else
	{
		len = FieldSize;
		tmp = (char*)&ValueOfFieldInteger;
	};
	p.len = 4 + len;

	p.buff[0] = TableNumber;
	memcpy(p.buff + 1, &FieldNumber, 2);
	p.buff[3] = RowNumber;
	memcpy(p.buff + 4, &tmp, len);

	if (conn->sendcommand(WRITE_TABLE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != WRITE_TABLE) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::ReadTable(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	int      len;

	if (!Connected) return -1;

	p.len = 4;

	p.buff[0] = TableNumber;
	memcpy(p.buff + 1, &FieldNumber, 2);
	p.buff[3] = RowNumber;

	if (conn->sendcommand(READ_TABLE, Password, &p) < 0) return -1;
	if ((len = conn->readanswer(&a)) < 0) return -1;
	if (a.buff[0] != READ_TABLE) return -1;

	GetFieldStruct();

	if (FieldType == 1) strncpy(ValueOfFieldString, (char*)a.buff, len - 2);
	else ValueOfFieldInteger = evalint64((unsigned char*)&a.buff + 2, FieldSize);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::GetFieldStruct(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 2;

	p.buff[0] = TableNumber;
	p.buff[1] = FieldNumber;

	if (conn->sendcommand(GET_FIELD_STRUCT, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_FIELD_STRUCT) return -1;

	strncpy(FieldName, (char*)&a.buff + 1, 40);
	FieldType = a.buff[41];
	FieldSize = a.buff[42];
	MINValueOfField = evalint64((unsigned char*)&a.buff + 43, FieldSize);
	MAXValueOfField = evalint64((unsigned char*)&a.buff + 43 + FieldSize, FieldSize);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::GetTableStruct(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 1;

	p.buff[0] = TableNumber;

	if (conn->sendcommand(GET_TABLE_STRUCT, Password, &p) < 0) return -1;
	if ((conn->readanswer(&a)) < 0) return -1;
	if (a.buff[0] != GET_TABLE_STRUCT) return -1;

	strncpy(TableName, (char*)&a.buff + 1, 40);
	RowNumber = evalint((unsigned char*)&a.buff + 41, 2);;
	FieldNumber = a.buff[43];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::WriteLicense(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	__int64_t num = atoll(License);
	p.len = 5;

	memcpy(p.buff, &num, 5);

	if (conn->sendcommand(WRITE_LICENSE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != WRITE_LICENSE) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::ReadLicense(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(READ_LICENSE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != READ_LICENSE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	sprintf(License, "%.0lg", (double)evalint64((unsigned char*)&a.buff + 2, 5));

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::InitFM(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(INIT_FM, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != INIT_FM) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::Fiscalization(void)
{
	parameter  p;
	answer 	   a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	int     nti = NewPasswordTI;
	__int64_t rnm = atoll(RNM);
	__int64_t inn = atoll(INN);
	p.len = 14;

	memcpy(p.buff, &nti, 4);
	memcpy(p.buff + 4, &rnm, 5);
	memcpy(p.buff + 9, &inn, 6);

	if (conn->sendcommand(FISCALIZATION, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != FISCALIZATION) return -1;

	if (errhand(&a) != 0) return ResultCode;

	RegistrationNumber = evalint((unsigned char*)&a.buff + 2, 1);
	FreeRegistration = evalint((unsigned char*)&a.buff + 3, 1);
	SessionNumber = evalint((unsigned char*)&a.buff + 4, 2);
	evaldate((unsigned char*)&a.buff + 6, &Date);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FiscalReportForDatesRange(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 7;

	p.buff[0] = ReportType;

	p.buff[1] = FirstSessionDate.tm_year - 100;
	p.buff[2] = FirstSessionDate.tm_mon + 1;
	p.buff[3] = FirstSessionDate.tm_mday;

	p.buff[4] = LastSessionDate.tm_year - 100;
	p.buff[5] = LastSessionDate.tm_mon + 1;
	p.buff[6] = LastSessionDate.tm_mday;

	if (conn->sendcommand(FISCAL_REPORT_FOR_DATES_RANGE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != FISCAL_REPORT_FOR_DATES_RANGE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	evaldate((unsigned char*)&a.buff + 2, &FirstSessionDate);
	evaldate((unsigned char*)&a.buff + 5, &LastSessionDate);
	FirstSessionNumber = evalint((unsigned char*)&a.buff + 8, 2);
	LastSessionNumber = evalint((unsigned char*)&a.buff + 10, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FiscalReportForSessionRange(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 5;

	p.buff[0] = ReportType;

	memcpy(p.buff + 1, &FirstSessionNumber, 2);
	memcpy(p.buff + 3, &LastSessionNumber, 2);

	if (conn->sendcommand(FISCAL_REPORT_FOR_SESSION_RANGE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != FISCAL_REPORT_FOR_SESSION_RANGE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	evaldate((unsigned char*)&a.buff + 2, &FirstSessionDate);
	evaldate((unsigned char*)&a.buff + 5, &LastSessionDate);
	FirstSessionNumber = evalint((unsigned char*)&a.buff + 8, 2);
	LastSessionNumber = evalint((unsigned char*)&a.buff + 10, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::InterruptFullReport(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(INTERRUPT_FULL_REPORT, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != INTERRUPT_FULL_REPORT) return -1;

	return errhand(&a);
}
//-----------------------------------------------------------------------------
int DrvFR::GetFiscalizationParameters(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 1;

	p.buff[0] = RegistrationNumber;

	if (conn->sendcommand(GET_FISCALIZATION_PARAMETERS, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_FISCALIZATION_PARAMETERS) return -1;

	if (errhand(&a) != 0) return ResultCode;

	NewPasswordTI = evalint((unsigned char*)&a.buff + 2, 4);
	sprintf(RNM, "%.0lg", (double)evalint64((unsigned char*)&a.buff + 6, 5));
	sprintf(INN, "%.0lg", (double)evalint64((unsigned char*)&a.buff + 11, 6));
	SessionNumber = evalint((unsigned char*)&a.buff + 17, 2);
	evaldate((unsigned char*)&a.buff + 19, &Date);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::GetFMRecordsSum(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 1;

	p.buff[0] = TypeOfSumOfEntriesFM;

	if (conn->sendcommand(GET_FM_RECORDS_SUM, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_FM_RECORDS_SUM) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	Summ1 = evalint64((unsigned char*)&a.buff + 3, 8);
	Summ1 /= 100;
	Summ2 = evalint64((unsigned char*)&a.buff + 11, 6);
	Summ2 /= 100;
	Summ3 = evalint64((unsigned char*)&a.buff + 17, 6);
	Summ3 /= 100;
	Summ4 = evalint64((unsigned char*)&a.buff + 23, 6);
	Summ4 /= 100;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::GetLastFMRecordDate(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(GET_LAST_FM_RECORD_DATE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_LAST_FM_RECORD_DATE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	LastFMRecordType = a.buff[3];
	evaldate((unsigned char*)&a.buff + 4, &Date);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::GetRangeDatesAndSessions(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	if (conn->sendcommand(GET_RANGE_DATES_AND_SESSIONS, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != GET_RANGE_DATES_AND_SESSIONS) return -1;

	if (errhand(&a) != 0) return ResultCode;

	evaldate((unsigned char*)&a.buff + 2, &FirstSessionDate);
	evaldate((unsigned char*)&a.buff + 5, &LastSessionDate);
	FirstSessionNumber = evalint((unsigned char*)&a.buff + 8, 2);
	LastSessionNumber = evalint((unsigned char*)&a.buff + 10, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::EKLZDepartmentReportInDatesRange(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::EKLZDepartmentReportInSessionsRange(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::EKLZJournalOnSessionNumber(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::EKLZSessionReportInDatesRange(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::EKLZSessionReportInSessionRange(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::ReadEKLZDocumentOnKPK(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::ReadEKLZSessionTotal(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::StopEKLZDocumentPrinting(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::Correction(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::DozeOilCheck(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::SummOilCheck(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::SetDozeInMilliliters(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::SetDozeInMoney(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::OilSale(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::GetLiterSumCounter(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::GetRKStatus(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::LaunchRK(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::StopRK(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::ResetRK(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::ResetAllTRK(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::SetRKParameters(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::EjectSlipDocument(void) { return 0; }
//-----------------------------------------------------------------------------
int DrvFR::LoadLineData(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 41;

	p.buff[0] = LineNumber;
	memcpy(p.buff + 1, LineData, 40);

	if (conn->sendcommand(LOAD_LINE_DATA, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != LOAD_LINE_DATA) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::Draw(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 2;

	p.buff[0] = FirstLineNumber;
	p.buff[1] = LastLineNumber;

	if (conn->sendcommand(DRAW, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != DRAW) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::PrintBarCode(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	__int64_t barcode = atoll(BarCode);
	p.len = 5;

	memcpy(p.buff, &barcode, 5);

	if (conn->sendcommand(PRINT_BARCODE, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != PRINT_BARCODE) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::OpenCheck(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.buff[0] = DocumentType;
	p.len = 1;

	if (conn->sendcommand(OPEN_CHECK, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != OPEN_CHECK) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FNGetStatus(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	p.len = 0;

	if (conn->sendcommand(FN_GET_STATUS, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if ((a.buff[0] != 0xFF) || (a.buff[1] != (FN_GET_STATUS & 0xFF))) return -1;

	evaldate(&a.buff[9], &Date);
	evaldate(&a.buff[12], &Time);
	memset(SerialNumber, 0, sizeof(SerialNumber));
	memcpy(SerialNumber, &a.buff[14], 16);
	DocumentNumber		= evalint(&a.buff[29], 3);

	#ifdef DEBUG
		printf("Buff [%d]:\n", a.len);
		int i = 0;
		while (i < a.len)
			{ printf (" %02X", a.buff[i]); i++; }
		printf("\n");
		fflush(stdout);
	#endif

	if (errhand(&a) != 0) return ResultCode;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FNCloseCheckEx(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	__int64_t  sum;
	memset(&p, 0, sizeof(p));
	p.len = 131;

	sum = llround(Summ1 * 100);
	memcpy(p.buff, &sum, 5);			// 0-4
	sum = llround(Summ2 * 100);
	memcpy(p.buff + 5, &sum, 5);			// 5-9
	sum = llround(Summ3 * 100);
	memcpy(p.buff + 10, &sum, 5);			//10-14
	sum = llround(Summ4 * 100);
	memcpy(p.buff + 15, &sum, 5);
	sum = llround(Summ5 * 100);
	memcpy(p.buff + 20, &sum, 5);
	sum = llround(Summ6 * 100);
	memcpy(p.buff + 25, &sum, 5);
	sum = llround(Summ7 * 100);
	memcpy(p.buff + 30, &sum, 5);
	sum = llround(Summ8 * 100);
	memcpy(p.buff + 35, &sum, 5);
	sum = llround(Summ9 * 100);
	memcpy(p.buff + 40, &sum, 5);
	sum = llround(Summ10 * 100);
	memcpy(p.buff + 45, &sum, 5);
	sum = llround(Summ11 * 100);
	memcpy(p.buff + 50, &sum, 5);
	sum = llround(Summ12 * 100);
	memcpy(p.buff + 55, &sum, 5);
	sum = llround(Summ13 * 100);
	memcpy(p.buff + 60, &sum, 5);
	sum = llround(Summ14 * 100);
	memcpy(p.buff + 65, &sum, 5);
	sum = llround(Summ15 * 100);
	memcpy(p.buff + 70, &sum, 5);
	sum = llround(Summ16 * 100);
	memcpy(p.buff + 75, &sum, 5);

	sum = llround(DiscountOnCheck * 100);
	memcpy(p.buff + 80, &sum, 2);			//80-82

	p.buff[82] = 0;				//83
	p.buff[83] = TaxValue1;				//84
	p.buff[84] = TaxValue2;				//85
	p.buff[85] = TaxValue3;				//86

	strncpy((char*)p.buff + 86, (char*)StringForPrinting, 20);
	p.buff[117] = TaxType;

	if (conn->sendcommand(FN_CLOSE_CHECK_EX, Password, &p) < 0) return -1;
	delay_ms(50);
	if (conn->readanswer(&a) < 0) return -1;
	if ((a.buff[0] != 0xFF) || (a.buff[1] != (FN_CLOSE_CHECK_EX & 0xFF))) return -1;

	OperatorNumber = a.buff[2];
	Change = evalint64((unsigned char*)&a.buff + 3, 5);
	Change /= 100;

//	printf("Oper: %d Change: %f\n", OperatorNumber, Change);

	if (errhand(&a) != 0) return ResultCode;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FNOperation(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 160;

	__int64_t quant = llround(Quantity * 1000000);
	__int64_t price = llround(Price * 100);
	__int64_t summ = 0xFFFFFFFFFFFFFFFF;
	__int64_t tax = 0xFFFFFFFFFFFFFFFF;

	p.buff[0] = CheckType;
	memcpy(p.buff + 1, &quant, 6);
	memcpy(p.buff + 7, &price, 5);
	memcpy(p.buff + 12, &summ, 5);
	memcpy(p.buff + 17, &tax, 5);
	p.buff[22] = TaxType;
	p.buff[23] = Department;
	p.buff[24] = PaymentTypeSign;
	p.buff[25] = PaymentItemSign;
	strncpy((char*)p.buff + 26, (char*)StringForPrinting, 128);

	//for (int i=0; i<p.len; i++)
	//	printf("%02X|", p.buff[i]);
	//printf("\n");

	if (conn->sendcommand(FN_OPERATION, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if ((a.buff[0] != 0xFF) || (a.buff[1] != (FN_OPERATION & 0xFF))) return -1;


	if (errhand(&a) != 0) return ResultCode;

	return 0;
}
//-----------------------------------------------------------------------------

int DrvFR::FNCloseSession(void)
{
	return  this->PrintReportWithCleaning();
}
//-----------------------------------------------------------------------------
int DrvFR::FNOpenSession(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (conn->sendcommand(FN_OPEN_SESSION, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != FN_OPEN_SESSION) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];
	SessionNumber = evalint((unsigned char*)&a.buff + 3, 2);

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::CheckConnection(void)
{
	int tries = 0;
	int state = 0;
	answer      a;
	int tmout = 0;

	if (conn == NULL) return -1;

	if (!Connected) Connect();

	while (tries < 3)
	{
		#ifdef DEBUG
		if (tries > 0)
			printf("fn: CheckConnection [%d]\n", tries);
		#endif
		tmout	= 0;
		state = conn->checkstate();
		switch (state)
		{
		case NAK:
		#ifdef DEBUG
			printf("fn: CheckConnection [NAK]\n");
		#endif
			Connected = true;
			return 1;
		case ACK:
		#ifdef DEBUG
			printf("fn: CheckConnection [ACK]\n");
		#endif
			Connected = true;
			conn->readanswer(&a);
			while ((tmout++ < 5) && (conn->checkstate() == ACK))
			{
				delay_ms(50);
				conn->readanswer(&a);
			}
			return 1;
		case 0xFF:
		#ifdef DEBUG
			printf("fn: CheckConnection [ERR]\n");
		#endif
			tries++;
			fflush(stdout);
		}
	}
	#ifdef DEBUG
	printf("fn: CheckConnection FAILED[%d]\n", tries);
	#endif
	Disconnect();
	return -1;
}
//-----------------------------------------------------------------------------
int DrvFR::FNCancelCheck(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));
	p.len = 5;

	if (!Connected) return -1;

	if (conn->sendcommand(FN_CANCEL_CHECK, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if (a.buff[0] != CANCEL_CHECK) return -1;

	if (errhand(&a) != 0) return ResultCode;

	OperatorNumber = a.buff[2];

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FNGetInfoExchangeStatus(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));


	if (!Connected) return -1;

	p.len = 5;

	if (conn->sendcommand(FN_FNGETINFOEXCHANGESTATUS, Password, &p) < 0) return -1;
	if (conn->readanswer(&a) < 0) return -1;
	if ((a.buff[0] != 0xFF) || (a.buff[1] != (FN_FNGETINFOEXCHANGESTATUS & 0xFF))) return -1;

	if (a.len > 15)
	{
		InfoExchangeStatus = a.buff[2];
		MessageState 		= evalint(&a.buff[3], 2);
		MessageCount 		= evalint(&a.buff[5], 2);
		DocumentNumber		= evalint(&a.buff[7], 4);
		evaldate(&a.buff[9], &Date);
	#ifdef DEBUG
		printf("[FNGetInfoExchangeStatus]: InfoExchangeStatus:%0X  MessageState: %d MessageCount: %d DocumentNumber: %d Date: %0X Time: %0X\n", InfoExchangeStatus, MessageState, MessageCount, DocumentNumber, Date, Time);
		fflush(stdout);
	#endif
	}

	if (errhand(&a) != 0) return ResultCode;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FNRequestFiscalDocumentTLV(void)
{
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	p.len = 4;
	memcpy(p.buff, &DocumentNumber, sizeof(DocumentNumber));

	if (conn->sendcommand(FN_FNREQUESTFISCALDOCUMENTTLV, Password, &p) < 0) return -1;
	delay_ms(500);
	if (conn->readanswer(&a) < 0) return -1;
	if ((a.buff[0] != 0xFF) || (a.buff[1] != (FN_FNREQUESTFISCALDOCUMENTTLV & 0xFF))) return -1;

	if ((a.len > 6) && (a.buff[2] == 0x00))
	{
		DocumentType 	= evalint(&a.buff[3], 2);
		DataLength 		= evalint(&a.buff[5], 2);
	#ifdef DEBUG
		printf("[FNRequestFiscalDocumentTLV]: DocumentType:%d  DataLength: %d\n", DocumentType, DataLength);
		fflush(stdout);
	#endif
	}

	if (errhand(&a) != 0) return ResultCode;

	return 0;
}
//-----------------------------------------------------------------------------
int DrvFR::FNReadFiscalDocumentTLV(void)
{
	int dataLength = DataLength;
	parameter  p;
	answer     a;
	memset(&p, 0, sizeof(p));
	memset(&a, 0, sizeof(a));

	if (!Connected) return -1;

	if (dataLength < 5) return -1;

	int index = 0;
	memset(TLVData, 0, sizeof(TLVData));
	#ifdef DEBUG
		printf("[FNReadFiscalDocumentTLV]: DataLength: %d\n", DataLength);
		fflush(stdout);
	#endif
	while (dataLength >= 0)
	{
		p.len = 0;

		if (conn->sendcommand(FN_FNREADFISCALDOCUMENTTLV, Password, &p) < 0) return -1;
		if (conn->readanswer(&a) < 0) return -1;

		if ((a.buff[0] != 0xFF) || (a.buff[1] != (FN_FNREADFISCALDOCUMENTTLV & 0xFF))) return -1;

		if (a.len == 3) return 1;

		memcpy((TLVData+index), &a.buff[3], (a.len - 3));
		index += (a.len - 3);
		dataLength -= (a.len - 3);
		int i = 3;
	#ifdef DEBUG
		printf("Buff [%d]:\n", a.len);
		while (i < a.len)
			{ printf (" %02X", a.buff[i]); i++; }
		printf("\n");
		fflush(stdout);
	#endif
		delay_ms(20);
	}

	if (errhand(&a) != 0) return ResultCode;

	return 0;
}
//-----------------------------------------------------------------------------
