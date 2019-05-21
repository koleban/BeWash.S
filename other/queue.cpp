#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <sys/statvfs.h>

#include "queue.h"

QueueArray::QueueArray()
{
	QueueCount = 0;
	QueueSize = 100;
	QUEUE_ELEMENTS = 100;
	QUEUE_SIZE = QUEUE_ELEMENTS + 1;
	QueueIn = QueueOut = 0;
	Queue = (QueueType*)malloc(QUEUE_ELEMENTS * sizeof(QueueType));
	memset(Queue, 0, QUEUE_ELEMENTS * sizeof(QueueType));
}

QueueArray::QueueArray(unsigned int queueSize)
{
	QueueCount = 0;
	QueueSize = queueSize;
	QUEUE_ELEMENTS = queueSize;
	QUEUE_SIZE = QUEUE_ELEMENTS + 1;
    QueueIn = QueueOut = 0;
	Queue = (QueueType*)malloc(QUEUE_ELEMENTS * sizeof(QueueType));
}

int QueueArray::QueuePut(QueueType newElement)
{
	if (QueueCount++ >= QueueSize) QueueCount = QueueSize;
    Queue[QueueIn].eventTime = newElement.eventTime;
    Queue[QueueIn].eventId = newElement.eventId;
    Queue[QueueIn].data1 = newElement.data1;
    Queue[QueueIn].data2 = newElement.data2;
	if (newElement.note != NULL) strcpy(Queue[QueueIn].note, newElement.note);
    QueueIn = (QueueIn + 1) % QUEUE_SIZE;
    return 0; // No errors
}

int QueueArray::QueueGet(QueueType *old)
{
    if(QueueIn == QueueOut)
    {
        return -1; /* Queue Empty - nothing to get*/
    }

	if (QueueCount-- < 1) QueueCount = 0;
    old->eventTime = Queue[QueueOut].eventTime;
    old->eventId = Queue[QueueOut].eventId;
    old->data1 = Queue[QueueOut].data1;
    old->data2 = Queue[QueueOut].data2;
	if (Queue[QueueOut].note != NULL) strcpy(old->note, Queue[QueueOut].note);
    //memcpy(old, &Queue[QueueOut], sizeof(Queue[QueueOut]));

    QueueOut = (QueueOut + 1) % QUEUE_SIZE;

    return 0; // No errors
}

int QueueArray::QueuePut(int eventId, double data1, double data2, char* note)
{
	QueueType v;
	v.eventTime = time(NULL);
	v.eventId = eventId;
	v.data1 = data1;
	v.data2 = data2;
	memset(&v.note[0], 0, 512);
	if (note != NULL) strcpy(v.note, note);
	return QueuePut(v);
}

//********************************************************************
//********************************************************************
//********************************************************************

KKMQueueArray::KKMQueueArray()
{
	QueueSize = 100;
	QUEUE_ELEMENTS = 100;
	QUEUE_SIZE = QUEUE_ELEMENTS + 1;
	QueueIn = QueueOut = 0;
	Queue = (KKMQueueType*)malloc(QUEUE_ELEMENTS * sizeof(KKMQueueType));
	memset(Queue, 0, QUEUE_ELEMENTS * sizeof(KKMQueueType));
}

KKMQueueArray::KKMQueueArray(unsigned int queueSize)
{
	QueueSize = queueSize;
	QUEUE_ELEMENTS = queueSize;
	QUEUE_SIZE = QUEUE_ELEMENTS + 1;
    QueueIn = QueueOut = 0;
	Queue = (KKMQueueType*)malloc(QUEUE_ELEMENTS * sizeof(KKMQueueType));
}

int KKMQueueArray::QueuePut(KKMQueueType newElement)
{
	if (QueueCount++ >= QueueSize) QueueCount = QueueSize;
/*
    Queue[QueueIn].eventTime = newElement.eventTime;
    Queue[QueueIn].eventId = newElement.eventId;
    Queue[QueueIn].data1 = newElement.data1;
    Queue[QueueIn].data2 = newElement.data2;
	if (newElement.note != NULL) strcpy(Queue[QueueIn].note, newElement.note);
*/
    memcpy(&Queue[QueueIn], &newElement, sizeof(KKMQueueType));
    QueueIn = (QueueIn + 1) % QUEUE_SIZE;
    return 0; // No errors
}

int KKMQueueArray::QueueGet(KKMQueueType *old)
{
    if(QueueIn == QueueOut)
    {
        return -1; /* Queue Empty - nothing to get*/
    }

	if (QueueCount-- < 1) QueueCount = 0;
/*
    old->eventTime = Queue[QueueOut].eventTime;
    old->eventId = Queue[QueueOut].eventId;
    old->data1 = Queue[QueueOut].data1;
    old->data2 = Queue[QueueOut].data2;
	if (Queue[QueueOut].note != NULL) strcpy(old->note, Queue[QueueOut].note);
*/
    memcpy(old, &Queue[QueueOut], sizeof(KKMQueueType));

    QueueOut = (QueueOut + 1) % QUEUE_SIZE;

    return 0; // No errors
}

int KKMQueueArray::QueuePut( int eventid, char* fn, char* rn, char* inn, int docNum,
		struct tm* docDateTime, unsigned long fp, unsigned int smena, unsigned int checkNum,
		double docSumm, double docSummCash, double docSummVisa, char* note1, char* note2, char* note3, char* note4)
{
	KKMQueueType v;
	memset(&v, 0, sizeof(KKMQueueType));
	v.id = (long)time(NULL);
	v.eventTime = time(NULL);
	v.eventId = eventid;
	strcpy(v.FN, fn);
	strcpy(v.RN, rn);
	strcpy(v.INN, inn);
	v.DocNum = docNum;
	memcpy(&v.DocDateTime, docDateTime, sizeof(v.DocDateTime));
	v.FP = fp;
	v.Smena = smena;
	v.CheckNum = checkNum;
	v.DocSumm = docSumm;
	v.DocSummCash = docSummCash;
	v.DocSummVisa = docSummVisa;

	if (note1 != NULL) strcpy(v.Note1, note1);
	if (note2 != NULL) strcpy(v.Note2, note2);
	if (note3 != NULL) strcpy(v.Note3, note3);
	if (note4 != NULL) strcpy(v.Note4, note4);

	return QueuePut(v);
}
