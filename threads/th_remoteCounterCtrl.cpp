#include "../main.h"

//#pragma region Подсистема "Modbus удаленные контроллеры"

PI_THREAD(RemoteCounterCtrlWatch)
{
	int ctx;
	ModbusCommand command;
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.RemoteCounterCtrlThread) return (void*)0;

	////******************************************************
	////******************************************************
	////******************************************************
	////******************************************************

	if ( (strlen(settings->modbus.portName) <= 0) || (settings->modbus.baudRate == 0))
	{
		if (settings->debugFlag.RemoteCounterCtrlThread)
			printf("[MODBUS] Interface port name or other parameters not defined. Exit from thread.\n");
	}

	try
	{
		ctx = serialOpen(settings->modbus.portName, settings->modbus.baudRate);
   } catch (...)
   { printf("[MODBUS] Unable create MODBUS interface. Creation failed %s [baud: %d parity: %c bits: %d stop: %d].\n", settings->modbus.portName, settings->modbus.baudRate, (char)settings->modbus.dataParity, settings->modbus.dataBit, settings->modbus.stopBit); return (void*)0; }

	pinMode(RS485_RWPin, OUTPUT);			// PIN RS485_RWPin - Управление R/W режимом для RS485
	RS485_ClearBuffer(ctx);

	while (settings->threadFlag.RemoteCounterCtrlThread)
	{
		for(int index = 1; index <= settings->modbus.slaveCount; index++)
			if (remoteCtrl[index].doCmd == 1)
			{
		   		int devId = remoteCtrl[index].devId;
				RS485_ClearBuffer(ctx);
				remoteCtrl[index].cmdResult = 0;

				///////////////////////////////////////////////////////
				// Command READ
				///////////////////////////////////////////////////////
		   		if (remoteCtrl[index].cmdRead)
				{
					printf("[MODBUS] Salve device %02d do command READ COUNTER 1\n", index);

					memset(&command, 0, sizeof(command));
					command.slaveId = devId;
					command.cmd = MODBUS_CMD_READ_HOLDING_REGISTER;
					//
					command.regAddr.prmH = 0;
					command.regAddr.prmL = 11;
					command.regCount.prmH = 0;
					command.regCount.prmL = 1;
					if (!RS485_doCommandS(ctx, (char*)&command, 8))
					{
						remoteCtrl[index].cmdResult = 0xFFFFFFFF;
						if (settings->debugFlag.RemoteCounterCtrlThread)
							printf("[MODBUS] [1] Error read (0x03) register %d [%X]\n", command.regAddr.prmL, command.regAddr.prmL);
					}
					else
					{
						if ((command.cmd != MODBUS_CMD_READ_HOLDING_REGISTER) || (command.slaveId != devId))
						{
							remoteCtrl[index].cmdResult = 0xFFFFFFFF;
							if (settings->debugFlag.RemoteCounterCtrlThread)
								printf("[MODBUS] [1] Error read (0x03) register %d [%X] dev: %d [%d]\n", command.regAddr.prmL, command.regAddr.prmL, devId, command.slaveId);
						}
						else
						{
							remoteCtrl[index].cmdResult = (command.regAddr.prmL << 8) + command.regCount.prmH;
							if (remoteCtrl[index].cmdResult < 0xFFFFFF)
								remoteCounter[index][0] = (DWORD)(remoteCtrl[index].cmdResult & 0xFFFF);
							printf("[MODBUS] [1] Slave device result: %08X Counter: %d\n", remoteCtrl[index].cmdResult, remoteCounter[index][0]);
						}
					}

					delay_ms(10);

					printf("[MODBUS] Salve device %02d do command READ COUNTER 2\n", index);

					memset(&command, 0, sizeof(command));
					command.slaveId = devId;
					command.cmd = MODBUS_CMD_READ_HOLDING_REGISTER;
					//
					command.regAddr.prmH = 0;
					command.regAddr.prmL = 12;
					command.regCount.prmH = 0;
					command.regCount.prmL = 1;
					if (!RS485_doCommandS(ctx, (char*)&command, 8))
					{
						remoteCtrl[index].cmdResult = 0xFFFFFFFF;
						if (settings->debugFlag.RemoteCounterCtrlThread)
							printf("[MODBUS] [2] Error read (0x03) register %d [%X]\n", command.regAddr.prmL, command.regAddr.prmL);
					}
					else
					{
						if ((command.cmd != MODBUS_CMD_READ_HOLDING_REGISTER) || (command.slaveId != devId))
						{
							remoteCtrl[index].cmdResult = 0xFFFFFFFF;
							if (settings->debugFlag.RemoteCounterCtrlThread)
								printf("[MODBUS] [2] Error read (0x03) register %d [%X] dev: %d [%d]\n", command.regAddr.prmL, command.regAddr.prmL, devId, command.slaveId);
						}
						else
						{
							remoteCtrl[index].cmdResult = (command.regAddr.prmL << 8) + command.regCount.prmH;
							if (remoteCtrl[index].cmdResult < 0xFFFFFF)
								remoteCounter[index][1] = (DWORD)(remoteCtrl[index].cmdResult & 0xFFFF);
							printf("[MODBUS] [2] Slave device result: %08X Counter: %d\n", remoteCtrl[index].cmdResult, remoteCounter[index][1]);
						}
					}

					remoteCtrl[index].doCmd = 0;
					remoteCtrl[index].cmdCheck = 0;
					remoteCtrl[index].cmdRead = 0;
					remoteCtrl[index].cmdWrite = 0;
					remoteCtrl[index].cmdReadPrm = 0;
					remoteCtrl[index].cmdWritePrm = 0;
					continue;
				}

				///////////////////////////////////////////////////////
				// Command WRITE
				///////////////////////////////////////////////////////
		   		if (remoteCtrl[index].cmdWrite)
				{
					printf("[MODBUS] Salve device %02d do command WRITE COUNTER 1\n", index);
					memset(&command, 0, sizeof(command));
					command.slaveId = devId;
					command.cmd = MODBUS_CMD_WRITE_HOLDING_REGISTERS;
					command.regAddr.prmH = 0;
					command.regAddr.prmL = 11;
					command.regCount.prmH = 0;
					command.regCount.prmL = 2;
					command.dataCount = 4;
					for (int j=0; j<2; j++)
					{
						command.prm[j].prmH = (BYTE)((remoteCounter[index][j] >> 8) & 0xFF);
						command.prm[j].prmL = (BYTE)(remoteCounter[index][j] & 0xFF);
					}
					if (!RS485_doCommandS(ctx, (char*)&command, 7+command.dataCount+2))
					{
						remoteCtrl[index].cmdResult = 0xFFFFFFFFUL;
						if (settings->debugFlag.RemoteCounterCtrlThread)
							printf("[MODBUS] Error write (0x10) register %d [%X] reg count %d\n", command.regAddr.prmL, command.regAddr.prmL, command.regCount.prmL);
					}
					else
					{
						if (command.cmd != MODBUS_CMD_WRITE_HOLDING_REGISTERS)
						{
							remoteCtrl[index].cmdResult = 0xFFFFFFFFUL;
							if (settings->debugFlag.RemoteCounterCtrlThread)
								printf("[MODBUS] Error write (0x10) register %d [%X] reg count %d\n", command.regAddr.prmL, command.regAddr.prmL, command.regCount.prmL);
						}
						else
						{
							remoteCtrl[index].cmdResult = (command.regAddr.prmH << 8) + command.regAddr.prmL;
							printf("[MODBUS] Slave device result: %08X\n", remoteCtrl[index].cmdResult);
						}
					}

					remoteCtrl[index].doCmd = 0;
					remoteCtrl[index].cmdCheck = 0;
					remoteCtrl[index].cmdRead = 0;
					remoteCtrl[index].cmdWrite = 0;
					remoteCtrl[index].cmdReadPrm = 0;
					remoteCtrl[index].cmdWritePrm = 0;
				}

			} 	// if (remoteCtrl[index].doCmd == 1)
		delay_ms(100);
	}

	RS485_ClearBuffer(ctx);
	close(ctx);
	printf("[RemoteCounterCtrlThread]: Thread ended.\n");
	return (void*)0;
}

//#pragma endregion
