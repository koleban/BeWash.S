#ifndef _RS485_H_
#define _RS485_H_

#define BAUND_RATE			9600
#define RS485_ReadMode 		0
#define RS485_WriteMode 	1
// 15000
#define RS485_CommDelay 	(int)((10000/BAUND_RATE) + 1)
#define RS485_RWPin			1

#define MODBUS_CMD_READ_HOLDING_REGISTER	0x03
#define MODBUS_CMD_WRITE_HOLDING_REGISTER	0x05
#define MODBUS_CMD_WRITE_HOLDING_REGISTERS	0x10
#define MODBUS_CMD_DIAG						0x08
#define MODBUS_CMD_REPORT_SLAVE_ID			0x11

typedef struct _Modbus_Prm_
{
	BYTE prmH;
	BYTE prmL;
} ModbusPrm;

typedef struct _Modbus_Command_
{
	BYTE slaveId;
	BYTE cmd;
	ModbusPrm regAddr;
	ModbusPrm regCount;
	BYTE dataCount;
	ModbusPrm prm[40];
} ModbusCommand;

void RS485_getCRC(char *buf, int bufLen, char *crc);
void RS485_setRWMode(int mode);
void RS485_ClearBuffer(int fd);
int RS485_doCommand(int fd, char *command);
int RS485_doCommandS(int fd, char *command, int size);

#endif