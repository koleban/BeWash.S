#ifndef _NETCLIENT_H_
#define _NETCLIENT_H_
class Settings;

class NetClient
{
	public:
		int port;
		char hostAddr[20];
		int sock;
		struct sockaddr_in addr;
		bool isConnected;
		NetClient();
	  	void Init(Settings* settings);
		bool OpenConnection();
		void CloseConnection();
		bool cmdGetExtDeviceInfo();
		bool cmdSetExtPrg();
		bool cmdSendBalance();
};

#endif