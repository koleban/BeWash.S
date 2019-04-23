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
//#define DEBUG_CMD

/**********************************************************
 * Implementation of procedures                           *
 **********************************************************/
void DrvFR_Conn::PrintComm(command* cmd)
{
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
//		#ifdef DEBUG
		printf("DrvFR_Conn::opendev:\n  Password: %d\n  Timeout: %d\n  IP: %s\n", drvFR->Password, drvFR->Timeout, drvFR->IPAddress);
//		#endif
		struct sockaddr_in addr;
		devfile = socket(AF_INET, SOCK_STREAM, 0);
		signal(SIGPIPE, SIG_IGN);
		if (devfile < 0) return -1;

		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		int trueVal = 1;
		if (setsockopt (devfile, SOL_SOCKET, SO_REUSEADDR, (const char*)&trueVal,
                sizeof(trueVal)) < 0)
		{
			printf ("Error: Can't set SOCKET SO_REUSEADDR\n");
			perror("SO_REUSEADDR");
        	return -1;
        }

		trueVal = 1;
		if (setsockopt (devfile, SOL_SOCKET, SO_KEEPALIVE, (const char*)&trueVal,
                sizeof(trueVal)) < 0)
		{
			printf ("Error: Can't set SOCKET SO_KEEPALIVE\n");
			perror("SO_KEEPALIVE");
        	return -1;
        }

		if (setsockopt (devfile, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
		{
			printf ("Error: Can't set SOCKET RECV TIMEOUT\n");
			perror("RECV TIMEOUT");
        	return -1;
        }

		if (setsockopt (devfile, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
		{
			printf ("Error: Can't set SOCKET SEND TIMEOUT\n");
			perror("SEND TIMEOUT");
        	return -1;
        }

		addr.sin_family = AF_INET;
		addr.sin_port = htons(drvFR->TCPPort);
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		addr.sin_addr.s_addr = inet_addr(drvFR->IPAddress);
		int res = connect(devfile, (struct sockaddr *)&addr, sizeof(addr));

		if ( res < 0) { return -1; }
		connected = 1;
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
		#ifdef DEBUG_CMD
		printf(">> (1) %02X NAK\n", NAK);
		fflush(stdout);
		#endif
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
		#ifdef DEBUG_CMD
		printf(">> (1) %02X ACK\n", ACK);
		fflush(stdout);
		#endif
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
		#ifdef DEBUG_CMD
		printf(">> (1) %02X ENQ\n", ENQ);
		fflush(stdout);
		#endif
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
	if (len >= 5 && param->len > (len - 5 - FNCmd))
		param->len = len - 5 - FNCmd;
	#ifdef DEBUG_CMD
			printf("FN Command %4X: [LEN %d] Buffer LEN: [%d]\n", comm, len, param->len);
	#endif
	cmd->buff[0] = STX;
	cmd->buff[1] = len;
	if (FNCmd == 0)
		cmd->buff[2] = comm;
	else
	{
		cmd->buff[2] = (comm >> 8) & 0xFF;
		cmd->buff[3] = comm  & 0xFF;
	#ifdef DEBUG_CMD
		printf("FN Command with FN [%02X] [%02X]\n", cmd->buff[2], cmd->buff[3]);
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
			if (recv_bytes > 0)
			{
				#ifdef DEBUG_CMD
				printf("<< (%d) ", recv_bytes);
				for (int index=0; index < recv_bytes; index++)
					printf("%02X ", readbuff[index]);
				printf("  [readbyte]\n");
				fflush(stdout);
				#endif
				return readbuff[0];
			}
		}
		if ((netTimeout >= msec / 1000))
		{
			sendNAK();
			return 0xFF;
		}
	}
	return 0xFF;
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
		#ifdef DEBUG_CMD
		if (index > 0)
		{
			printf("<< (%d) ", index);
			for (int i=0; i < index; i++)
				printf("%02X ", buff[i]);
			printf("  [readbytes]\n");
			fflush(stdout);
		}
		#endif
		return index;
	}
	return 0;
}
//--------------------------------------------------------------------------------------
int DrvFR_Conn::checkstate(void)
{
	unsigned char readbuff[255];
	unsigned char repl;

	if (connected == 0) return -1;
	connected = sendENQ();
	usleep(100);
	return readbyte(drvFR->Timeout);
};
//--------------------------------------------------------------------------------------
int DrvFR_Conn::sendcommand(int comm, int pass, parameter *param)
{
	short int repl, tries;
	command cmd;
	memset(&cmd, 0, sizeof(cmd));
	composecomm(&cmd, comm, pass, param);
	int state = 0;
	answer      a;

	for (tries = 1; tries <= MAX_TRIES; tries++)
	{
		int send_status = -1;
		if (drvFR->ConnectionType == TConnectionType::ctLocal)
			send_status = write(devfile, cmd.buff, cmd.len);
		else if (drvFR->ConnectionType == TConnectionType::ctSocket)
		{
			#ifdef DEBUG_CMD
			if (cmd.len > 0)
			{
				printf(">> (%d) ", cmd.len);
				for (int index=0; index < cmd.len; index++)
					printf("%02X ", cmd.buff[index]);
				printf("  [sendcommand]\n");
				fflush(stdout);
			}
			#endif
			send_status = send(devfile, cmd.buff, cmd.len, 0);
		}
		if (send_status != -1)
		{
			repl = readbyte(drvFR->Timeout*2);
			if (repl == ACK) return 1;
			repl = readbyte(drvFR->Timeout);
			if (repl == ACK) return 1;
		};
	};
	return -1;
};
//--------------------------------------------------------------------------------------
int DrvFR_Conn::readanswer(answer *ans)
{
	answer tmp11;
	short int  len, crc, tries, repl;

	if (connected == 1)
	{
		memset(ans, 0, sizeof(tmp11));
		repl = readbyte(drvFR->Timeout);
		if (repl == STX)
		{
			len = readbyte(drvFR->Timeout);
			int recv_bytes = readbytes(&ans->buff[0], len) ;
			ans->len = recv_bytes;
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
					{
						sendNAK();
					}
				}
			}
			else
			{
				sendNAK();
			}
		}
		else
		{
			sendNAK();
		}

//		repl = readbyte(drvFR->Timeout);
//		if (connected != 1 || repl != ACK) tries++;
	}
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
			return len;
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
