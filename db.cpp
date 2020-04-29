#include "main.h"

Database::Database()
{
	useDatabase = 1;
	lastError = DB_OK;
	isOpened = 0;
	db = NULL;
	memset(&databaseSettings, 0, sizeof(databaseSettings));
}

void Database::Init(Settings* settings)
{
	useDatabase = settings->useDatabase;
	if (!useDatabase) return;
	memcpy(&databaseSettings, &(settings->databaseSettings), sizeof(settings->databaseSettings));
	db = IBPP::DatabaseFactory(databaseSettings.serverName, databaseSettings.databaseName, databaseSettings.userId, databaseSettings.userPasswd);
}

void Database::Init(DatabaseSettings* dbSettings)
{
	if (!useDatabase) return;
	memcpy(&databaseSettings, dbSettings, sizeof(databaseSettings));
	db = IBPP::DatabaseFactory(databaseSettings.serverName, databaseSettings.databaseName, databaseSettings.userId, databaseSettings.userPasswd);
}

int Database::Open()
{
	if (isOpened) {lastError = DB_OK; return lastError;}
	if (!useDatabase) { isOpened = 1; lastError = DB_OK; return lastError;}
    try
    {
    	db->Connect();
    }
    catch (IBPP::Exception& e)
    {
        sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        lastError = DB_CONNECTION_OPEN_ERROR;
        return lastError;
    }

	isOpened = 1;
	lastError = DB_OK;
	return lastError;
}

int Database::Close()
{
	if (!isOpened) {lastError = DB_OK; return lastError;}
	if (!useDatabase) { isOpened = 0; lastError = DB_OK; return lastError;}
    try
    {
    	db->Disconnect();
    }
    catch (IBPP::Exception& e)
    {
        sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        lastError = DB_CONNECTION_CLOSE_ERROR;
        return lastError;
    }

	isOpened = 0;
	lastError = DB_OK;
	return lastError;
}

int Database::Log(int eventId, double data1, double data2, char* note)
{
	settings->busyFlag.QueueLog++;
	lastError = DB_OK; 
	queueLog->QueuePut(eventId, data1, data2, note);
	settings->busyFlag.QueueLog--;
	return lastError;
}

int Database::LogDb(time_t eventTime, int eventId, double data1, double data2, char* note)
{
	if (!useDatabase) { lastError = DB_OK; return lastError;}
	this->Open();
	if (!isOpened)
	{
        sprintf(lastErrorMessage, "%s", "Database connection is closed");
		lastError = DB_CONNECTION_CLOSED;
		return lastError;
	}

	char eventText[1024];
	char queryStr[] = "INSERT INTO LOG(DEV_ID, OBJ_ID, USR_ID, LOG_DATE, EVENT_ID, EVENT_TEXT, DATA1, DATA2, NOTE) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	time_t currentTime = time(NULL);
	struct tm * timeinfo;
	timeinfo = localtime (&eventTime);
	IBPP::Timestamp eventTimeVal;
	if ((currentTime - eventTime) > 90000)
		timeinfo = localtime (&currentTime);
	eventTimeVal.SetDate(1900+timeinfo->tm_year, timeinfo->tm_mon+1, timeinfo->tm_mday);
	eventTimeVal.SetTime(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	IBPP::Transaction tr = IBPP::TransactionFactory(db);
	try
	{
		eventId2eventText(eventId, eventText);
		tr->Start();
		IBPP::Statement st = IBPP::StatementFactory(db, tr);
		st->Prepare(queryStr);
		st->Set(1, settings->commonParams.deviceId);
		st->Set(2, settings->commonParams.objectId);
		st->Set(3, settings->commonParams.userId);
		st->Set(4, eventTimeVal);
		st->Set(5, eventId);
		st->Set(6, eventText);
		st->Set(7, data1);
		st->Set(8, data2);
		st->Set(9, note);
		st->Execute();
		tr->Commit();
		lastError = DB_OK;
	}
	catch (IBPP::Exception& e)
	{
		this->Close();
		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
		lastError = DB_QUERY_ERROR;
		return lastError;
	}
	this->Close();
	return lastError;
}

int Database::Query(DWORD queryType, void* queryParam, void* queryOutput)
{
	if (!useDatabase) return DB_OK;
	this->Open();
	if (!isOpened)
	{
        sprintf(lastErrorMessage, "%s", "Database connection is closed");
        lastError = DB_CONNECTION_CLOSED;
        return lastError;
	}

	char queryStr[1024];
	IBPP::Transaction tr = IBPP::TransactionFactory(db);
	switch (queryType)
	{
		//////////////////////////////////////////////////////////////////////////
		default:
        	sprintf(lastErrorMessage, "%s", "Unknown query type");
			lastError = DB_UNKNOWN_QUERY_TYPE;
			this->Close();
			return lastError;
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_TEST:
			lastError = DB_OK;
			this->Close();
			return lastError;
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GETRECORDCOUNT:
			if (!useDatabase) { *((int*)queryOutput) = 0; lastError = DB_OK; return lastError;}
			sprintf( queryStr, "%s", "SELECT COUNT(ID) FROM LOG");
			try
			{
				int* recordCount = (int*)queryOutput;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Execute(queryStr);
				while (st->Fetch())
				{
		    		st->Get(1, *recordCount);
		    		break;
				}
				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_SYSTIME:
			if (!useDatabase) { queryOutput = NULL; lastError = DB_OK; return lastError;}
			sprintf( queryStr, "%s", "SELECT FIRST 1 CURRENT_TIMESTAMP FROM LOG");
			try
			{
				IBPP::Timestamp* param1 = (IBPP::Timestamp*)queryOutput;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Execute(queryStr);
				while (st->Fetch())
				{
		    		st->Get(1, *param1);
		    		break;
				}
				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GET_PARAM:
			if (!useDatabase) { queryOutput = NULL; lastError = DB_OK; return lastError;}
			if (queryParam == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close(); return lastError;}
			sprintf( queryStr, "%s", "SELECT DATA2 FROM LOG WHERE ((EVENT_ID=?) AND (DATA1=?) AND ((DEV_ID=?) OR (DEV_ID=999))) ORDER BY LOG_DATE DESC, DEV_ID DESC;");
			try
			{
				double* outParams = (double*)queryOutput;
				double* dataParams = (double*)queryParam;
				int eventid = *(dataParams);
				double data1 = *(dataParams+1);
				*outParams = -1;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, eventid);
				st->Set(2, data1);
				st->Set(3, settings->commonParams.deviceId);
				st->Execute();
				while (st->Fetch())
				{
					outParams = (double*)queryOutput;
		    		st->Get(1, *outParams);
		    		break;
				}
				tr->Commit();
				this->Close();
				lastError = DB_OK;
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GET_PARAM_ARRAY:
			if (!useDatabase) { queryOutput = NULL; lastError = DB_OK; return lastError;}
			if (queryParam == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close(); return lastError;}
			sprintf( queryStr, "%s", "SELECT DATA1, DATA2 FROM LOG WHERE ((EVENT_ID=?) AND ((DEV_ID=?) OR (DEV_ID=999))) ORDER BY LOG_DATE DESC, DEV_ID DESC;");
			try
			{
				double* outParams = (double*)queryOutput;
				double* dataParams = (double*)queryParam;
				int eventid = *(dataParams);
				double data1 = *(dataParams+1);
				*outParams = -1;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, eventid);
				st->Set(2, settings->commonParams.deviceId);
				st->Execute();
				int indext = 1;
				outParams = (double*)queryOutput;
				while (st->Fetch())
				{
		    		st->Get(1, *(outParams+indext));
		    		st->Get(2, *(outParams+indext+1));
		    		indext+=2;
		    		if (indext > (MAX_PARAM_ARRAY_ITEM)) break;
				}
				*outParams = (double)((indext-1) / 2.0);
				tr->Commit();
				this->Close();
				lastError = DB_OK;
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GET_NEW_PARAM:
			if (!useDatabase) { queryOutput = NULL; lastError = DB_OK; return lastError;}
			sprintf( queryStr, "%s", "SELECT FIRST 1 ID FROM LOG WHERE ((EVENT_ID<95) AND ((DEV_ID=?) OR (DEV_ID=999))) ORDER BY ID DESC;");
			try
			{
				double* outParams = (double*)queryOutput;
				*outParams = -1;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, settings->commonParams.deviceId);
				st->Execute();
				while (st->Fetch())
				{
					outParams = (double*)queryOutput;
		    		st->Get(1, *outParams);
		    		break;
				}
				tr->Commit();
				this->Close();
				lastError = DB_OK;
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_WORKSPACE_OPEN:
			if (!useDatabase) { lastError = DB_OK; return lastError;}
			sprintf( queryStr, "%s", "INSERT INTO WRKSPACE(DEV_ID, OPEN_DATE, DATA1, DATA2, NOTE, OBJ_ID, USR_ID) VALUES(?, CURRENT_TIMESTAMP, ?, ?, ?, ?, ?)");
			try
			{
				double data1 = 0;
				double data2 = 0;
				char note[] = "Open new workspace ...";
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, settings->commonParams.deviceId);
				st->Set(2, data1);
				st->Set(3, data2);
				st->Set(4, note);
				st->Set(5, settings->commonParams.objectId);
				st->Set(6, settings->commonParams.userId);
				st->Execute();
				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_WORKSPACE_CLOSE:
			if (!useDatabase) { lastError = DB_OK; return lastError;}
			if (queryParam == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close(); return lastError;}
			sprintf( queryStr, "%s", "UPDATE WRKSPACE SET CLOSE_DATE=CURRENT_TIMESTAMP, DATA1 = ?, DATA2 = ?, NOTE = 'Closed workspace' WHERE (CLOSE_DATE>'31.12.2029 00:00:00') AND (DEV_ID = ?) AND (OBJ_ID = ?)");
			try
			{
				double* dataParams = (double*)queryParam;
				double data1 = *(dataParams);
				double data2 = *(dataParams+1);
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, data1);
				st->Set(2, data2);
				st->Set(3, settings->commonParams.deviceId);
				st->Set(4, settings->commonParams.objectId);
				st->Execute();
				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GET_OPENED_WORKSPACE:
			if (!useDatabase) { lastError = DB_OK; return lastError;}
			if (queryOutput == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close(); return lastError;}
			sprintf( queryStr, "%s", "SELECT FIRST 1 OPEN_DATE, CLOSE_DATE, DATA1, DATA2 FROM WRKSPACE WHERE (DEV_ID = ?) ORDER BY CLOSE_DATE DESC, OPEN_DATE DESC");
			try
			{
				IBPP::Timestamp* outParams = (IBPP::Timestamp*)queryOutput;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, settings->commonParams.deviceId);
				st->Execute();

				while (st->Fetch())
				{
		    		st->Get(1, *outParams);
		    		st->Get(2, *(outParams + 1));
		    		break;
				}

				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GET_ENGINE_WORK_TIME:
			if (!useDatabase) { lastError = DB_OK; return lastError;}
			if (queryOutput == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close();return lastError;}
			sprintf( queryStr, "%s", "SELECT SUM(DATA1) FROM LOG WHERE (DEV_ID = ?) AND (EVENT_ID=202) AND (DATA1 < 2000)");
			try
			{
				double* outParams = (double*)queryOutput;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, settings->commonParams.deviceId);
				st->Execute();

				while (st->Fetch())
				{
		    		st->Get(1, *outParams);
		    		break;
				}

				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GET_CASH_STORE:
			if (!useDatabase) { lastError = DB_OK; return lastError;}
			if (queryParam == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close();return lastError;}
			if (queryOutput == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close();return lastError;}
			sprintf( queryStr, "%s", "SELECT SUM(DATA1* DATA2) AS MONEY FROM LOG WHERE (LOG_DATE>=?) AND (LOG_DATE<=?) AND (DEV_ID = ?) AND (EVENT_ID=111)");
			try
			{
				double* outParams = (double*)queryOutput;
				*outParams = 0;
				IBPP::Timestamp* dataParams = (IBPP::Timestamp*)queryParam;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, *dataParams);
				st->Set(2, *(dataParams+1));
				st->Set(3, settings->commonParams.deviceId);
				st->Execute();

				while (st->Fetch())
				{
		    		st->Get(1, *outParams);
		    		break;
				}

				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GET_ALL_CASH_STORE:
			if (!useDatabase) { lastError = DB_OK; return lastError;}
			if (queryOutput == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close();return lastError;}
			sprintf( queryStr, "%s", "SELECT SUM(DATA1* DATA2) AS MONEY FROM LOG WHERE (DEV_ID = ?) AND (EVENT_ID=111)");
			try
			{
				double* outParams = (double*)queryOutput;
				*outParams = 0;
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, settings->commonParams.deviceId);
				st->Execute();

				while (st->Fetch())
				{
		    		st->Get(1, *outParams);
		    		break;
				}

				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_GET_CARD_INFO:
			if (!useDatabase) { lastError = DB_OK; return lastError;}
			if (queryParam == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close();return lastError;}
			if (queryOutput == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close();return lastError;}
			try
			{
				IBPP::Statement st;
				DB_RFIDCardInfo* outParams = (DB_RFIDCardInfo*)queryOutput;
				DWORD* dataParams = (DWORD*)queryParam;
				outParams->cardId = *dataParams;
				// Get card information
/*
				sprintf( queryStr, "SELECT OBJ_ID AS BLOCKED FROM rfid_info WHERE CARD_ID = %lu", outParams->cardId);
				printf("[1] queryStr: %s\n", queryStr);
				tr->Start();
				st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Execute();

				printf("[2] queryStr: %s\n", queryStr);
				while (st->Fetch())
				{
					printf("[3] queryStr: %s\n", queryStr);
		    		st->Get(1, outParams->cardBlocked);
		    		break;
				}

				tr->Commit();
				lastError = DB_OK;
				if (outParams->cardBlocked == 1)
					{ outParams->cardMoney = 0; this->Close(); return lastError; }
*/
				outParams->cardBlocked = 0;
				// Get card money
				sprintf( queryStr, "SELECT SUM(MONEY) FROM rfid_money where card_id = %lu", outParams->cardId);
				printf("[DEBUG] DB: Get card info [%lu]\n %s\n", outParams->cardId, queryStr);
				tr->Start();
				st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Execute();
				outParams->cardMoney = 0;

				while (st->Fetch())
				{
		    		st->Get(1, outParams->cardMoney);
		    		break;
				}

				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
		case DB_QUERY_TYPE_SET_CARD_INFO:
			if (!useDatabase) { lastError = DB_OK; return lastError;}
			if (queryParam == NULL) { lastError = DB_QUERY_ERROR; sprintf(lastErrorMessage, "%s", "NULL PARAM EXCEPTION"); 				this->Close();return lastError;}
			try
			{
				DB_RFIDCardInfo* outParams = (DB_RFIDCardInfo*)queryParam;
				printf("[DEBUG] DB: Set card info [%lu] - %d\n", outParams->cardId, outParams->cardMoney);
				sprintf( queryStr, "INSERT INTO RFID_MONEY(CARD_ID, MONEY) VALUES( %lu, ?)", outParams->cardId);
				tr->Start();
				IBPP::Statement st = IBPP::StatementFactory(db, tr);
				st->Prepare(queryStr);
				st->Set(1, (int32_t)outParams->cardMoney);
				st->Execute();
				tr->Commit();
				lastError = DB_OK;
				this->Close();
				return lastError;
			}
    		catch (IBPP::Exception& e)
    		{
        		sprintf(lastErrorMessage, "%s", e.ErrorMessage());
        		lastError = DB_QUERY_ERROR;
				this->Close();
        		return lastError;
    		}
			break;
		//////////////////////////////////////////////////////////////////////////
	}

	lastError = DB_ERROR;
	this->Close();
	return lastError;
}

void Database::eventId2eventText(int eventId, char* outputBuffer)
{
	switch(eventId)
	{
	/////////////////
		default:
		sprintf(outputBuffer, "%s", "UNKNOWN LOG EVENT");
		break;
	/////////////////
		case DB_EVENT_TYPE_TEST:
		sprintf(outputBuffer, "%s", "[INFO] Test log message");
		break;
	/////////////////
		case DB_EVENT_TYPE_SYSTEMSTART:
		sprintf(outputBuffer, "%s", "[INFO] System start: Car wash self ""beWash"" (c) 2012-2015");
		break;
	/////////////////
		case DB_EVENT_TYPE_MONEY_INCOME:
		sprintf(outputBuffer, "%s", "[INFO] Money (coin) forwarding");
		break;
	/////////////////
		case DB_EVENT_TYPE_THREAD_INIT:
		sprintf(outputBuffer, "%s", "[THREAD] Thread init and start");
		break;
	/////////////////
		case DB_EVENT_TYPE_DVC_BILL_WARNING:
		sprintf(outputBuffer, "%s", "[DVC] Bill validator DROP CASSETE REMOVED!");
		break;
	/////////////////
		case DB_EVENT_TYPE_DVC_BILL_ERROR:
		sprintf(outputBuffer, "%s", "[DVC] Bill validator RESET!");
		break;
	/////////////////
		case DB_EVENT_TYPE_DVC_CLOSE:
		sprintf(outputBuffer, "%s", "[DVC] Device closed");
		break;
	/////////////////
		case DB_EVENT_TYPE_DVC_COIN_INIT:
		sprintf(outputBuffer, "%s", "[DVC] Coin acceptor device opened");
		break;
	/////////////////
		case DB_EVENT_TYPE_DVC_BUTTON_INIT:
		sprintf(outputBuffer, "%s", "[DVC] Button panel device opened");
		break;
	/////////////////
		case DB_EVENT_TYPE_INT_CARD_INSERTED:
		sprintf(outputBuffer, "%s", "[INT] New card inserted");
		break;
	/////////////////
		case DB_EVENT_TYPE_INT_CARD_GONE:
		sprintf(outputBuffer, "%s", "[INT] Card is gone");
		break;
	/////////////////
		case DB_EVENT_TYPE_INT_PROGRAM_CHANGED:
		sprintf(outputBuffer, "%s", "[INT] Programm changed");
		break;
	/////////////////
		case DB_EVENT_TYPE_INT_MONEY_EVENT:
		sprintf(outputBuffer, "%s", "[INT] COIN INCOME");
		break;
	/////////////////
		case DB_EVENT_TYPE_DVC_ENGINE_WORK_TIME:
		sprintf(outputBuffer, "%s", "[ENG] {WORK TIME}");
		break;
	/////////////////
		case DB_EVENT_TYPE_INT_IDKFA_EVENT:
		sprintf(outputBuffer, "%s", "[INT] Warning!!! IDKFA event. Hello my dear pussy!");
		break;
	/////////////////
		case DB_EVENT_TYPE_HARDWARE_ERROR:
		sprintf(outputBuffer, "%s", "[HARDWARE] Warning!!! Hardware unknown error!");
		break;
	/////////////////
		case DB_EVENT_TYPE_SYSTEM_LIVE:
		sprintf(outputBuffer, "%s", "[COMMON] System live message. All OK!");
		break;
	/////////////////
		case DB_EVENT_TYPE_EXT_SRV_ADD_MONEY:
		sprintf(outputBuffer, "%s", "[EXT] Ext cmd: Add money");
		break;
	/////////////////
		case DB_EVENT_TYPE_EXT_SRV_NEW_PRG:
		sprintf(outputBuffer, "%s", "[EXT] Ext cmd: Set new program");
		break;
	/////////////////
		case DB_EVENT_TYPE_EXT_COIN_REJECTED:
		sprintf(outputBuffer, "%s", "[MONEY] Coin rejected");
		break;
	/////////////////
		case DB_EVENT_TYPE_EXT_NEW_BUTTON:
		sprintf(outputBuffer, "%s", "[External] Ext: New button pressed");
		break;
	/////////////////
	}
}