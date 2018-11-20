#include "../main.h"

Engine::Engine()
{
    streamId 			= 0;
	isOpened 			= false;
	comPortNumber 		= 10;
	comPortBaundRate 	= 9600;

	needFreq 			= 0;
	currFreq 			= 0;
	errorCode 			= 0;
	engineRotates 		= false;
    regState 			= 0;
    lastRegState 		= 0;
	engineCommandTryCount = 3;
	workTimeSec			= 0;
	startTime			= 0;
	bypassMode			= 0;
	bypassValve 		= 24;
}

Engine::~Engine()
{
}