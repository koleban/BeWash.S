#ifndef QUEUE_ARRAY
#define QUEUE_ARRAY
struct LogDataStruct {
	public:
		int databaseType;
		time_t eventTime;
		int eventId;
		double data1;
		double data2;
		char note[512];
};

struct KkmDataStruct {
	public:
		long id;
		time_t eventTime;
		int eventId;
		char FN[30];
		char RN[30];
		char INN[30];
		int DocNum;
		struct tm DocDateTime;
		unsigned long FP;
		unsigned int Smena;
		unsigned int CheckNum;
		double DocSumm;
		double DocSummCash;
		double DocSummVisa;
		char Note1[256];
		char Note2[256];
		char Note3[256];
		char Note4[256];
};

typedef KkmDataStruct
	KKMQueueType;

typedef LogDataStruct
	QueueType;

class QueueArray
{
	private:
		QueueType* Queue;
		int QueueIn, QueueOut;
		unsigned int QUEUE_ELEMENTS;
		unsigned int QUEUE_SIZE;
	public:
		unsigned int QueueSize;
		unsigned int QueueCount;
		QueueArray();
		QueueArray(unsigned int queueSize);
		int QueuePut(QueueType newElement);
		int QueueGet(QueueType *old);
		//////////////////////////////////////
		int QueuePut(int dbType, int eventId, double data1, double data2, char* note);
};

class KKMQueueArray
{
	private:
		KKMQueueType* Queue;
		int QueueIn, QueueOut;
		unsigned int QUEUE_ELEMENTS;
		unsigned int QUEUE_SIZE;
	public:
		unsigned int QueueSize;
		unsigned int QueueCount;
		KKMQueueArray();
		KKMQueueArray(unsigned int queueSize);
		int QueuePut(KKMQueueType newElement);
		int QueueGet(KKMQueueType *old);
		//////////////////////////////////////
		int QueuePut( int eventid, char* fn, char* rn, char* inn, int docNum,
		struct tm* docDateTime, unsigned long fp, unsigned int smena, unsigned int checkNum,
		double docSumm, double docSummCash, double docSummVisa, char* note1, char* note2, char* note3, char* note4);
};
#endif