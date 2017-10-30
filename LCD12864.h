#include "stc12c5a60s2.h"

#define SERIAL_MODE

//∑¿÷π÷ÿ‘ÿŒƒº˛
#ifndef LCD12864Driver
#define	LCD12864Driver

#define LCD_CURSOR_INC					0x06
#define	LCD_CURSOR_DEC					0x04
#define	LCD_PHOTO						0x36
#define LCD_NO_PHOTO					0x34
#define LCD_EX_CODE						0x34
#define	LCD_BASE_CODE					0x30
#define	LCD_CLEAR						0x01
#define	LCD_HOME						0x02
#define	LCD_ENABLE_INVERT_HIGHLIGHT		0x01
#define	LCD_ENABLE_CURSOR				0x02
#define	LCD_ENABLE_DISPLAY_ALL			0x04
#define	LCD_DISPLAY_STATUS 				(0x08 | LCD_ENABLE_DISPLAY_ALL)
#define	LCD_ENTRY_MODE_RIGHT_SHIFT		0x02
#define	LCD_ENTRY_MODE_LEFT_SHIFT		0x00
#define	LCD_ENTRY_MODE_WHOLE_SHIFT		0x01
#define	LCD_ENTRYMODE					(0x04 | LCD_ENTRY_MODE_RIGHT_SHIFT)
#define	ReadData			0xFE
#define WriteCode			0xF8
#define	WriteData			0xFA
#define	ReadBusy			0xFC
#define	LCD_DATA			P0
#define	LCD_RS				LCD_CS
#define LCD_RW				LCD_SID
#define	LCD_EN				LCD_SCLK
//sbit	LCD_CS		=	P2^6;
//sbit	LCD_SID		=	P2^5;
//sbit	LCD_SCLK	=	P2^7;
//sbit 	LCD_PSB		= 	P3^2;

//sbit 	LCD_RST		= 	P3^7;
sbit	LCD_CS		=	P2^0;
sbit	LCD_SID		=	P2^1;
sbit	LCD_SCLK	=	P2^2;

sbit 	LCD_PSB		= 	P2^3;
sbit 	LCD_RST		= 	P2^4;

void LCDInit(void);
void LCDWrite(unsigned char Mode, unsigned char Code);
void LCDSetXY(unsigned char x, unsigned char y);
void LCDPrintfByte(unsigned char n, unsigned char PrintfZero);
void LCDPrintfWord(unsigned int n, unsigned char PrintfZero);
void LCDWriteString(char* str);
void LCDPhotoDisplay(unsigned char *bmp);

#ifdef SERIAL_MODE
void WriteByte(unsigned char dat);
#endif

#endif
