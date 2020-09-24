#include "../main.h"
//#pragma region Подсистема "пополнение с терминала"
PI_THREAD(TerminalCtrlThread)
{
	if (!settings->threadFlag.TerminalCtrlThread) return (void*)0;

	int serverFd, clientFd;
	struct sockaddr_in server, client;
	int len;
	int port = 3356;
	char buffer[1024];
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd < 0) 
	{
		perror("Cannot create socket");
		exit(1);
	}
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);
	len = sizeof(server);
	if (bind(serverFd, (struct sockaddr *)&server, len) < 0) 
	{
		perror("Cannot bind sokcet");
		exit(2);
	}
	if (listen(serverFd, 10) < 0) 
	{
		perror("Listen error");
		exit(3);
	}
	while (settings->threadFlag.TerminalCtrlThread > 0) 
	{

	len = sizeof(client);
	printf("waiting for clients\n");
	if ((clientFd = accept(serverFd, (struct sockaddr *)&client, (socklen_t*)&len)) < 0) 
	{
		perror("accept error");
		exit(4);
	}
	char *client_ip = inet_ntoa(client.sin_addr);
	printf("Accepted new connection from a client %s:%d\n", client_ip, ntohs(client.sin_port));

	while (settings->threadFlag.TerminalCtrlThread > 0) 
	{
		memset(buffer, 0, sizeof(buffer));
		int size = read(clientFd, buffer, sizeof(buffer));
		if ( size <= 0 ) 
		{
			perror("read error");
			break;
		}

		char param[10][25];
		bool cmdYes = 0;
		int chIndex = 0;
		memset(param, 0x00, sizeof(param));
		int bal;
		if (strlen(buffer) < 4) continue;

		char * pch = strtok (buffer," ");
		pch = strtok (NULL, " ");
		while ((pch != NULL) && (chIndex++ < 10))
		{
			strcpy(param[chIndex-1], pch);
      		pch = strtok (NULL, " ");
		}

		printf ("command: %s\n", buffer);
		for (int i = 0; i < chIndex-1; i++)
			printf ("param[%d]: %s\n", i, param[i]);

		if (strstr(buffer, "add") == buffer)
		{
			bal = atoi(param[0]);
			printf("Add money: %4d\n", bal);
			status.intDeviceInfo.money_currentBalance += bal;
			status.extDeviceInfo.remote_currentBalance += bal;
		}

		sprintf(buffer, "New balance: %d\n", status.intDeviceInfo.money_currentBalance);

		if (write(clientFd, buffer, size) < 0) 
		{
			perror("write error");
			break;
		}
	}
	close(clientFd);
	}
	close(serverFd);

	printf("[TERM CTRL]: Thread ended.\n");
	return (void*)0;
}
//#pragma endregion
