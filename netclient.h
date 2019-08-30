#ifndef _NETCLIENT_H_
#define _NETCLIENT_H_
class Settings;

class NetClient
{
	private:
		int port;
		char hostAddr[20];
		int sock;
		struct sockaddr_in addr;
	public:
		bool isConnected;
		NetClient();
	  	void Init(Settings* settings);
		bool OpenConnection();
		void CloseConnection();
		bool cmdGetExtDeviceInfo();
		bool cmdSetExtPrg();
		bool cmdSendBalance(int balance, int* currentBalance);
};

#endif