#include "lcd12864.h"
#include "Delay.h"
#include <string.h>

#ifdef SERIAL_MODE

void WriteByte(unsigned char dat)
{
	unsigned char i;
    for(i = 0; i < 8; i++)
    {
		LCD_SID = (dat & 0x80);
		LCD_SCLK = 0;
		delay1us();
		LCD_SCLK = 1; 
		delay1us();
		LCD_SCLK = 0;
		dat <<= 1;
		LCD_SID = 0; 
	}
}
#endif

void LCDWrite(unsigned char Mode, unsigned char Code)
{
#ifdef SERIAL_MODE
	LCD_CS = 1;
	WriteByte(Mode);	
	WriteByte(Code & 0xF0);
	WriteByte((Code & 0x0F) << 4);
	LCD_CS = 0;
	delay100us();//安全值72us

#else
	LCD_DATA= Code;
    LCD_RS = Mode & 0x02;
    LCD_RW = Mode & 0x04;
    LCD_EN = 1;
    delay1us();
	delay1us();
    LCD_EN = 0;
	delay100us();
#endif
}

void LCDSetXY(unsigned char x, unsigned char y)
{
	switch(y)
	{
		case 1: LCDWrite(WriteCode,0X7F+x);break;              //Line1
		case 2: LCDWrite(WriteCode,0X8F+x);break;              //Line2
		case 3: LCDWrite(WriteCode,0X87+x);break;              //Line3
		case 4: LCDWrite(WriteCode,0X97+x);break;              //Line4
	}
}

void LCDInit(void)
{
#ifdef SERIAL_MODE
	LCD_PSB = 0;
#else
	LCD_PSB = 1;
	LCD_EN = 0;
#endif
	delayms(50);								//Wait > 40ms XRESET Low to High
	LCD_RST = 0;
	delayms(1);					  			
	LCD_RST = 1;
	delayms(1);
 	LCDWrite(WriteCode, 0x30);					//Function Set
	delayms(1);	
	LCDWrite(WriteCode, 0x30);					//Function Set
	delayms(1);
	LCDWrite(WriteCode,LCD_DISPLAY_STATUS);			//Display ON/OFF control
	delayms(1);
	LCDWrite(WriteCode,LCD_CLEAR);				//Display Clear > 10ms
	delayms(20);
	LCDWrite(WriteCode,LCD_ENTRYMODE);			//Entry Mode Set
	delayms(1);
}

void LCDPrintfByte(unsigned char n, unsigned char PrintfZero)
{	
#define WeiShu_N		3
	uchar i, tmp[WeiShu_N + 1];
	tmp[WeiShu_N] = 0;
	for(i = WeiShu_N - 1; i != 0xFF; i--, n /= 10)
		tmp[i] = n % 10 + '0';
		for(i = 0; (i < WeiShu_N) && (PrintfZero == 0); i++)
			if(tmp[i] != '0') break;
	LCDWriteString(&tmp[i]);
#undef WeiShu_N
}

void LCDPrintfWord(unsigned int n, unsigned char PrintfZero)
{
#define WeiShu_N		5
	uchar i, tmp[WeiShu_N + 1];
	tmp[WeiShu_N] = 0;
	for(i = WeiShu_N - 1; i != 0xFF; i--, n /= 10)
		tmp[i] = n % 10 + '0';
		for(i = 0; (i < WeiShu_N) && (PrintfZero == 0); i++)
			if(tmp[i] != '0') break;
	LCDWriteString(&tmp[i]);
#undef WeiShu_N	
}

void LCDWriteString(char* str)
{
	unsigned char i = 16;
	while((i--) && (*str != 0))
		LCDWrite(WriteData, *str++);
}

void LCDPhotoDisplay(uchar *bmp)	
{ 
  uchar i,j;

  LCDWrite(WriteCode, LCD_PHOTO);        //写数据时,关闭图形显示

  for(i=0;i<32;i++)
  {
    LCDWrite(WriteCode, 0x80+i);    //先写入水平坐标值
    LCDWrite(WriteCode, 0x80);      //写入垂直坐标值
    for(j=0;j<16;j++)   //再写入两个8位元的数据    
    	LCDWrite(WriteData, *bmp++);    
  }

  for(i=0;i<32;i++)
  { 
    LCDWrite(WriteCode, 0x80+i);
    LCDWrite(WriteCode, 0x88);
    for(j=0;j<16;j++)         
		LCDWrite(WriteData, *bmp++);
  }
  LCDWrite(WriteCode, LCD_PHOTO);       //写完数据,开图形显示 
}

