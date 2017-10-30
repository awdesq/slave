#include "stc12c5a60s2.h"
#include "Delay.h"
#include "SoftReset.h"
#ifdef SOFTRESET
void SoftResetToISPMonitor()
{
	IAP_CONTR = 0x60;				//IAP���Ʒ�
}
void Init_UART_For_Download()		//9600bps@12MHz
{
	EA = 0;				//���ж�
	PCON &= 0x7f;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	BRT = 0xD9;			//�趨���������ʷ�������װֵ
	AUXR |= 0x04;		//���������ʷ�����ʱ��ΪFosc,��1T
	AUXR |= 0x01;		//����1ѡ����������ʷ�����Ϊ�����ʷ�����
	AUXR |= 0x10;		//�������������ʷ�����
	RI = TI = 0;		//�����־λ
	ES = 1;				//���ж�
	EA = 1;				//���ж�
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