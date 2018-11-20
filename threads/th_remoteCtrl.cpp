#include "../main.h"

//#pragma region Подсистема "Modbus удаленные контроллеры"

modbus_t *ctx;		// Дескриптор линии MODBUS


PI_THREAD(RemoteCtrlWatch)
{
	Settings* 		settings 	= Settings::getInstance();
	if (!settings->threadFlag.RemoteCtrlThread) return (void*)0;

	////******************************************************
	////******************************************************
	////******************************************************
	////******************************************************

	if (strlen(settings->modbus.portName) <= 0)
	{
		if (settings->debugFlag.RemoteCtrlThread)
			printf("[MODBUS] Interface port name not defined. Exit from thread.\n");
	}

	try{
   		ctx = modbus_new_rtu(settings->modbus.portName, settings->modbus.baudRate, settings->modbus.dataParity, settings->modbus.dataBit, settings->modbus.stopBit);
   	} catch (...)
   	{ printf("[MODBUS] Unable create MODBUS interface. Creation failed %s [baud: %d parity: %c bits: %d stop: %d].\n", settings->modbus.portName, settings->modbus.baudRate, (char)settings->modbus.dataParity, settings->modbus.dataBit, settings->modbus.stopBit); return (void*)0; }

	modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
   	modbus_flush(ctx);
   	modbus_set_slave(ctx, 1);
   	modbus_set_debug(ctx, (settings->debugFlag.RemoteCtrlThread != 0));

	int rc = modbus_connect(ctx);
	if (rc == -1)
	{
		if (settings->debugFlag.RemoteCtrlThread)
			printf("[MODBUS] Unable to connect %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return (void*)0;
	}

	while (settings->threadFlag.RemoteCtrlThread)
	{
		for(int index = 0; index < 30; index++)
			if (remoteCtrl[index].doCmd == 1)
			{
				printf("[MODBUS] Salve device %02d do command R:%d W:%d\n", index, remoteCtrl[index].cmdRead, remoteCtrl[index].cmdWrite);
			   	modbus_flush(ctx);
			   	modbus_set_slave(ctx, remoteCtrl[index].devId);
				remoteCtrl[index].cmdResult = 0;

			   	if (remoteCtrl[index].cmdRead)
			   	{
			   		rc = modbus_read_registers(ctx, 0, remoteCtrl[index].cmdReadPrm, (WORD*)&remoteCtrl[index].cmdResult);
					if (rc == -1)
					{
						if (settings->debugFlag.RemoteCtrlThread)
						printf("[MODBUS] Error read register %s\n", modbus_strerror(errno));
						remoteCtrl[index].cmdResult = 0xFFFFFFFF;
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

	modbus_close(ctx);
	modbus_free(ctx);
	printf("[RemoteCtrl]: Thread ended.\n");
	return (void*)0;
}

//#pragma endregion
