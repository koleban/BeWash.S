#ifndef _EEPROM_H_
#define _EEPROM_H_

#ifndef DWORD
#define DWORD			unsigned int
#define WORD			unsigned short
#define BYTE			unsigned char
#define SDWORD			int
#endif


#ifndef EEPROM_I2C_DEV_ADDR
#define EEPROM_I2C_DEV_ADDR 0x50
#endif

#ifndef I2C_SMBUS_BYTE_DATA
#define I2C_SMBUS_READ	1
#define I2C_SMBUS_WRITE	0

#define I2C_SMBUS_BYTE_DATA	    2
#define I2C_SMBUS_WORD_DATA	    3
#endif

class Settings;

class EEPROM
{
private:
	static EEPROM* p_instance;

	EEPROM();
	int i2c_handle;
public:
	bool useEeprom;
  	bool Init();
  	void Close();
  	int ClearEEPROM();
  	int WriteByte(WORD address, BYTE data);
	int ReadByte(WORD address);
	static EEPROM* getInstance()
	{
		if(!p_instance)
			p_instance = new EEPROM();
		return p_instance;
	}
};

#endif