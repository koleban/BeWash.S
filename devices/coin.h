#ifndef _COIN_H_
#define _COIN_H_

#ifndef TRUE
#define TRUE (1==1)
#endif

#ifndef FALSE
#define FALSE (!TRUE)
#endif

#define BYTE				unsigned char
#define WORD				unsigned short
#define DWORD				unsigned int
#define SDWORD				int

#define MONEY_COIN_TYPE_COUNT	17

class CoinDevice
{
private:
    static CoinDevice* p_instance;
	CoinDevice();
    CoinDevice( const CoinDevice& );
    CoinDevice& operator=( CoinDevice& );

    bool		isOpened;				// Порт устройства открыт
    BYTE		eventId;
public:
    int 		comPortNumber;
	MoneyCoinInfo moneyCoinInfo;
	MoneyCoinInfo allMoneyCoinInfo;
	int delay_size;
	void Init(void* settings);
	bool IsOpened();
	bool OpenDevice();
	bool CloseDevice();
	bool cmdReset();
	bool cmdCheck();
	bool cmdSiplePoll();
	DWORD cmdRequestPollingPriority();
	BYTE cmdRequestStatus();
	DWORD cmdReadEventBuffer(BYTE* bufData);
	void cmdSetCoinSortPath(BYTE coinIndex, BYTE sortPath);
	bool cmdGetMasterInhibitStatus();
	bool cmdSetMasterInhibitStatus();
	void cmdModifyInhibitStatus(BYTE data1, BYTE data2);
	int Detect();
	void clearMoneyCoinInfo();
	static CoinDevice* getInstance()
   	{
        if(!p_instance)
            p_instance = new CoinDevice();
       	return p_instance;
   	}
};

#endif