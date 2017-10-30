#include "24C02.h"
#include "Delay.h"

#ifdef I2C_AND_2402

uchar myEEPROM[20] = {29, 0, 0, 0xFF, 0x01, '3', '2', '7', '7', '2', '5', '0', '3'};
uchar ReadEEPROM(uchar DeviceAddress, uchar DataAddress, uchar *pData, uchar n)
{
	uchar i;
	for(i = 0; i < n; i++,pData++)
		*pData = myEEPROM[i + DataAddress];

	myEEPROM[0] = 0;
	Start_I2c();
	if(SendByte(DeviceAddress) == 1) return 0;
	if(SendByte(DataAddress) == 1) return 0;
	Start_I2c();
	if(SendByte(DeviceAddress + 1) == 1) return 0;

	for(i = 0; i < n - 1; i++)
	{
     	*pData = RecvByte();
      	Ack_I2c(0); 
     	pData++;
	} 
	*pData = RecvByte();
	Ack_I2c(1);
	Stop_I2c();
	return 1;	
}

uchar WriteEEPROM(uchar DeviceAddress, uchar DataAddress, uchar* pData, uchar n)
{
	uchar i;

	for(i = 0; i < n; i++,pData++)
		myEEPROM[i + DataAddress] =  *pData;

	Start_I2c();
	if(SendByte(DeviceAddress) == 1) return 0;
	if(SendByte(DataAddress) == 1) return 0;
	for(i = 0; i < n; i++)
	{
		if(SendByte(*(pData++)) == 1) return 0;
	}
	Stop_I2c();
	delayms(5);
	return 1;		
}


void Start_I2c()
{
	SDA=1;
	SCL=1;
	delay5us();
	SDA=0;
	delay5us();
	SCL=0;
}

void Stop_I2c()
{
	SDA=0;
	SCL=1;
	delay5us();
	SDA=1;
	delay5us();
}

void Ack_I2c(bit a)
{
	SDA = a;     
	SCL=1;
	delay5us();
	SCL=0;
	delay5us();
}

bit RecvACK()
{
	bit ack;
    SCL = 1;
    delay5us();
    ack = SDA;
    SCL = 0;
    delay5us();
    return ack;
}

bit SendByte(uchar  c)
{
	uchar  BitCnt;
	for(BitCnt = 0; BitCnt < 8; BitCnt++)  /*要传送的数据长度为8位*/
	{
		SDA = (c << BitCnt) & 0x80;                
		SCL = 1;
		delay5us();      
		SCL = 0; 
	}
	return RecvACK();
}
   
uchar RecvByte()
{
	uchar  retc;
	uchar  BitCnt;
	
	retc = 0; 
	SDA = 1;
	for(BitCnt = 0; BitCnt < 8; BitCnt++)
	{
		SCL = 0;
		delay5us();
		SCL = 1;
		delay5us();
		retc = retc << 1;
		retc = retc + (uchar)SDA;
	}
	SCL = 0;    
	return(retc);
}

#endif




