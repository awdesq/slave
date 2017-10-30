#include "stc12c5a60s2.h"
#include "Delay.h"
#include "SoftReset.h"
#ifdef SOFTRESET
void SoftResetToISPMonitor()
{
	IAP_CONTR = 0x60;				//IAP控制符
}
void Init_UART_For_Download()		//9600bps@12MHz
{
	EA = 0;				//关中断
	PCON &= 0x7f;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	BRT = 0xD9;			//设定独立波特率发生器重装值
	AUXR |= 0x04;		//独立波特率发生器时钟为Fosc,即1T
	AUXR |= 0x01;		//串口1选择独立波特率发生器为波特率发生器
	AUXR |= 0x10;		//启动独立波特率发生器
	RI = TI = 0;		//清零标志位
	ES = 1;				//开中断
	EA = 1;				//开中断
}
void UARTInterruptForDownload() interrupt 4
{
	uchar UART_Code;

	if(RI)
	{
		RI = 0;
		UART_Code = SBUF;
		if(UART_Code == SelfDefineISPDownloadCommand)
		{
			SoftResetToISPMonitor();
		}		
	}
	if(TI) TI = 0;
}
#endif