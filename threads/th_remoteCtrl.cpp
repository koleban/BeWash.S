#include "../main.h"

//#pragma region Подсистема "Modbus удаленные контроллеры"

PI_THREAD(RemoteCtrlWatch)
{
	int ctx;
	ModbusCommand command;
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.RemoteCtrlThread) return (void*)0;

	////******************************************************
	////******************************************************
	////******************************************************
	////******************************************************

	if ( (strlen(settings->modbus.portName) <= 0) || (settings->modbus.baudRate == 0))
	{
		if (settings->debugFlag.RemoteCtrlThread)
			printf("[MODBUS] Interface port name or other parameters not defined. Exit from thread.\n");
	}
	
	try
	{
		ctx = serialOpen(settings->modbus.portName, settings->modbus.baudRate);
   } catch (...)
   { printf("[MODBUS] Unable create MODBUS interface. Creation failed %s [baud: %d parity: %c bits: %d stop: %d].\n", settings->modbus.portName, settings->modbus.baudRate, (char)settings->modbus.dataParity, settings->modbus.dataBit, settings->modbus.stopBit); return (void*)0; }

	pinMode(RS485_RWPin, OUTPUT);			// PIN RS485_RWPin - Управление R/W режимом для RS485
	RS485_ClearBuffer(ctx);

	while (settings->threadFlag.RemoteCtrlThread)
	{
		for(int index = 0; index < 30; index++)
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
					printf("[MODBUS] Salve device %02d do command READ\n", index);

					memset(&command, 0, sizeof(command));
					command.slaveId = devId;
					command.cmd = MODBUS_CMD_READ_HOLDING_REGISTER;
					// 
					command.regAddr.prmH = 0;
					command.regAddr.prmL = 10;
					command.regCount.prmH = 0;
					command.regCount.prmL = 1;
					if (!RS485_doCommandS(ctx, (char*)&command, 8))
					{
						remoteCtrl[index].cmdResult = 0xFFFFFFFF;
						if (settings->debugFlag.RemoteCtrlThread)
							printf("[MODBUS] Error read (0x03) register %d [%X]\n", command.cmd, command.cmd);
					}
					else
					{
						if (command.cmd != MODBUS_CMD_READ_HOLDING_REGISTER)
						{
							remoteCtrl[index].cmdResult = 0xFFFFFFFF;
							if (settings->debugFlag.RemoteCtrlThread)
								printf("[MODBUS] Error read (0x03) register %d [%X]\n", command.cmd, command.cmd);
						}
						else
						{
							remoteCtrl[index].cmdResult = (command.regAddr.prmL << 8) + command.regCount.prmH;
							printf("[MODBUS] Slave device result: %08X\n", remoteCtrl[index].cmdResult);
						}
					}
				}

				///////////////////////////////////////////////////////
				// Command WRITE
				///////////////////////////////////////////////////////
		   	if (remoteCtrl[index].cmdWrite)
				{
					printf("[MODBUS] Salve device %02d do command WRITE\n", index);
					memset(&command, 0, sizeof(command));
					command.slaveId = devId;
					command.cmd = MODBUS_CMD_WRITE_HOLDING_REGISTERS;
					command.regAddr.prmH = 0;
					command.regAddr.prmL = SLAVE_REG_IMPULSE_COUNT_1;
					command.regCount.prmH = 0;
					command.regCount.prmL = 4;
					command.dataCount = 8;
					for (int j=0; j<4; j++)
					{
						command.prm[j].prmH = (BYTE)((remoteCtrl[index].devImpVal[j] >> 8) & 0xFF);
						command.prm[j].prmL = (BYTE)(remoteCtrl[index].devImpVal[j] & 0xFF);
					}
					if (!RS485_doCommandS(ctx, (char*)&command, 7+command.dataCount+2))
					{
						remoteCtrl[index].cmdResult = 0xFFFFFFFFUL;
						if (settings->debugFlag.RemoteCtrlThread)
							printf("[MODBUS] Error write (0x10) register %d [%X]\n", command.cmd, command.cmd);
					}
					else
					{
						if (command.cmd != MODBUS_CMD_WRITE_HOLDING_REGISTERS)
						{
							remoteCtrl[index].cmdResult = 0xFFFFFFFFUL;
							if (settings->debugFlag.RemoteCtrlThread)
								printf("[MODBUS] Error write (0x10) register %d [%X]\n", command.cmd, command.cmd);
						}
						else
						{
							remoteCtrl[index].cmdResult = (command.regAddr.prmH << 8) + command.regAddr.prmL;
							printf("[MODBUS] Slave device result: %08X\n", remoteCtrl[index].cmdResult);
						}
					}
				}

				remoteCtrl[index].doCmd = 0;
				remoteCtrl[index].cmdCheck = 0;
				remoteCtrl[index].cmdRead = 0;
				remoteCtrl[index].cmdWrite = 0;
				remoteCtrl[index].cmdReadPrm = 0;
				remoteCtrl[index].cmdWritePrm = 0;
			}
		delay_ms(500);
	}

	RS485_ClearBuffer(ctx);
	close(ctx);
	printf("[RemoteCtrl]: Thread ended.\n");
	return (void*)0;
}

//#pragma endregion
