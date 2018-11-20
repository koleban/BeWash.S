#ifndef QUEUE_ARRAY
#define QUEUE_ARRAY
struct LogDataStruct {
	public:          
		time_t eventTime;
		int eventId;
		double data1;
		double data2;
		char note[512];
};

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
		int QueuePut(int eventId, double data1, double data2, char* note);
};
#endif