#pragma once
#ifndef _DRVFR_CONN_H
#define _DRVFR_CONN_H
#include <termios.h>
#include <sys/select.h>
#include "DrvFR.h"

namespace DriverFR
{

#define MAX_TRIES	10

#define ENQ 			0x05
#define STX 			0x02
#define ACK 			0x06
#define NAK 			0x15


/**********************************************************
 *  The length of commands                                *
 **********************************************************/

const unsigned commlen[0x100] =
{//0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
   0,  6,  5,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x00 - 0x0f
   0,  5, 26,  5,  8,  6,  1, 46, 37,  6,  6,  6, 10,  5,255,  9, // 0x10 - 0x1f
   6,  8,  8,  8,  5,  6,  0,  5,  6,  7,  5,  5,  5,  6,  7,  47, // 0x20 - 0x2f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x30 - 0x3f
   5,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x40 - 0x4f
  10, 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x50 - 0x5f
   9,  1,  6,  5,  5, 20, 12, 10,  5,  6,  0,  0,  0,  0,  0,  0, // 0x60 - 0x6f
 255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x70 - 0x7f
  60, 60, 60, 60, 60, 71, 54, 54,  5,  5, 54, 54,  0,  6,  131,  0, // 0x80 - 0x8f
  61, 57, 52, 11, 12, 52,  7,  7,  7,  5, 13,  7,  0,  0,  7,  7, // 0x90 - 0x9f
  13, 11, 12, 10, 10,  8,  7,  5,  0,  0,  0,  0,  0,  0,  0,  0, // 0xa0 - 0xaf
   5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xb0 - 0xbf
  46,  7, 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xc0 - 0xcf
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xd0 - 0xdf
   5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xe0 - 0xef
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,255,  0,  0  // 0xf0 - 0xff
};

/// STX, LEN [ CMD1 CMD2 XX1 XX2 XX3 XX4 ...] CRC
/// LEN ALL CMD and XXx
/// LEN = 6 + param->len
const unsigned fncommlen[0x100] =
{//0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
   0,  6,  0,  0,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0, // 0x00 - 0x0f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,// 0x10 - 0x1f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x20 - 0x2f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  6, 10,  6,  0,  0,  0,  0, // 0x30 - 0x3f
   0,  0,  0,  0,  0,  119,  160,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x40 - 0x4f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x50 - 0x5f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x60 - 0x6f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x70 - 0x7f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x80 - 0x8f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0x90 - 0x9f
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xa0 - 0xaf
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,// 0xb0 - 0xbf
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xc0 - 0xcf
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xd0 - 0xdf
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0xe0 - 0xef
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 // 0xf0 - 0xff
};

enum { s2400 = 0, s4800, s9600, s19200, s38400, s57600, s115200 };

enum { WAIT_READ = 0x01, WAIT_WRITE };

enum TConnectionType {ctLocal = 0, ctServerKKM_TCP, ctServerKKM_DCOM, ctEscape, ctUndef1, ctEmulator, ctSocket};

const speed_t LineSpeedVal[7] =
{ B2400, B4800, B9600, B19200, B38400, B57600, B115200 };

typedef struct
{
	int len;
	unsigned char buff[260];
} command;

typedef struct
{
	int len;
	unsigned char buff[2096];
} answer;

typedef struct
{
	int len;
	unsigned char buff[260];
} parameter;

class DrvFR_Conn
{
	/**************************************************/
private:
	int devfile;
	int fdflags;
	DrvFR_Conn() {}
	DrvFR* drvFR = NULL;
public:
	DrvFR_Conn(DrvFR* drv) { drvFR = drv; }
	int connected;
	fd_set set;
	struct timeval timeout;
	/**************************************************/
	int sendNAK(void);
	int sendACK(void);
	int sendENQ(void);
	void PrintComm(command*);

	int set_up_tty(int);
	int input_timeout(int);
	int composecomm(command *cmd, int comm, int pass, parameter *param);
	void settimeout(int);
	char* devname(int number);
	unsigned char readbyte(int);
	int readbytes(unsigned char *buff, int len);
	unsigned short int LRC(unsigned char*, int, int);

	int opendev();
	int closedev(void);
	int readanswer(answer*);
	int sendcommand(int, int, parameter*);
	int checkstate(void);
	int clearanswer(void);
};

}
#endif
