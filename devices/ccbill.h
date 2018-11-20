#ifndef _ccbill_h_
#define _ccbill_h_

#define MONEY_COIN_TYPE_COUNT	17
union CashCodeID
{
	char 	billId[7];
	int 	digId[2];
};

class CCBillDevice
{
  private:
    static CCBillDevice* p_instance;
	CCBillDevice();
    CCBillDevice( const CCBillDevice& );
    CCBillDevice& operator=( CCBillDevice& );

    int		eventId;
public:
	char	model[16];
	char	serial[13];
	CashCodeID	cashCodeID;
    bool	isOpened;				// Порт устройства открыт
	int comPortBaundRate;
	int comPortNumber;
	int delay_size;
	MoneyCoinInfo moneyCoinInfo;
	MoneyCoinInfo allMoneyCoinInfo;
	void Init(void* settings);
	bool IsOpened();
	bool OpenDevice();
	bool CloseDevice();
	bool cmdReset();
	bool cmdCheck();
	bool cmdGetStatus();
	bool cmdSetAcceptBillType(BYTE billType1, BYTE billType2, BYTE billType3);
	void cmdDispenseBill();
	WORD cmdPoll();	// [FLAG][BILL_TYPE] - 2 bytes
	bool cmdASK();
	WORD cmdUnload();
	int Detect();
	static CCBillDevice* getInstance()
   	{
        if(!p_instance)
            p_instance = new CCBillDevice();
       	return p_instance;
   	}
};

#endif