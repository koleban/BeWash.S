#ifndef _NETSERVER_H_
#define _NETSERVER_H_

#define CMD_GET_DEVICE_INFO			0x10
#define CMD_CLEAR_MONEY_COIN_INFO	0x11
#define CMD_SET_EXT_PRG				0x12
#define CMD_SEND_BALANCE			0x13

#define CMD_SRV_GET_DEVICE_INFO		0x20
#define CMD_SRV_ADD_MONEY			0x21
#define CMD_SRV_SET_EXT_PRG			0x22
#define CMD_SRV_RELOAD_APP			0x23
#define CMD_SRV_GET_APP_VER			0x24
#define CMD_SRV_GET_DEV_TYPE		0x25
#define CMD_SRV_GET_SERIAL_NUMBER	0x26	// Вернуть идентификатор
#define CMD_ADD_MONEY				0x27
#define CMD_RESTART_SERVICE			0x28
#define CMD_BLOCK					0x29
#define CMD_UNBLOCK					0x2A
		// 0xFF	-	Error
		// 0x01 -	threadFlag.ExtCommonThread
		// 0x02 -	threadFlag.IntCommonThread
		// 0x03 -	threadFlag.ExtCommonThread && threadFlag.IntCommonThread

#define CMD_SRV_DONWLOAD_CONFIG		0xA0
#define CMD_SRV_UPLOAD_CONFIG		0xA1
#define CMD_SRV_EXT_INKASS		0xC0	// External inkass (collection) command
class Settings;

class NetServer
{
	private:
		int port;
	public:
		NetServer();
	  	void Init(Settings* settings);
		bool StartServer();
		void StopServer();
};

#endif