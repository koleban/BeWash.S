#include "../main.h"

PI_THREAD(AlienDeviceWatch)
{
	/// ����� ���������
	Settings* settings = Settings::getInstance();
	/// ���� ����� ��������, �� �����������
	if (!(settings->threadFlag.AlienDeviceThread)) return (void*)0;
	Database* db = new Database();
	db->Init(settings);
	db->Log(DB_EVENT_TYPE_THREAD_INIT, 		0, 0, "[THREAD] Alien device control: AlienDevice thread init");

	if (settings->debugFlag.AlienDeviceThread)
		printf("[DEBUG] AlienDeviceThread: Debug information is showed\n");

	while (settings->threadFlag.AlienDeviceThread)
	{
		int delayTime = 100;
		settings->workFlag.AlienDeviceThread = 0;

		for (index = 0; index < settings->modbus.slaveCount; index++)
		{
			int slaveId = index;

			if ((remoteCounterSumm[index][0] + remoteCounterSumm[index][1]) > 0)
			{
				time_t rcv_timer_out;
				time(&rcv_timer_out);

				if ((((DWORD)rcv_timer_out) - remoteCounterSumm[index][2]) > 60)
				{
					// Add information for print KKM documents
					// queueKkm->QueuePut( CashSumm, DON'T USED, DON'T USED, ServiceName);
					//
					DWORD sendedBal = remoteCounterSumm[index][0] + remoteCounterSumm[index][1]*10;
					char serviceNote[256];
					sprintf(serviceNote, "%s (���� N %d)", settings->kkmParam.ServiceName, slaveId+1);
					if (settings->debugFlag.AlienDeviceThread)
						printf("[DEBUG] AlienDeviceThread: Add KKM check in queue [%s on %d rur]\n" serviceNote, sendedBal);
					queueKkm->QueuePut(sendedBal, 0, 0, serviceNote);
				}
			}

			// Add to structure remoteCounterCtrl information for send to slave ctrl other modbus protocole
			//
			// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			int retryCounter = 3;
			remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
			int timeout = 1000;
			while ((remoteCtrl[slaveId].cmdResult > 0xFFFF) && (retryCounter-- > 0))
			{
				printf ("          [1] READ COMMAND COUNTER 1\n");
				remoteCtrl[slaveId].cmdRead = 1;
				remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
				remoteCtrl[slaveId].doCmd = 1;
				while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
			}
			printf ("          [1] COMMAND RESULT: %08X (timeout: %d) Counter [1 rur, 10 rur]: [%d, %d]\n", remoteCtrl[slaveId].cmdResult, timeout, remoteCounter[index][0], remoteCounter[index][1]);
			if ((remoteCtrl[slaveId].cmdResult > 0xFFFFUL) || (timeout < 0)) continue;

			// If we received ZERO in answer. Continue.
			if ((remoteCounter[index][0] + remoteCounter[index][1]) == 0) { delay_ms(100); continue;}

			//
			// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			printf ("              WRITE COMMAND COUNTER\n");
			remoteCtrl[slaveId].cmdResult = 0xFFFFFFFFUL;
			remoteCtrl[slaveId].cmdWrite = 1;
			remoteCtrl[slaveId].devImpVal[0] = remoteCounter[index][0];	// 1 rur
			remoteCtrl[slaveId].devImpVal[1] = remoteCounter[index][1];	// 10  rur
			remoteCtrl[slaveId].doCmd = 1;
			timeout = 1000;
			while ((remoteCtrl[slaveId].doCmd) && (timeout-- > 0)) delay_ms(1);
			printf ("              COMMAND RESULT: %08X (timeout: %d)\n", remoteCtrl[slaveId].cmdResult, timeout);
			if ((remoteCtrl[slaveId].doCmd != 0) || (remoteCtrl[slaveId].cmdResult > 0xFFFF) || (timeout < 0))
			{
				printf ("              WRITE COMMAND ERROR\n");
			}
			else
			{
				time_t rcv_timer;
				time(&rcv_timer);
				remoteCounterSumm[index][2] = (DWORD)rcv_timer;
				remoteCounterSumm[index][0] += remoteCounter[index][0];
				remoteCounterSumm[index][1] += remoteCounter[index][1];
				remoteCounter[index][0] = 0;
				remoteCounter[index][1] = 0;
				printf ("              WRITE COMMAND OK\n");
				delay_ms(100);
			}
		}

		delay_ms(1000);

	}
	return (void*)0;
}
