#ifndef _cctalk_h_
#define _cctalk_h_

class CcTalkCommand
{
private:
	int var;
public:
	BYTE addrFrom;
	BYTE addrTo;
	BYTE command;
	BYTE dataLength;
	BYTE data[250];
	BYTE crc;
	BYTE commandData[254];
	BYTE commandLength;

	CcTalkCommand();
	CcTalkCommand(BYTE adrfrom, BYTE adrto, BYTE cmd, BYTE* bufdata, BYTE dataLen);
	void SetData(BYTE* bufData, BYTE dataSize);
	BYTE GetCRC(BYTE* bufData, BYTE sizeData);
	void BuildCommand();
	bool ParseCommand(BYTE* bufData, BYTE sizeData);
	BYTE* GetCmdData();
	BYTE GetCmdLength();
};
#endif