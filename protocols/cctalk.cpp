#include "../main.h"

CcTalkCommand::CcTalkCommand()
{
	addrFrom 	= 0x02;
	addrTo		= 0x01;
	command		= 0x00;
	dataLength	= 0x00;
	memset(data, 0, sizeof(data));
	crc			= 0;
	memset(commandData, 0, sizeof(commandData));
	commandLength = 0;
}

CcTalkCommand::CcTalkCommand(BYTE adrfrom, BYTE adrto, BYTE cmd, BYTE* bufdata, BYTE dataLen)
{
	CcTalkCommand();
	addrFrom 	= adrfrom;
	addrTo		= adrto;
	command		= cmd;
	dataLength	= dataLen;
	if ((bufdata != NULL) && (dataLen > 0))
		memcpy(data, bufdata, dataLen);
}

void CcTalkCommand::SetData(BYTE* bufData, BYTE dataSize)
{
	if ((bufData != NULL) && (dataSize > 0))
		memcpy(data, bufData, dataSize);
	dataLength = dataSize;
}

BYTE CcTalkCommand::GetCRC(BYTE* bufData, BYTE sizeData)
{
	BYTE result = 0;
	int t = 0;
	for(t=0; t < sizeData; t++)
		result += bufData[t];
	crc = 256-(result%256);
	return crc;
}

void CcTalkCommand::BuildCommand()
{
	memset(commandData, 0, sizeof(commandData));
	commandData[0] = addrFrom;
	commandData[1] = dataLength;
	commandData[2] = addrTo;
	commandData[3] = command;
	memcpy(&commandData[4], data, dataLength);
	GetCRC(&commandData[0], 5+dataLength);
	commandData[4+dataLength] = crc;
	commandLength = 5+dataLength;
}

bool CcTalkCommand::ParseCommand(BYTE* bufData, BYTE sizeData)
{
	BYTE point = 0;
	bool result = FALSE;
	if (sizeData < 5) return FALSE;

	if (*(bufData) == 0x02)
		point = 0x05 + *(bufData + 1);
	sizeData -= point;
	if (sizeData < 5) return FALSE;
	memset(commandData, 0, sizeof(commandData));
	commandLength = sizeData;
	memcpy(&commandData[0], (bufData+point), sizeData);
	GetCRC(&commandData[0], commandLength-1);
	if (crc == commandData[sizeData-1]) result = TRUE;
	if (commandLength < 5) commandLength = 5;
 	addrFrom 	= commandData[0];
	dataLength	= commandData[1];//commandLength-5;
	addrTo		= commandData[2];
	command		= commandData[3];
	memcpy(data, &commandData[4], dataLength);
	return result;
}

BYTE* CcTalkCommand::GetCmdData()
{
	return commandData;
}

BYTE CcTalkCommand::GetCmdLength()
{
	return commandLength;
}