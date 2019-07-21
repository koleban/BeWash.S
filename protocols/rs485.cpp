#include "../main.h"

//#define DEBUG

void RS485_getCRC(char *buf, int bufLen, char *crc)
{
	unsigned long crc_0 = 0xffff;
	unsigned long crc_1 = 0x0000;
	int i, j;
	for (i = 0; i < bufLen; i++)
	{
		crc_0 ^= ((unsigned long)buf[i] & 0x00ff);
		for (j = 0; j < 8; j++)
		{
			crc_1 = (crc_0 >> 1) & 0x7fff;

			if (crc_0 & 0x0001)
				crc_0 = (crc_1 ^ 0xa001);
			else
				crc_0 = crc_1;
		}
	}
	crc[0] = (unsigned char)((crc_0 / 256) & 0x00ff);
	crc[1] = (unsigned char)(crc_0 & 0x00ff);
}

void RS485_setRWMode(int mode)
{
	digitalWrite(RS485_RWPin, mode);
}

void RS485_ClearBuffer(int fd)
{
	serialFlush(fd);
	while (serialDataAvail(fd) > 0)
	{
		serialGetchar(fd);
		delay_ms(RS485_CommDelay);
	}
}

int RS485_doCommand(int fd, char *command)
{
	int count = 0;
	int result = 0;
	char devID = 0x01;
	char crc_out[2] = {0};
	char out_buff[2048] = {0};
	RS485_getCRC(&command[0], 6, &crc_out[0]);
	devID = command[0];

	setPinModeMy(RS485_RWPin, 0);	// PIN RS485_RWPin - Управление R/W режимом для RS485

	RS485_ClearBuffer(fd);

	RS485_setRWMode(RS485_WriteMode);
	serialPutchar(fd, command[0]);
	delay_ms(RS485_CommDelay);
	serialPutchar(fd, command[1]);
	delay_ms(RS485_CommDelay);
	serialPutchar(fd, command[2]);
	delay_ms(RS485_CommDelay);
	serialPutchar(fd, command[3]);
	delay_ms(RS485_CommDelay);
	serialPutchar(fd, command[4]);
	delay_ms(RS485_CommDelay);
	serialPutchar(fd, command[5]);
	delay_ms(RS485_CommDelay);
	serialPutchar(fd, crc_out[1]);
	delay_ms(RS485_CommDelay);
	serialPutchar(fd, crc_out[0]);
	delay_ms(RS485_CommDelay);
	serialFlush(fd);

	count = 0;
	RS485_setRWMode(RS485_ReadMode);
	int t = 0;
	while ((serialDataAvail(fd) == 0) && (t++ < 200)) delay_ms(1);
	while (serialDataAvail(fd) > 0)
	{
		if (count > (sizeof(out_buff) - 2)) break;
		out_buff[count++] = serialGetchar(fd);
		delay_ms(RS485_CommDelay);
	}

	for (int i = 0; ((i < 8) && (i < count)); i++)
		command[i] = out_buff[i];
	result = (count >= 5);
	if (!result) return result;
	RS485_getCRC(&command[0], count-2, &crc_out[0]);
#ifdef DEBUG
	printf("\nCRC: %02X %02X == %02X %02X\n", out_buff[count-2], out_buff[count-1], crc_out[0], crc_out[1]);
#endif
	result = ((out_buff[count-2] == crc_out[1]) && (out_buff[count-1] == crc_out[0]));
	return result;

/*
	RS485_getCRC(&command[0], 6, &crc_out[0]);
	result |= (((out_buff[6] ^ out_buff[7]) == (crc_out[0] ^ crc_out[1])) && (devID != command[0]));
	return result;
*/
}

int RS485_doCommandS(int fd, char *command, int size)
{
	int count;
	int result;
	char devID;
	char crc_out[2];
	char out_buff[2048];
	RS485_getCRC(&command[0], size-2, &crc_out[0]);
	devID = command[0];

///////////////////////////////////////
///////////////////////////////////////
#ifdef DEBUG
	printf("\nCMD: ");
	for(int mindex=0; mindex < size-2; mindex++)
		printf("[%02X] ", command[mindex]);
	printf("[%02X] ", crc_out[1]);
	printf("[%02X] ", crc_out[0]);
	printf("\n");
#endif
///////////////////////////////////////

	setPinModeMy(RS485_RWPin, 0);	// PIN RS485_RWPin - Управление R/W режимом для RS485

	RS485_ClearBuffer(fd);

	RS485_setRWMode(RS485_WriteMode);
	for (int i=0; i<size-2; i++ )
	{
		serialPutchar(fd, command[i]);
		delay_ms(RS485_CommDelay);
	}
	serialPutchar(fd, crc_out[1]);
	delay_ms(RS485_CommDelay);
	serialPutchar(fd, crc_out[0]);
	delay_ms(RS485_CommDelay);
	serialFlush(fd);

	count = 0;
	RS485_setRWMode(RS485_ReadMode);
	int t = 0;
	while ((serialDataAvail(fd) == 0) && (t++ < 200)) delay_ms(1);
	while (serialDataAvail(fd) > 0)
	{
		if (count > (sizeof(out_buff) - 2)) break;
		out_buff[count++] = serialGetchar(fd);
		delay_ms(RS485_CommDelay);
	}

	for (int i = 0; ((i < size) && (i < count)); i++)
		command[i] = out_buff[i];

///////////////////////////////////////
///////////////////////////////////////
#ifdef DEBUG
	printf("\nANS: ");
	for(int mindex=0; mindex < count; mindex++)
		printf("[%02X] ", command[mindex]);
	printf("\n");
#endif
///////////////////////////////////////

	
	result = 0;
	if ((count < 5) || (count > 20)) return result;
	if ((devID != command[0]) || ((command[0] + command[1]) == 0)) return result;
	result = 1;
	RS485_getCRC(&command[0], count-2, &crc_out[0]);
#ifdef DEBUG
	printf("\nCRC: %02X %02X == %02X %02X\n", out_buff[count-2], out_buff[count-1], crc_out[0], crc_out[1]);
#endif
	result = ((out_buff[count-2] == crc_out[1]) && (out_buff[count-1] == crc_out[0]));
//	if (!result)
//		printf("\nERROR CRC: %02X %02X == %02X %02X\n", out_buff[count-2], out_buff[count-1], crc_out[0], crc_out[1]);
	return result;
}
