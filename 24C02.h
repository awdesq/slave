#include "STC12C5A60S2.h"

#ifndef I2C_AND_2402
#define I2C_AND_2402

#define ATMEL24C02		0xA0
sbit     SCL=P2^1;
sbit     SDA=P2^0;

void Start_I2c();
void Stop_I2c();
void Ack_I2c(bit a);
bit RecvACK();
bit SendByte(uchar  c);              
uchar RecvByte();

uchar WriteEEPROM(uchar DeviceAddress, uchar DataAddress, uchar *pData, uchar n);
uchar ReadEEPROM(uchar DeviceAddress, uchar DataAddress, uchar *pData, uchar n);
#endif

