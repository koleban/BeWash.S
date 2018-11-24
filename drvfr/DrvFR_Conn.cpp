#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "DrvFR_Conn.h"

using namespace DriverFR;

//#define DEBUG

/**********************************************************
 * Implementation of procedures                           *
 **********************************************************/
void DrvFR_Conn::PrintComm(command* cmd)
{
#ifdef DEBUG
	char *s;
	char *t;

	s = (char*)malloc(cmd->len * 3 * 3);
	t = (char*)malloc(5);
	memset(s, 0, cmd->len * 3 * 3);
	for (int i = 0; i < cmd->len; i++)
	{
		sprintf(t, "%02X|", (int)cmd->buff[i]);
		s = strcat(s, t);
	}
	s = strcat(s, "\n");
	printf("%s", s);
#endif
};
//--------------------------------------------------------------------------------------
void DrvFR_Conn::settimeout(int msec)
{
	if (msec <= 1000)
	{
		timeout.tv_sec = 0;
		timeout.tv_usec = msec * 1000;
	}
	else
	{
		timeout.tv_sec = msec / 1000;
		timeout.tv_usec = (msec - timeout.tv_sec * 1000) * 1000;
	};
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::opendev()
{
	if (drvFR->ConnectionType == TConnectionType::ctLocal)
	{
		while ((devfile = open(devname(drvFR->ComNumber), O_NONBLOCK | O_RDWR, 0)) < 0)
			if (errno != EINTR) return -1;
		if ((fdflags = fcntl(devfile, F_GETFL)) == -1 || fcntl(devfile, F_SETFL, fdflags & ~O_NONBLOCK) < 0) return -1;
		set_up_tty(devfile);
		FD_ZERO(&set);
		FD_SET(devfile, &set);
		settimeout(drvFR->Timeout);
	}
	else if (drvFR->ConnectionType == TConnectionType::ctSocket)
	{
		struct sockaddr_in addr;
		devfile = socket(AF_INET, SOCK_STREAM, 0);
		signal(SIGPIPE, SIG_IGN);
		if (devfile < 0) return -1;

		struct timeval timeout;      
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		if (setsockopt (devfile, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        return -1;

		if (setsockopt (devfile, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        return -1;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(drvFR->TCPPort); 
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		addr.sin_addr.s_addr = inet_addr(drvFR->IPAddress);
		int res = connect(devfile, (struct sockaddr *)&addr, sizeof(addr));
		
		if ( res < 0) { return -1; }
	}
	else
	{
		return -1;
	}
	return 1;
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::set_up_tty(int tty_fd)
{
	int speed;
	struct termios tios;

	if (tcgetattr(tty_fd, &tios) < 0) return -1;

	tios.c_cflag &= ~(CSIZE | CSTOPB | PARENB | CLOCAL);
	tios.c_cflag |= CS8 | CREAD | HUPCL;

	tios.c_iflag = IGNBRK | IGNPAR;
	tios.c_oflag = 0;
	tios.c_lflag = 0;
	tios.c_cc[VMIN] = 1;
	tios.c_cc[VTIME] = 0;

	tios.c_cflag &= ~CRTSCTS;

	speed = LineSpeedVal[drvFR->BaudRate];
	if (speed)
	{
		cfsetospeed(&tios, speed);
		cfsetispeed(&tios, speed);
	}

	if (tcsetattr(tty_fd, TCSAFLUSH, &tios) < 0) return -1;

	return 1;
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::closedev(void)
{
	return close(devfile);
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::input_timeout(int msec)
{
	if (msec > 0) settimeout(msec);
	connected = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &timeout));
	return connected;
}
//--------------------------------------------------------------------------------------
unsigned short int DrvFR_Conn::LRC(unsigned char *str, int len, int offset = 0)
{
	int i;
	unsigned char *ptr;
	unsigned char ch = 0;

	ptr = str + offset;
	for (i = 0; i < len; i++)ch ^= ptr[i];
	return ch;
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::sendNAK(void)
{
	char buff[2];
	buff[0] = NAK;
	if (drvFR->ConnectionType == TConnectionType::ctLocal)
		return write(devfile, buff, 1);
	else if (drvFR->ConnectionType == TConnectionType::ctSocket)
	{
		usleep(100); 
		return send(devfile, buff, 1, 0);
	}
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::sendACK(void)
{
	char buff[2];
	buff[0] = ACK;
	if (drvFR->ConnectionType == TConnectionType::ctLocal)
		return write(devfile, buff, 1);
	else if (drvFR->ConnectionType == TConnectionType::ctSocket)
	{
		usleep(100); 
		return send(devfile, buff, 1, 0);
	}
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::sendENQ(void)
{
	char buff[2];
	buff[0] = ENQ;
	if (drvFR->ConnectionType == TConnectionType::ctLocal)
		return write(devfile, buff, 1);
	else if (drvFR->ConnectionType == TConnectionType::ctSocket)
	{
		usleep(100); 
		return send(devfile, buff, 1, 0);
	}
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::composecomm(command *cmd, int comm, int pass, parameter *param)
{
	int len = 0;

	int FNCmd = 0;
	if (comm > 0xFF) 
	{
		FNCmd = 1;
		len = fncommlen[(comm & 0xFF)];
	}
	else
			len = commlen[comm];
	#ifdef DEBUG
			printf("FN Command [LEN %d] Buffer LEN: [%d]\n", len, param->len);
	#endif
	if (len >= 5 && param->len > (len - 5 - FNCmd)) 
		param->len = len - 5 - FNCmd;
	cmd->buff[0] = STX;
	cmd->buff[1] = len;
	if (FNCmd == 0)
		cmd->buff[2] = comm;
	else
	{
		cmd->buff[2] = (comm >> 8) & 0xFF;
		cmd->buff[3] = comm  & 0xFF;
	#ifdef DEBUG
		printf("FN Command [%02X] [%02X]\n", cmd->buff[2], cmd->buff[3]);
	#endif
	}
	if (len >= 5)
	{
		memcpy(cmd->buff + 3 + FNCmd, &pass, sizeof(int));
		if (param->len > 0) memcpy(cmd->buff + 7 + FNCmd, param->buff, param->len);
	};
	cmd->buff[len + 2 ] = LRC(cmd->buff, len + 1 + FNCmd, 1);
	cmd->len = len + 3;
	return 1;
}
//--------------------------------------------------------------------------------------
unsigned char DrvFR_Conn::readbyte(int msec)
{
	unsigned char readbuff[2] = "";
	if (drvFR->ConnectionType == TConnectionType::ctLocal)
	{
		if (input_timeout(msec) == 0) { return 0;}
		if (read(devfile, readbuff, 1) > 0) return (unsigned int)readbuff[0];
		else return 0;
	}
	else if (drvFR->ConnectionType == TConnectionType::ctSocket)
	{
		int netTimeout = 0;
		while (netTimeout++ < (msec / 1000))
		{
			int recv_bytes = recv(devfile, readbuff, 1, 0);
			if (recv_bytes > 0)  return readbuff[0];
		}
		if ((netTimeout >= msec / 1000))
			sendNAK();
	}
	return 0;
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::readbytes(unsigned char *buff, int len)
{
	int i;
	if (drvFR->ConnectionType == TConnectionType::ctLocal)
	{
		for (i = 0; i < len; i++)
		{
			if (input_timeout(drvFR->Timeout) == 1)
				if (read(devfile, buff + i, 1) == -1) return -1;
		}
		return len;
	}
	else if (drvFR->ConnectionType == TConnectionType::ctSocket)
	{
		int index = 0;
		int bytes = 0;
		int netTimeout = 0;
		while (netTimeout++ < (drvFR->Timeout / 1000))
		{
			bytes = recv(devfile, (buff + index), len - index, MSG_DONTWAIT);
			if (bytes > 0) { index += bytes; netTimeout = 0; if (bytes >= len) break; }
		}
		return bytes;
	}
	return 0;
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::checkstate(void)
{
	unsigned char repl;

	connected = sendENQ();
	repl = readbyte(drvFR->Timeout);
	if (connected == 0) return -1;
	switch (repl)
	{
	case NAK:
	#ifdef DEBUG
	//	printf ("recv NAK\n");
	#endif
		return NAK;
	case ACK:
	#ifdef DEBUG
	//	printf ("recv ACK\n");
	#endif
		return ACK;
	default:
		return -1;
	};
};
//--------------------------------------------------------------------------------------
int DrvFR_Conn::sendcommand(int comm, int pass, parameter *param)
{
	short int repl, tries;
	command cmd;
	memset(&cmd, 0, sizeof(cmd));
	composecomm(&cmd, comm, pass, param);
#ifdef DEBUG
	PrintComm(&cmd);
#endif

	int state = 0;
	answer      a;

	tries = 1;
	bool flag = false;
	while (tries < MAX_TRIES && !flag)
	{
		state = checkstate();
		switch (state)
		{
		case NAK:
			flag = true;
			break;
		case ACK:
			readanswer(&a);
			//clearanswer();
			flag = true;
			break;
		case -1:
			tries++;
		};
	};

	if (!flag)
		return -1;

	for (tries = 1; tries <= MAX_TRIES; tries++)
	{
		int send_status = -1;
		if (drvFR->ConnectionType == TConnectionType::ctLocal)
			send_status = write(devfile, cmd.buff, cmd.len);
		else if (drvFR->ConnectionType == TConnectionType::ctSocket)
			send_status = send(devfile, cmd.buff, cmd.len, 0);
		if (send_status != -1)
		{
			repl = readbyte(drvFR->Timeout * 100);
			if (connected != 0)
			{
				if (repl == ACK) 
					return 1;
			};
		};
	};
	return -1;
};
//--------------------------------------------------------------------------------------
int DrvFR_Conn::readanswer(answer *ans)
{
	short int  len, crc, tries, repl;
	for (tries = 0; tries < MAX_TRIES; tries++)
	{
		repl = readbyte(drvFR->Timeout * 100);
		if (connected == 1)
		{
			if (repl == STX)
			{
				len = readbyte(drvFR->Timeout);
				if (connected == 1)
				{
					int recv_bytes = readbytes(ans->buff, len) ;
					#ifdef DEBUG
					printf ("readanswer: rcv bytes %d - %d\n", recv_bytes, len);
					#endif
					if ( recv_bytes == len)
					{
						crc = readbyte(drvFR->Timeout);
						if (connected != 0)
						{
							if (crc == (LRC(ans->buff, len, 0) ^ len))
							{
								sendACK();
								ans->len = len;
								return len;
							}
							else
								sendNAK();
						}
					}
				}
			}
		}
		sendENQ();
		repl = readbyte(drvFR->Timeout * 2);
		if (connected != 1 || repl != ACK) tries++;
	};
	return -1;
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::clearanswer(void)
{
	short int len;
	unsigned char buf[0x100];

	sendENQ();
	len = readbytes(buf, 0x100);
	if (connected == 1 && len > 0)
	{
		if (buf[0] != NAK)
		{
			sendACK();
			return 1;
		};
	};
	return 0;
}
//--------------------------------------------------------------------------------------
char* DrvFR_Conn::devname(int number)
{
	static char *devn[] =
	{
	  "/dev/null",
	  "/dev/ttyS0",
	  "/dev/ttyS1",
	  "/dev/ttyS2",
	  "/dev/ttyS3"
	};
	if (number > 0 && number < 5) return devn[number];
	return devn[0];
}
//--------------------------------------------------------------------------------------
