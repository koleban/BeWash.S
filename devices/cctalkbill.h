#ifndef _cctalkbill_h_
#define _cctalkbill_h_

#define MONEY_COIN_TYPE_COUNT	17
#define CCTB_ERROR_CODE_DEVICE_NO_ERROR			0x0000
#define CCTB_ERROR_CODE_DEVICE_ERROR_CONNECTION	0xFFFF
#define CCTB_ERROR_CODE_DEVICE_ERROR_ANSWER		0xFEFE

#define DWORD			unsigned int
#define WORD			unsigned short
#define BYTE			unsigned char
#define SDWORD			int

class CCTalkBillDevice
{
  private:
    static CCTalkBillDevice* p_instance;
	CCTalkBillDevice();
    CCTalkBillDevice( const CCTalkBillDevice& );
    CCTalkBillDevice& operator=( CCTalkBillDevice& );

    int		eventId;
public:
	char	model[16];
	char	serial[13];
    bool	isOpened;				// Порт устройства открыт
	int comPortBaundRate;
	int comPortNumber;
	int delay_size;
	BYTE currentEventId;
	MoneyCoinInfo moneyCoinInfo;
	MoneyCoinInfo allMoneyCoinInfo;
	void Init(void* settings);
	bool IsOpened();
	bool OpenDevice();
	bool CloseDevice();
	BYTE getCrc8(BYTE *data, BYTE dataLength);
	bool cmdReset();
	bool cmdGetStatus();
	bool cmdSetAcceptBillType(BYTE billType1, BYTE billType2, BYTE billType3);
	void cmdDispenseBill();
	DWORD cmdPoll();	// [FLAG][BILL_TYPE] - 2 bytes
	bool cmdASK();
	bool cmdModifyInhibit();
	bool cmdModifyMasterInhibit();
	bool cmdModifyBillMode();
	char* getErrorInfo(BYTE errorId);
	int Detect();
	static CCTalkBillDevice* getInstance()
   	{
        if(!p_instance)
            p_instance = new CCTalkBillDevice();
       	return p_instance;
   	}
};

#endif