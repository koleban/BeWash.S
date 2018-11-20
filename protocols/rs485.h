#ifndef _RS485_H_
#define _RS485_H_

#define BAUND_RATE			9600
#define RS485_ReadMode 		0
#define RS485_WriteMode 	1
// 15000
#define RS485_CommDelay 	(int)((10000/BAUND_RATE) + 1)
#define RS485_RWPin			1

void RS485_getCRC(char *buf, int bufLen, char *crc);
void RS485_setRWMode(int mode);
void RS485_ClearBuffer(int fd);
int RS485_doCommand(int fd, char *command);
int RS485_doCommandS(int fd, char *command, int size);

#endif