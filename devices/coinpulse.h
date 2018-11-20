#ifndef _COINPULSE_H_
#define _COINPULSE_H_

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

class CoinPulseDevice
{
private:
    static CoinPulseDevice* p_instance;
	 CoinPulseDevice();
    CoinPulseDevice( const CoinPulseDevice& );
    CoinPulseDevice& operator=( CoinPulseDevice& );
public:
    BYTE pinCoinType1;
    BYTE pinCoinType2;
    BYTE pinCoinType3;
    BYTE pinCoinType4;
    BYTE pinCoinLock;
    int  sigWidth;
	MoneyCoinInfo moneyCoinInfo;
	MoneyCoinInfo allMoneyCoinInfo;
	void Init(Settings* settings);
	bool cmdPoll();
	bool Lock(bool newLockState);
	static CoinPulseDevice* getInstance()
   	{
        if(!p_instance)
            p_instance = new CoinPulseDevice();
       	return p_instance;
   	}
};

#endif