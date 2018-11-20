#ifndef _ccnet_h_
#define _ccnet_h_

#define POLYNOMIAL 0x08408

class CcNetCommand
{
private:
	int var;
public:
	BYTE SYNC;
	BYTE ADR;
	BYTE LEN;
	BYTE CMD;
	BYTE DATA[1024];
	WORD CRC;
	BYTE dataLength;
	BYTE commandLength;
	BYTE commandData[1024];

	CcNetCommand();
	CcNetCommand(BYTE address, BYTE command, BYTE* bufData, BYTE dataLen);
	void SetData(BYTE* bufData, BYTE dataSize);
	WORD GetCRC(BYTE* bufData, BYTE sizeData);
	void BuildCommand();
	bool ParseCommand(BYTE* bufData, BYTE sizeData);
	BYTE* GetCmdData();
	BYTE GetCmdLength();
};
#endif