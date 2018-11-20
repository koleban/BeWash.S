#include "../main.h"

CcNetCommand::CcNetCommand()
{
	SYNC		= 0x02;
	ADR 		= 0x01;
	LEN			= 0x00;
	CMD			= 0x00;
	CRC			= 0x00;
	dataLength	= 0x00;
	memset(DATA, 0, sizeof(DATA));
	memset(commandData, 0, sizeof(commandData));
	commandLength = 0;
}

CcNetCommand::CcNetCommand(BYTE address, BYTE command, BYTE* bufData, BYTE dataLen)
{
	SYNC		= 0x02;
	ADR 		= address;
	LEN			= 0x00;
	CMD			= command;
	CRC			= 0x00;
	dataLength	= dataLen;
	memset(DATA, 0, sizeof(DATA));
	memset(commandData, 0, sizeof(commandData));
	commandLength = 0;
	if ((dataLen > 0) && (dataLen < sizeof(DATA)) && (bufData != NULL))
		memcpy(DATA, bufData, dataLen);
}

void CcNetCommand::SetData(BYTE* bufData, BYTE dataSize)
{
	if ((dataSize > 0) && (bufData != NULL))
		memcpy(&DATA[0], bufData, dataSize);
	dataLength = dataSize;
}

WORD CcNetCommand::GetCRC(BYTE* bufData, BYTE sizeData)
{
  WORD CRC, i;
  unsigned char j;
  CRC = 0;
  for(i=0; i < sizeData; i++)
   {
    CRC = CRC ^ bufData[i];
    for(j=0; j < 8; j++)
     {
      if(CRC & 0x0001) {CRC >>= 1; CRC ^= POLYNOMIAL;}
      else CRC >>= 1;
     }
   }

  return CRC;
}

void CcNetCommand::BuildCommand()
{
	LEN = 6+dataLength;
	memset(commandData, 0, sizeof(commandData));
	commandData[0] = SYNC;
	commandData[1] = ADR;
	commandData[2] = LEN;
	commandData[3] = CMD;
	if (dataLength > 0)
		memcpy(&commandData[4], DATA, dataLength);
	WORD crc = GetCRC(commandData, 4+dataLength);
	commandData[4+dataLength] = ((BYTE)(crc&0xFF));
	commandData[5+dataLength] = ((BYTE)(crc >> 8));
	commandLength = 6+dataLength;
}

bool CcNetCommand::ParseCommand(BYTE* bufData, BYTE sizeData)
{
	BYTE point = 0;
	bool result = FALSE;
	if (sizeData < 6) return FALSE;

	memset(DATA, 0, sizeof(DATA));
	memset(commandData, 0, sizeof(commandData));
	commandLength = sizeData;
	if ((sizeData > 0) && (bufData != NULL))
		memcpy(&commandData[0], bufData, sizeData);
	WORD crc = GetCRC(&commandData[0], commandLength-2);
	if (crc == *((WORD*)&commandData[sizeData-2])) result = TRUE;
	if (commandLength < 6) commandLength = 6;
 	SYNC 	= commandData[0];
	ADR		= commandData[1];//commandLength-5;
	LEN		= commandData[2];
	CMD		= commandData[3];
	dataLength = LEN-6;
	if (dataLength > 0)
		memcpy(DATA, &commandData[4], dataLength);

	return result;
}

BYTE* CcNetCommand::GetCmdData()
{
	return &commandData[0];
}

BYTE CcNetCommand::GetCmdLength()
{
	return commandLength;
}