#include "../main.h"

EEPROM* EEPROM::p_instance = 0;

EEPROM::EEPROM()
{
	i2c_handle = -1;
}

bool EEPROM::Init()
{
	if (i2c_handle >= 0) return 1;
	i2c_handle = wiringPiI2CSetup(EEPROM_I2C_DEV_ADDR);
	int timeout = 200;
	if (i2c_handle > 0)
	{
		WORD sign = 0x0000;
		int val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = ReadByte(0x0002);
			delay_ms(EEPROM_DELAY);
		}
		if (timeout < 1) return 0;
		sign = sign | val;
		val = -1;
		timeout = 20;
		while ((timeout--) && (val < 0))
		{
			val = ReadByte(0x0001);
			delay_ms(EEPROM_DELAY);
		}
		if (timeout < 1) return 0;
		sign |= (val << 8);
		if (sign != 0x2302)
		{
			printf ("\nEEPROM: [%04X] New eeprom detected. Clear ...", sign);
			if (ClearEEPROM() == 0)
				printf ("ERROR\n");
			else
				printf ("OK\n");
		}
	}
	return 1;
}

void EEPROM::Close()
{
	if (i2c_handle > 0)
		close(i2c_handle);
	i2c_handle  = -1;
}

int EEPROM::WriteByte(WORD address, BYTE data)
{
	useEeprom = true;
	union i2c_smbus_data i2c_data;
	i2c_data.block[0] = data;
	int result = i2c_smbus_access (i2c_handle, I2C_SMBUS_WRITE, (address) & 0xFF, I2C_SMBUS_BYTE_DATA, &i2c_data);
// 	**********************************
//	FOR AT24C256 - address data - WORD
//	i2c_data.block[0] = address & 0xFF;
//	i2c_data.block[1] = data;
//	int result = i2c_smbus_access (i2c_handle, I2C_SMBUS_WRITE, (address >> 8) & 0xFF, I2C_SMBUS_WORD_DATA, &i2c_data);
//
	useEeprom = false;
	if (result < 0)
		return -1;
	delay_ms(EEPROM_DELAY);
	return data;
}

int EEPROM::ReadByte(WORD address)
{
	union i2c_smbus_data data;
	if (i2c_smbus_access(i2c_handle,I2C_SMBUS_READ,address,
	                     I2C_SMBUS_BYTE_DATA,&data))
		return -1;
	else
return 0x0FF & data.byte;

	useEeprom = true;
//	union i2c_smbus_data data;
//	******************************
//	data.block[0] = address & 0xFF;
//	int result = i2c_smbus_access (i2c_handle, I2C_SMBUS_WRITE, (address >> 8) & 0xFF, I2C_SMBUS_BYTE_DATA, &data);
//
	data.block[0] = 0x00;
	int result = i2c_smbus_access (i2c_handle, I2C_SMBUS_WRITE, (address) & 0xFF, I2C_SMBUS_QUICK, NULL);
	if (result < 0)
		return -1;
	delay_ms(EEPROM_DELAY);
	int val = wiringPiI2CRead(i2c_handle);
//	int val = data.byte;
	useEeprom = false;
	return val & 0xFF;
}

int EEPROM::ClearEEPROM()
{
	useEeprom = true;
	int i = 0;
	while(i++ < 0xFF)
	{
		int timeout = 20;
		while ((timeout--) && (WriteByte(i, 0x00) < 0))
			delay_ms(EEPROM_DELAY);
		if (timeout < 1) return 0;
		delay_ms(EEPROM_DELAY);
	}
	printf (" Writing signature ... ");
	int timeout = 20;
	while ((timeout--) && (WriteByte(0x0001, 0x23) < 0))
		delay_ms(EEPROM_DELAY);
	if (timeout < 1) return 0;
	delay_ms(EEPROM_DELAY);
	timeout = 20;
	while ((timeout--) && (WriteByte(0x0002, 0x02) < 0))
		delay_ms(EEPROM_DELAY);
	if (timeout < 1) return 0;
	delay_ms(EEPROM_DELAY);
	useEeprom = false;

	return 1;
}
